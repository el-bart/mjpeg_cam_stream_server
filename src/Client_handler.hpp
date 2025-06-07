#pragma once
#include "Jpeg.hpp"
#include <But/System/Descriptor.hpp>

struct Client_handler final
{
  explicit Client_handler(But::System::Descriptor fd);

  Client_handler(Client_handler&&) = default;
  Client_handler& operator=(Client_handler&&) = default;

  Client_handler(Client_handler const&) = delete;
  Client_handler& operator=(Client_handler const&) = delete;

  void enqueueFrame(JpegPtr frame);
  void nonBlockingIo();

  int socket() const { return fd_.get(); }

private:
  void sendHeaders();
  void sendFrameData();

  But::System::Descriptor fd_;
  char const* headers_{nullptr};
  JpegPtr frame_;
  unsigned char const* frameRemaingPtr_{nullptr};
  size_t frameRemaingBytes_{0};
};
