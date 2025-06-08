#pragma once
#include "Jpeg.hpp"
#include "Logger.hpp"
#include <But/System/Descriptor.hpp>

struct Client_handler final
{
  Client_handler(Logger log, But::System::Descriptor fd);

  Client_handler(Client_handler&&) = default;
  Client_handler& operator=(Client_handler&&) = default;

  Client_handler(Client_handler const&) = delete;
  Client_handler& operator=(Client_handler const&) = delete;

  void enqueueFrame(JpegPtr frame);
  void nonBlockingIo();
  bool hasWorkToDo() const;

  int socket() const { return fd_.get(); }

private:
  void sendHeaders();
  void sendFrameData();

  Logger log_;
  But::System::Descriptor fd_;

  char const* topHeaders_{nullptr};

  std::string preFrameHeaders_;

  JpegPtr frame_;
  unsigned char const* frameRemaingPtr_{nullptr};
  size_t frameRemaingBytes_{0};

  char const* postFrameHeaders_{nullptr};
};
