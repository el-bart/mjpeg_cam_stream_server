#pragma once
#include "Client_handler.hpp"
#include "program_options.hpp"
#include <But/System/Epoll.hpp>
#include <But/System/Descriptor.hpp>
#include <But/Threading/JoiningThread.hpp>
#include <atomic>
#include <memory>
#include <vector>
#include <thread>

struct Server final
{
  explicit Server(Server_config sc);
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
  void removeDeadClients();
  void enqueueNewFrame();
  JpegPtr nextFrame();

  std::mutex nextFrameMutex_;
  JpegPtr nextFrame_; // nullptr == nothing is waiting

  But::System::Descriptor listenSocket_;

  std::atomic_bool quit_{false};
  std::vector<std::weak_ptr<Client_handler>> clients_;
  But::System::Epoll epoll_;
  But::Threading::JoiningThread<std::thread> th_;
};
