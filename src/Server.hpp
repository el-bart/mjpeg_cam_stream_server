#pragma once
#include "Client_handler.hpp"
#include "program_options.hpp"
#include "Logger.hpp"
#include "Ip.hpp"
#include <But/System/Epoll.hpp>
#include <But/System/Descriptor.hpp>
#include <But/Threading/JoiningThread.hpp>
#include <atomic>
#include <memory>
#include <thread>
#include <map>

struct Server final
{
  struct Client_context
  {
    Ip ip_;
    Logger log_;
    Client_handler handler_;
  };

  Server(Logger log, Server_config sc);
  ~Server();

  Server(Server&&) = delete;
  Server& operator=(Server&&) = delete;

  Server(Server const&) = delete;
  Server& operator=(Server const&) = delete;

  /** enqueues new frame. can be called from any thread. */
  void enqueueFrame(JpegPtr frame);

private:
  void loop();
  void loopOnce();
  void enqueueNewFrame();
  void acceptClient();
  void disconnectClient(int fd);
  void clientIo(int fd);
  void startObserving(int fd);
  void stopObserving(int fd);
  JpegPtr nextFrame();

  std::mutex nextFrameMutex_;
  JpegPtr nextFrame_; // nullptr == nothing is waiting

  Logger log_;
  std::atomic_bool quit_{false};
  But::System::Descriptor listenSocket_;
  std::map<int, Client_context> clients_;
  But::System::Epoll epoll_;
  But::Threading::JoiningThread<std::thread> th_;
};
