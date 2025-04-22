#pragma once
#include <vector>
#include <cinttypes>
#include <memory>

struct Jpeg
{
  std::vector<uint8_t> data_;
};

using JpegPtr = std::shared_ptr<Jpeg>;
