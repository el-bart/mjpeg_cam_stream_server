#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using But::System::Epoll;
using But::System::Descriptor;


namespace
{
struct Clients_count
{
  size_t value_{};
};
constexpr auto fieldName(Clients_count const*) { return "Clients_count"; }
inline auto fieldValue(Clients_count const& cc) { return cc.value_; }


auto create_server(Server_config const& sc)
{
  auto fd = Descriptor{ socket(AF_INET, SOCK_STREAM, 0) };
  if(not fd)
    throw std::runtime_error{"Server: cannot create FD"};

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(sc.port_);
  if( bind(fd.get(), (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    throw std::runtime_error{"Server: bind() failed"};

  if( listen(fd.get(), 10) == -1)
    throw std::runtime_error{"Server: listen() failed"};

  return fd;
}
}


Server::Server(Logger log, Server_config sc):
  log_{ std::move(log) },
  listen_socket_{ create_server( std::move(sc) ) },
  th_{ [this] { this->loop(); } }
{
}


Server::~Server()
{
  quit_ = true;
  epoll_.interrupt();
}


void Server::enqueue_frame(JpegPtr frame)
{
  {
    std::lock_guard lock{next_frame_mutex_};
    next_frame_ = frame;
  }
  epoll_.interrupt();
}


void Server::loop()
{
  epoll_.add( listen_socket_.get(), [this](int, auto) { this->accept_client(); }, Epoll::Event::In );

  while(not quit_)
  {
    try
    {
      loop_once();
    }
    catch(std::exception const& ex)
    {
      log_.warning("exception thrown while processing clients", Exception{ex});
    }
  }
}


void Server::loop_once()
{
  enqueue_new_frame();
  epoll_.wait();
}


void Server::enqueue_new_frame()
{
  auto f = next_frame();
  if(not f)
    return;

  for(auto& cp: clients_)
  {
    cp.second.handler_.enqueue_frame(f);
    stop_observing(cp.first);
    start_observing(cp.first);
  }
}


JpegPtr Server::next_frame()
{
  std::lock_guard lock{next_frame_mutex_};
  JpegPtr frame;
  using std::swap;
  swap(frame, next_frame_);
  return frame;
}


namespace
{
auto clientIp(sockaddr_in const &client_addr)
{
  char client_ip[INET_ADDRSTRLEN];
  auto *addr_in = (struct sockaddr_in *)&client_addr;
  if( inet_ntop(AF_INET, &(addr_in->sin_addr), client_ip, INET_ADDRSTRLEN) == nullptr )
    throw std::runtime_error{"Server: clientIp(): failed to get client IP"};
  return Ip{client_ip};
}
}


void Server::accept_client()
{
  sockaddr_in client_addr;
  unsigned size = sizeof(client_addr);
  Descriptor client_fd{ accept(listen_socket_.get(), (struct sockaddr *)&client_addr, &size) };
  if(not client_fd)
    throw std::runtime_error{"Server::accept_client(): accept() failed"};
  auto const fd = client_fd.get();
  auto const ip = clientIp(client_addr);
  auto log = log_.withFields(ip);
  {
    Client_context ctx{ ip, log, Client_handler{ log, std::move(client_fd) } };
    clients_.insert( std::make_pair(fd, std::move(ctx)) );
    clients_count_ = clients_.size();
  }
  start_observing(fd);
  log.info("Server::accept_client(): accepted new client connection", Clients_count{ clients_.size() });
}


void Server::disconnect_client(int const fd)
{
  auto it = clients_.find(fd);
  if(it == end(clients_))
      return;
  auto const processed_frames = it->second.handler_.processed_frames();
  auto log = it->second.log_;
  stop_observing(fd);
  clients_.erase(it);
  clients_count_ = clients_.size();
  log.info("Server::disconnect_client(): client disconnected", Clients_count{ clients_.size() }, processed_frames);
}


void Server::client_io(int fd)
{
  auto it = clients_.find(fd);
  if(it == end(clients_))
    return;
  auto& h = it->second.handler_;

  h.non_blocking_io();
  if( not h.has_work_to_do() )
    stop_observing(fd);
}


void Server::start_observing(int fd)
{
  epoll_.add( fd, [this](int fd, auto) { this->disconnect_client(fd); }, Epoll::Event::Hup, Epoll::Event::Err );
  epoll_.add( fd, [this](int fd, auto) { this->client_io(fd); }, Epoll::Event::Out );
}


void Server::stop_observing(int fd)
{
  epoll_.remove(fd);
}
