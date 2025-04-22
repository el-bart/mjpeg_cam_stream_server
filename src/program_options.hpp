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
  Resolution capture_resolution;
  std::filesystem::path video_device;
};

struct Program_options
{
  Camera_config camera_config;
  std::optional<std::string> show_help;
};

Program_options parse_program_options(int argc, char** argv);
