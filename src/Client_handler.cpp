#include "Client_handler.hpp"
#include "But/System/makeNonblocking.hpp"
#include <cassert>
#include <cstring>
#include <unistd.h>

namespace
{
auto top_headers()
{
  return
    "HTTP/1.0 200 OK\r\n"
    "Server: localhost\r\n"
    "Connection: close\r\n"
    "Max-Age: 0\r\n"
    "Expires: 0\r\n"
    "Cache-Control: no-cache, private\r\n"
    "Pragma: no-cache\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=MjpegServerFrameBoundaryIndicator\r\n"
    "\r\n";
}


auto pre_frame_headers(JpegPtr const& frame)
{
  std::stringstream ss;
  ss << "--MjpegServerFrameBoundaryIndicator\r\n"
        "Content-type: image/jpeg\r\n"
        "Content-Length: " << frame->data_.size() << "\r\n"
        "\r\n";
  return ss.str();
}


auto post_frame_headers()
{
  return
    "\r\n"
    "\r\n";
}
}


Client_handler::Client_handler(Logger log, But::System::Descriptor fd):
  log_{ std::move(log) },
  fd_{ std::move(fd) },
  top_headers_{ top_headers() }
{
  But::System::makeNonblocking( fd_.get() );
}


void Client_handler::enqueue_frame(JpegPtr frame)
{
  if(top_headers_) // no point in enqueuing anything if initial headers are not yet complete
    return;
  if(post_frame_headers_) // still sending previosu frame - do not touch anything
    return;
  processed_frames_.count_ += 1u;
  frame_ = std::move(frame);
  pre_frame_headers_ = pre_frame_headers(frame_);
  frame_remaing_ptr_ = frame_->data_.data();
  frame_remaing_bytes_ = frame_->data_.size();
  post_frame_headers_ = post_frame_headers();
}


void Client_handler::non_blocking_io()
{
  if(top_headers_)
    send_headers();
  else
    send_frame_data();
}


namespace
{
size_t write_some(char const* msg, But::System::Descriptor const& fd, void const* data, size_t len)
{
  auto const ret = write(fd.get(), data, len);
  if(ret == -1)
  {
    if(errno == EWOULDBLOCK)
      return 0;
    throw std::runtime_error{"Client_handler::write_some(): error while writing data to client: " + std::string{msg}};
  }
  assert(ret > 0);
  return ret;
}
}


void Client_handler::send_headers()
{
  assert(top_headers_);
  if( top_headers_ == top_headers() )
    log_.info("sending top HTTP headers");

  top_headers_ += write_some("writing top headers", fd_, top_headers_, strlen(top_headers_));

  if(*top_headers_ == 0)
  {
    top_headers_ = nullptr;
    log_.info("top HTTP headers sent successfuly - sending frames from now on");
  }
}


void Client_handler::send_frame_data()
{
  if( not pre_frame_headers_.empty() )
  {
    pre_frame_headers_.erase( 0, write_some("writing frame pre-headers", fd_, pre_frame_headers_.data(), pre_frame_headers_.size()) );
    if( not pre_frame_headers_.empty() )
      return;
  }

  if(frame_)
  {
    assert(frame_remaing_ptr_);
    assert(frame_remaing_bytes_ > 0);
    auto const bytes = write_some("writing frame bytes", fd_, frame_remaing_ptr_, frame_remaing_bytes_);
    frame_remaing_ptr_ += bytes;
    frame_remaing_bytes_ -= bytes;
    if(frame_remaing_bytes_ > 0)
      return;
    assert(frame_remaing_bytes_ == 0);
    frame_remaing_ptr_ = nullptr;
    frame_.reset();
  }

  if(post_frame_headers_)
  {
    post_frame_headers_ += write_some("writing frame post-headers", fd_, post_frame_headers_, strlen(post_frame_headers_));
    if(*post_frame_headers_ == 0)
      post_frame_headers_ = nullptr;
  }
}


bool Client_handler::has_work_to_do() const
{
  if(top_headers_) return true;
  if(not pre_frame_headers_.empty()) return true;
  if(frame_remaing_ptr_) return true;
  if(post_frame_headers_) return true;
  return false;
}
