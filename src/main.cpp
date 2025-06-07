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
    if( signal(s, handle_quit) == SIG_ERR )
      throw std::runtime_error{"register_signals(): failed to register signal " + std::to_string(s) + " for 'ignore' action"};
}
}



int main(int argc, char** argv)
{
  auto log = makeConsoleLogger();
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
    log.warning("filenced OpenCV logs output to prevent spamming stdout");

    register_signals();
    log.info("registered required signal handlers");

    Server s{log, po.server_config_};
    log.info("server initialized");

    Camera c{log, po.camera_config_};
    log.info("cemera initialized");

    log.flush();
    while(not g_quit)
    {
      auto f = c.capture();
      s.enqueueFrame(f);
    }
    log.info("quit has been requested via signal");
  }
  catch(std::exception const& ex)
  {
    log.error("application failed with an exception", Exception{ex});
    return 13;
  }
}
