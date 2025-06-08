#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using But::System::Epoll;
using But::System::Descriptor;


namespace
{
struct ClientsCount
{
  size_t value_{};
};
constexpr auto fieldName(ClientsCount const*) { return "ClientsCount"; }
inline auto fieldValue(ClientsCount const& cc) { return cc.value_; }


auto createServer(Server_config const& sc)
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
  listenSocket_{ createServer( std::move(sc) ) },
  th_{ [this] { this->loop(); } }
{
}


Server::~Server()
{
  quit_ = true;
  epoll_.interrupt();
}


void Server::enqueueFrame(JpegPtr frame)
{
  {
    std::lock_guard lock{nextFrameMutex_};
    nextFrame_ = frame;
  }
  epoll_.interrupt();
}


void Server::loop()
{
  epoll_.add( listenSocket_.get(), [this](int, auto) { this->acceptClient(); }, Epoll::Event::In );

  while(not quit_)
  {
    try
    {
      loopOnce();
    }
    catch(std::exception const& ex)
    {
      log_.warning("exception thrown while processing clients", Exception{ex});
    }
  }
}


void Server::loopOnce()
{
  enqueueNewFrame();
  epoll_.wait();
}


void Server::enqueueNewFrame()
{
  auto f = nextFrame();
  if(not f)
    return;

  for(auto& cp: clients_)
  {
    cp.second.handler_.enqueueFrame(f);
    stopObserving(cp.first);
    startObserving(cp.first);
  }
}


JpegPtr Server::nextFrame()
{
  std::lock_guard lock{nextFrameMutex_};
  JpegPtr frame;
  using std::swap;
  swap(frame, nextFrame_);
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


void Server::acceptClient()
{
  sockaddr_in client_addr;
  unsigned size = sizeof(client_addr);
  Descriptor client_fd{ accept(listenSocket_.get(), (struct sockaddr *)&client_addr, &size) };
  if(not client_fd)
    throw std::runtime_error{"Server::acceptClient(): accept() failed"};
  auto const fd = client_fd.get();
  auto const ip = clientIp(client_addr);
  auto log = log_.withFields(ip);
  {
    Client_context ctx{ ip, log, Client_handler{ log, std::move(client_fd) } };
    clients_.insert( std::make_pair(fd, std::move(ctx)) );
  }
  startObserving(fd);
  log.info("Server::acceptClient(): accepted new client connection", ClientsCount{ clients_.size() });
}


void Server::disconnectClient(int const fd)
{
  auto it = clients_.find(fd);
  if(it == end(clients_))
      return;
  auto log = it->second.log_;
  stopObserving(fd);
  clients_.erase(it);
  log.info("Server::disconnectClient(): client disconnected", ClientsCount{ clients_.size() });
}


void Server::clientIo(int fd)
{
  auto it = clients_.find(fd);
  if(it == end(clients_))
    return;
  auto& h = it->second.handler_;

  h.nonBlockingIo();
  if( not h.hasWorkToDo() )
    stopObserving(fd);
}


void Server::startObserving(int fd)
{
  epoll_.add( fd, [this](int fd, auto) { this->disconnectClient(fd); }, Epoll::Event::Hup, Epoll::Event::Err );
  epoll_.add( fd, [this](int fd, auto) { this->clientIo(fd); }, Epoll::Event::Out );
}


void Server::stopObserving(int fd)
{
  epoll_.remove(fd);
}
