#include "Client_handler.hpp"
#include "But/System/makeNonblocking.hpp"
#include <cassert>
#include <cstring>
#include <unistd.h>

namespace
{
auto topHeaders()
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


auto preFrameHeaders(JpegPtr const& frame)
{
  std::stringstream ss;
  ss << "--MjpegServerFrameBoundaryIndicator\r\n"
        "Content-type: image/jpeg\r\n"
        "Content-Length: " << frame->data_.size() << "\r\n"
        "\r\n";
  return ss.str();
}


auto postFrameHeaders()
{
  return
    "\r\n"
    "\r\n";
}
}


Client_handler::Client_handler(Logger log, But::System::Descriptor fd):
  log_{ std::move(log) },
  fd_{ std::move(fd) },
  topHeaders_{ topHeaders() }
{
  But::System::makeNonblocking( fd_.get() );
}


void Client_handler::enqueueFrame(JpegPtr frame)
{
  if(topHeaders_) // no point in enqueuing anything if initial headers are not yet complete
    return;
  if(postFrameHeaders_) // still sending previosu frame - do not touch anything
    return;
  frame_ = std::move(frame);
  preFrameHeaders_ = preFrameHeaders(frame_);
  frameRemaingPtr_ = frame_->data_.data();
  frameRemaingBytes_ = frame_->data_.size();
  postFrameHeaders_ = postFrameHeaders();
}


void Client_handler::nonBlockingIo()
{
  if(topHeaders_)
    sendHeaders();
  else
    sendFrameData();
}


namespace
{
size_t writeSome(char const* msg, But::System::Descriptor const& fd, void const* data, size_t len)
{
  auto const ret = write(fd.get(), data, len);
  if(ret == -1)
  {
    if(errno == EWOULDBLOCK)
      return 0;
    throw std::runtime_error{"Client_handler::writeSome(): error while writing data to client: " + std::string{msg}};
  }
  assert(ret > 0);
  return ret;
}
}


void Client_handler::sendHeaders()
{
  assert(topHeaders_);
  if( topHeaders_ == topHeaders() )
    log_.info("sending top HTTP headers");

  topHeaders_ += writeSome("writing top headers", fd_, topHeaders_, strlen(topHeaders_));

  if(*topHeaders_ == 0)
  {
    topHeaders_ = nullptr;
    log_.info("top HTTP headers sent successfuly - sending frames from now on");
  }
}


void Client_handler::sendFrameData()
{
  if( not preFrameHeaders_.empty() )
  {
    preFrameHeaders_.erase( 0, writeSome("writing frame pre-headers", fd_, preFrameHeaders_.data(), preFrameHeaders_.size()) );
    if( not preFrameHeaders_.empty() )
      return;
  }

  if(frame_)
  {
    assert(frameRemaingPtr_);
    assert(frameRemaingBytes_ > 0);
    auto const bytes = writeSome("writing frame bytes", fd_, frameRemaingPtr_, frameRemaingBytes_);
    frameRemaingPtr_ += bytes;
    frameRemaingBytes_ -= bytes;
    if(frameRemaingBytes_ > 0)
      return;
    assert(frameRemaingBytes_ == 0);
    frameRemaingPtr_ = nullptr;
    frame_.reset();
  }

  if(postFrameHeaders_)
  {
    postFrameHeaders_ += writeSome("writing frame post-headers", fd_, postFrameHeaders_, strlen(postFrameHeaders_));
    if(*postFrameHeaders_ == 0)
      postFrameHeaders_ = nullptr;
  }
}
