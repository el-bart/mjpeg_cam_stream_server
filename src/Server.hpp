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
  void enqueue_frame(JpegPtr frame);

private:
  void loop();
  void loop_once();
  void enqueue_new_frame();
  void accept_client();
  void disconnect_client(int fd);
  void client_io(int fd);
  void start_observing(int fd);
  void stop_observing(int fd);
  JpegPtr next_frame();

  std::mutex next_frame_mutex_;
  JpegPtr next_frame_; // nullptr == nothing is waiting

  Logger log_;
  std::atomic_bool quit_{false};
  But::System::Descriptor listen_socket_;
  std::map<int, Client_context> clients_;
  But::System::Epoll epoll_;
  But::Threading::JoiningThread<std::thread> th_;
};
