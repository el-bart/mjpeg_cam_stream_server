#include <iostream>
#include <string>
#include "program_options.hpp"
#include "Camera.hpp"
#include "Server.hpp"
#include "Logger.hpp"

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

    // smoke test
    Camera c{log, po.camera_config_};
    c.capture();

    // TODO...
  }
  catch(std::exception const& ex)
  {
    log.error("application failed with an exception", Exception{ex});
    return 13;
  }
}
