#pragma once
#include <filesystem>
#include <optional>
#include <string>

struct Resolution
{
  unsigned x_{};
  unsigned y_{};
};

struct Camera_config
{
  Resolution capture_resolution_;
  std::filesystem::path video_device_;
};

struct Program_options
{
  Camera_config camera_config_;
  std::optional<std::string> show_help_;
};

Program_options parse_program_options(int argc, char** argv);
