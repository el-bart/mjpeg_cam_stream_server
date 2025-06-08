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

  void enqueue_frame(JpegPtr frame);
  void non_blocking_io();
  bool has_work_to_do() const;

  int socket() const { return fd_.get(); }
  auto processed_frames() const { return processed_frames_; }

private:
  void send_headers();
  void send_frame_data();

  Logger log_;
  But::System::Descriptor fd_;
  Processed_frames processed_frames_;

  char const* top_headers_{nullptr};

  std::string pre_frame_headers_;

  JpegPtr frame_;
  unsigned char const* frame_remaing_ptr_{nullptr};
  size_t frame_remaing_bytes_{0};

  char const* post_frame_headers_{nullptr};
};


constexpr auto fieldName(Client_handler::Processed_frames const*) { return "Processed_frames"; }
inline auto const& fieldValue(Client_handler::Processed_frames const& pf) { return pf.count_; }
