#include "program_options.hpp"
#include "Camera.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <string>
#include <atomic>
#include <signal.h>
#include <opencv2/core/utils/logger.hpp>

namespace
{
std::atomic_bool g_quit{false};
void handle_quit(int) { g_quit = true; }
void handle_ignore(int) { }

void register_signals()
{
  for(auto s: {SIGINT, SIGQUIT, SIGTERM})
    if( signal(s, handle_quit) == SIG_ERR )
      throw std::runtime_error{"register_signals(): failed to register signal " + std::to_string(s) + " for 'quit' action"};

  for(auto s: {SIGPIPE, SIGFPE, SIGIO})
    if( signal(s, handle_ignore) == SIG_ERR )
      throw std::runtime_error{"register_signals(): failed to register signal " + std::to_string(s) + " for 'ignore' action"};
}
}



int main(int argc, char** argv)
{
  auto log = make_console_logger();
  try
  {
    auto const po = parse_program_options(argc, argv);
    if(po.show_help_)
    {
      std::cout << *po.show_help_;
      return 2;
    }
    log.info("configuration used by application", po.camera_config_, po.server_config_);

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    log.warning("silenced OpenCV logs output to prevent spamming stdout");

    Server s{log, po.server_config_};
    log.info("server initialized", po.server_config_);

    Camera c{log, po.camera_config_};
    log.info("cemera initialized", po.camera_config_);

    // keep it below camera init, as OpenCV seems to override these and throw exceptions...
    register_signals();
    log.info("registered required signal handlers");

    log.info("all up - awaiting client connections");
    auto has_clients = false;
    while(not g_quit)
    {
      if( s.clients_count() == 0u )
      {
        if(has_clients)
        {
          log.info("no clients connected - stopping frame grabbing to save CPU cycles");
          has_clients = false;
        }
        // frame capture and processing is actually heavyweight - thus doing nothing, when there are no clients
        std::this_thread::sleep_for( std::chrono::milliseconds{100} );
        continue;
      }
      if(not has_clients)
      {
        log.info("client connected - starting frame grabbing again");
        has_clients = true;
      }
      auto f = c.capture();
      s.enqueue_frame(f);
    }
    log.info("quit has been requested via signal");
  }
  catch(std::exception const& ex)
  {
    log.error("application failed with an exception", Exception{ex});
    return 13;
  }
}
