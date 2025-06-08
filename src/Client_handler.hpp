#pragma once
#include "Jpeg.hpp"
#include "Logger.hpp"
#include <But/System/Descriptor.hpp>

struct Client_handler final
{
  struct Processed_frames
  {
    size_t count_{0};
  };

  Client_handler(Logger log, But::System::Descriptor fd);

  Client_handler(Client_handler&&) = default;
  Client_handler& operator=(Client_handler&&) = default;

  Client_handler(Client_handler const&) = delete;
  Client_handler& operator=(Client_handler const&) = delete;

  void enqueueFrame(JpegPtr frame);
  void nonBlockingIo();
  bool hasWorkToDo() const;

  int socket() const { return fd_.get(); }
  auto processed_frames() const { return processed_frames_; }

private:
  void sendHeaders();
  void sendFrameData();

  Logger log_;
  But::System::Descriptor fd_;
  Processed_frames processed_frames_;

  char const* topHeaders_{nullptr};

  std::string preFrameHeaders_;

  JpegPtr frame_;
  unsigned char const* frameRemaingPtr_{nullptr};
  size_t frameRemaingBytes_{0};

  char const* postFrameHeaders_{nullptr};
};


constexpr auto fieldName(Client_handler::Processed_frames const*) { return "Processed_frames"; }
inline auto const& fieldValue(Client_handler::Processed_frames const& pf) { return pf.count_; }
