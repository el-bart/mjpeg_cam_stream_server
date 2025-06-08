#pragma once
#include <string>
#include "Logger.hpp"

struct Ip
{
  std::string value_;
};

constexpr auto fieldName(Ip const*) { return "IP"; }
inline auto const& fieldValue(Ip const& ip)
{
  return ip.value_;
}
