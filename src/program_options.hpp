#pragma once
#include <filesystem>
#include <optional>
#include <string>

struct Program_options
{
  struct Resolution
  {
    unsigned x_{};
    unsigned y_{};
  };

  Resolution capture_resolution;
  std::filesystem::path video_device;
  std::optional<std::string> show_help;
};

Program_options parse_program_options(int argc, char** argv);
