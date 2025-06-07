#include "Server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
*/

namespace
{
auto createServer(Server_config const& sc)
{
  auto fd = But::System::Descriptor{ socket(AF_INET, SOCK_STREAM, 0) };
  if(not fd)
    throw std::runtime_error{"Server: cannot create FD"};

  struct sockaddr_in server_addr;
  socklen_t sin_size;
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


Server::Server(Server_config sc):
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
  std::lock_guard lock{nextFrameMutex_};
  nextFrame_ = frame;
}


void Server::loop()
{
  // TODO: register listening Socket
  while(not quit_)
  {
    try
    {
      loopOnce();
    }
    catch(std::exception const& ex)
    {
      std::cerr << "Server::loop(): error: " << ex.what() << "\n";
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
  auto constexpr isDead = [](auto& c) { return c.lock() == nullptr; };
  auto newEnd = std::remove_if( begin(clients_), end(clients_), isDead );
  clients_.erase( newEnd, end(clients_) );
}


void Server::enqueueNewFrame()
{
  auto f = nextFrame();
  if(not f)
    return;

  auto hasDeadClients = false;
  for(auto& cw: clients_)
    if(auto cs = cw.lock(); cs)
      cs->enqueueFrame(f);
    else
      hasDeadClients = true;

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
