#pragma once
#include "Jpeg.hpp"

struct Client_handler final
{
  //explicit Client_handler_handler(...)

  Client_handler(Client_handler&&) = default;
  Client_handler& operator=(Client_handler&&) = default;

  Client_handler(Client_handler const&) = delete;
  Client_handler& operator=(Client_handler const&) = delete;

  void enqueueFrame(JpegPtr frame);
  void nonBlockingIo();

private:
  char const* headers_{nullptr};
  JpegPtr frame_;
};
