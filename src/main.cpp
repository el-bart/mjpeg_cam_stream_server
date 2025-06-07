#include <iostream>
#include <string>
#include "program_options.hpp"
#include "Camera.hpp"

int main(int argc, char** argv)
{
  try
  {
    auto const po = parse_program_options(argc, argv);
    if(po.show_help_)
    {
      std::cout << *po.show_help_;
      return 1;
    }
    std::cout << "using " << po.camera_config_.video_device_ << " device in ";
    std::cout << po.camera_config_.capture_resolution_.x_ << "x" << po.camera_config_.capture_resolution_.y_ << "\n";
    std::cout << "stream served at: http://" << po.server_config_.ip_ << ":" << po.server_config_.port_ << "\n";

    // smoke test
    Camera c{po.camera_config_};
    c.capture();

    // TODO...
  }
  catch(std::exception const& ex)
  {
    std::cerr << argv[0] << ": ERROR: " << ex.what() << "\n";
    return 13;
  }
}
