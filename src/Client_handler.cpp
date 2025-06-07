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
    "HTTP/1.0 200 OK\n\r"
    "Server: localhost\n\r"
    "Connection: close\n\r"
    "Max-Age: 0\n\r"
    "Expires: 0\n\r"
    "Cache-Control: no-cache, private\n\r"
    "Pragma: no-cache\n\r"
    "Content-Type: multipart/x-mixed-replace; boundary=MjpegServerFrameBoundaryIndicator\n\r"
    "\n\r";
}
}


Client_handler::Client_handler(But::System::Descriptor fd):
  fd_{ std::move(fd) },
  headers_{ topHeaders() }
{
  But::System::makeNonblocking( fd.get() );
}


void Client_handler::enqueueFrame(std::shared_ptr<Jpeg> frame)
{
  if(headers_)
    return;
  if(frame_)
    return;
  frame_ = std::move(frame);
  frameRemaingPtr_ = frame->data_.data();
  frameRemaingBytes_ = frame->data_.size();
}


void Client_handler::nonBlockingIo()
{
  if(headers_)
    sendHeaders();
  else
    sendFrameData();
}


void Client_handler::sendHeaders()
{
  assert(headers_);
  auto const ret = write( fd_.get(), headers_, strlen(headers_) );
  if(ret == -1)
  {
    if(errno == EWOULDBLOCK)
      return;
    throw std::runtime_error{"Client_handler::sendHeaders(): error while writing headers"};
  }
  headers_ += ret;

  if(*headers_ == 0)
    headers_ = nullptr;
}


void Client_handler::sendFrameData()
{
  // TODO
}
