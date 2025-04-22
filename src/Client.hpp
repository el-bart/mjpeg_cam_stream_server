#pragma once
#include "Jpeg.hpp"
#include <vector>

struct Client final
{
  //explicit Client(...)

  Client(Client&&) = default;
  Client& operator=(Client&&) = default;

  Client(Client const&) = delete;
  Client& operator=(Client const&) = delete;

  void send(JpegPtr frame);

private:
  char const* headers_{nullptr};
  JpegPtr frame_;
};
