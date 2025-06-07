#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using But::System::Epoll;
using But::System::Descriptor;

constexpr auto fieldName(Server::Client_context const*) { return "Client_context"; }
auto objectValue(Logger::EntryProxy& p, Server::Client_context const& ctx)
{
  p.value("ip", ctx.ip_);
}


namespace
{
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


void Server::removeDeadClients()
{
  auto constexpr isDead = [](auto& c) { return c.handler_.lock() == nullptr; };
  auto newEnd = std::remove_if( begin(clients_), end(clients_), isDead );
  clients_.erase( newEnd, end(clients_) );
}


void Server::enqueueNewFrame()
{
  auto f = nextFrame();
  if(not f)
    return;

  auto hasDeadClients = false;
  for(auto& ctx: clients_)
    if(auto c = ctx.handler_.lock(); c)
      c->enqueueFrame(f);
    else
    {
      hasDeadClients = true;
      log_.info("client disconnected", ctx);
    }

  if(hasDeadClients)
    removeDeadClients();
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
std::string clientIp(sockaddr_in const &client_addr)
{
  char client_ip[INET_ADDRSTRLEN];
  auto *addr_in = (struct sockaddr_in *)&client_addr;
  if( inet_ntop(AF_INET, &(addr_in->sin_addr), client_ip, INET_ADDRSTRLEN) == nullptr )
    throw std::runtime_error{"Server: clientIp(): failed to get client IP"};
  return client_ip;
}
}


void Server::acceptClient()
{
  sockaddr_in client_addr;
  unsigned size = sizeof(client_addr);
  Descriptor client_fd{ accept(listenSocket_.get(), (struct sockaddr *)&client_addr, &size) };
  if(not client_fd)
    throw std::runtime_error{"Server::acceptClient(): accept() failed"};
  Client_context ctx{ clientIp(client_addr) };
  auto csp = std::make_shared<Client_handler>( log_.withFields(ctx), std::move(client_fd) );
  ctx.handler_ = csp;
  clients_.push_back( std::move(ctx) );

  epoll_.add( csp->socket(), [this](int fd, auto) { this->disconnectClient(fd); }, Epoll::Event::Hup );
  epoll_.add( csp->socket(), [csp](int, auto) { csp->nonBlockingIo(); }, Epoll::Event::In );

  log_.info("Server::acceptClient(): accepted new client connection", clients_.back());
}


void Server::disconnectClient(int const fd)
{
  this->epoll_.remove(fd);

  for(auto& ctx: clients_)
    if(auto c = ctx.handler_.lock(); not c)
      log_.info("client disconnected", ctx);
  removeDeadClients();
}
