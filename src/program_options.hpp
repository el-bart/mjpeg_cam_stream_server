#pragma once
#include "Logger.hpp"
#include <filesystem>
#include <optional>
#include <string>

struct Resolution
{
  unsigned x_{};
  unsigned y_{};
};

inline constexpr auto fieldName(Resolution const*) { return "Resolution"; }
inline auto objectValue(Logger::EntryProxy& p, Resolution const& res)
{
  p.value("x", res.x_);
  p.value("y", res.y_);
}


struct Camera_config
{
  Resolution capture_resolution_;
  std::filesystem::path video_device_;
};

inline constexpr auto fieldName(Camera_config const*) { return "Camera_config"; }
inline auto objectValue(Logger::EntryProxy& p, Camera_config const& cc)
{
  p.object("capture").nest(cc.capture_resolution_);
  p.value("device", cc.video_device_.string());
}


struct Server_config
{
  uint16_t port_{};
};

inline constexpr auto fieldName(Server_config const*) { return "Server_config"; }
inline auto objectValue(Logger::EntryProxy& p, Server_config const& sc)
{
  p.value("port", sc.port_);
}

struct Program_options
{
  Camera_config camera_config_;
  Server_config server_config_;
  std::optional<std::string> show_help_;
};

Program_options parse_program_options(int argc, char** argv);
