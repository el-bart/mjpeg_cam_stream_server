#include <iostream>
#include <string>
#include "program_options.hpp"

int main(int argc, char** argv)
{
  try
  {
    auto const po = parse_program_options(argc, argv);
    if(po.show_help)
    {
      std::cout << *po.show_help;
      return 1;
    }
    std::cout << "using " << po.camera_config.video_device << " device in " << po.camera_config.capture_resolution.x_ << "x" << po.camera_config.capture_resolution.y_ << "\n";
    // TODO...

  }
  catch(std::exception const& ex)
  {
    std::cerr << argv[0] << ": ERROR: " << ex.what() << "\n";
    return 13;
  }
}
