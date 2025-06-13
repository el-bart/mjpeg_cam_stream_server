#include "Camera.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

namespace
{
void set_resolution_and_format(But::System::Descriptor const& fd, Resolution const res)
{
  v4l2_format fmt;
  bzero(&fmt, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = res.x_;
  fmt.fmt.pix.height = res.y_;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  if( ioctl(fd.get(), VIDIOC_S_FMT, &fmt) == -1 )
    BUT_THROW(Camera::Error, "set_resolution_and_format(): ioctl(VIDIOC_S_FMT) failed: " << strerror(errno));
}


void set_framerate(But::System::Descriptor const& fd, int const fps)
{
  v4l2_streamparm parm;
  bzero(&parm, sizeof(parm));
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if( ioctl(fd.get(), VIDIOC_G_PARM, &parm) == -1)
    BUT_THROW(Camera::Error, "set_framerate(): ioctl(VIDIOC_G_PARM) failed: " << strerror(errno));

  parm.parm.capture.timeperframe.numerator = 1;
  parm.parm.capture.timeperframe.denominator = fps;
  if( ioctl(fd.get(), VIDIOC_S_PARM, &parm) == -1)
    BUT_THROW(Camera::Error, "set_framerate(): ioctl(VIDIOC_S_PARM) failed: " << strerror(errno));
}


template<typename Mmap>
auto prepare_buffers(But::System::Descriptor const& fd, unsigned const count)
{
  // request given buffers
  v4l2_requestbuffers req;
  bzero(&req, sizeof(req));
  req.count = count;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if( ioctl(fd.get(), VIDIOC_REQBUFS, &req) == -1 )
    BUT_THROW(Camera::Error, "prepare_buffers(): ioctl(VIDIOC_REQBUFS) failed: " << strerror(errno));
  if(req.count < count)
    BUT_THROW(Camera::Error, "prepare_buffers(): insufficient buffers: resquested " << count << " got " << req.count);

  // mmap-buffers
  std::vector<Mmap> buffers;
  buffers.reserve(count);
  for(auto i = 0u; i < count; ++i)
  {
    v4l2_buffer buf;
    bzero(&buf, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if( ioctl(fd.get(), VIDIOC_QUERYBUF, &buf) == -1 )
      BUT_THROW(Camera::Error, "prepare_buffers(): ioctl(VIDIOC_QUERYBUF) failed: " << strerror(errno));
    buffers.push_back( Mmap{fd, buf.length, buf.m.offset} );
  }
  return buffers;
}


void queue_buffers(But::System::Descriptor const& fd, size_t count)
{
  assert(count > 0u);
  for (auto i = 0u; i < count; ++i)
  {
    v4l2_buffer buf;
    bzero(&buf, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if( ioctl(fd.get(), VIDIOC_QBUF, &buf) == -1 )
      BUT_THROW(Camera::Error, "queue_buffers(): ioctl(VIDIOC_QBUF) failed: " << strerror(errno));
  }
}


void streaming_start(But::System::Descriptor const& fd)
{
  v4l2_buf_type const type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if( ioctl(fd.get(), VIDIOC_STREAMON, &type) == -1 )
      BUT_THROW(Camera::Error, "streaming_start(): ioctl(VIDIOC_STREAMON) failed: " << strerror(errno));
}


void streaming_stop(But::System::Descriptor const& fd)
{
  v4l2_buf_type const type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if( ioctl(fd.get(), VIDIOC_STREAMOFF, &type) == -1 )
      BUT_THROW(Camera::Error, "streaming_stop(): ioctl(VIDIOC_STREAMOFF) failed: " << strerror(errno));
}
} // unnamed namespace


Camera::Camera(Logger log, Camera_config const& cfg):
  log_{ std::move(log) },
  fd_{ open(cfg.video_device_.string().c_str(), O_RDWR) }
{
  if(not fd_)
    BUT_THROW(Error, "open(" << cfg.video_device_ << ") failed: " << strerror(errno));
  set_resolution_and_format(fd_, cfg.capture_resolution_);
  set_framerate(fd_, 30);   // TODO: this does not seem to work actually... :/
  buffers_ = prepare_buffers<Mmap_buf>(fd_, 2); // use double-buffering
  queue_buffers(fd_, buffers_.size());
  streaming_start(fd_); // keep this line as the last step in the c-tor!
}


Camera::~Camera()
{
  streaming_stop(fd_); // keep this line as the first step in the d-tor!
}


JpegPtr Camera::capture()
{
  v4l2_buffer buf;
  bzero(&buf, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  // dequeue buffer with a frame (blocking operation)
  if( ioctl(fd_.get(), VIDIOC_DQBUF, &buf) == -1)
    BUT_THROW(Error, "Camera::capture(): ioctl(VIDIOC_DQBUF) dequeuing failed: " << strerror(errno));
  assert( buf.bytesused <= buffers_.at(buf.index).size_ && "syscall returned more bytes that fits the buffer" );

  // copy as shared ptr and return
  auto const ptr  = static_cast<uint8_t const*>( buffers_.at(buf.index).ptr_ );
  auto const size = buf.bytesused;
  auto const frame = std::make_shared<Jpeg>( std::vector<uint8_t>(ptr, ptr + size) );

  // re-enqueue buffer back for the next capture
  if( ioctl(fd_.get(), VIDIOC_QBUF, &buf) == -1)
    BUT_THROW(Error, "Camera::capture(): ioctl(VIDIOC_QBUF) re-enqueuing failed: " << strerror(errno));

  return frame;
}


Camera::Mmap_buf::Mmap_buf(But::System::Descriptor const& fd, size_t const size, uint32_t const offset):
  size_{size},
  ptr_{ mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd.get(), offset) }
{
  if(not ptr_)
    BUT_THROW(Error, "mmap() failed: " << strerror(errno));
}


Camera::Mmap_buf::Mmap_buf(Mmap_buf && other):
  size_{ std::exchange(other.size_, 0) },
  ptr_{ std::exchange(other.ptr_, nullptr) }
{
}


Camera::Mmap_buf& Camera::Mmap_buf::operator=(Mmap_buf &&other)
{
  close();
  size_ = std::exchange(other.size_, 0);
  ptr_ = std::exchange(other.ptr_, nullptr);
  return *this;
}


void Camera::Mmap_buf::close() noexcept
{
  if(not ptr_)
    return;
  munmap(ptr_, size_);
}

//  size_t size_{0};
//  void *ptr_{nullptr};
