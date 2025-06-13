#include "Camera.hpp"
#include <sys/mman.h>

Camera::Camera(Logger log, Camera_config const& cfg):
  log_{ std::move(log) }//,
  //dev_{cfg.video_device_}
{
  (void)cfg;
  /*
    if( not dev_.isOpened() )
      throw std::runtime_error{"Camera: failed to open device: " + cfg.video_device_.string()};
    dev_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    dev_.set(cv::CAP_PROP_FRAME_WIDTH, cfg.capture_resolution_.x_);
    dev_.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.capture_resolution_.y_);

    auto x = dev_.get(cv::CAP_PROP_FRAME_WIDTH);
    auto y = dev_.get(cv::CAP_PROP_FRAME_HEIGHT);
    if( x != cfg.capture_resolution_.x_ || y != cfg.capture_resolution_.y_ )
    {
      std::stringstream ss;
      ss << "Camera: failed to set requested " << cfg.capture_resolution_.x_ << "x" << cfg.capture_resolution_.y_ << " - ";
      ss << "instead got " << x << "x" << y;
      throw std::runtime_error{ ss.str() };
    }
    */
}


JpegPtr Camera::capture()
{
  throw 42;
  /*
  if( not dev_.read(buffer_) )
    throw std::runtime_error{"Camera::capture(): failed to capture frame"};
  return to_jpeg(buffer_, params_);
  */
}

Camera::Mmap_buf::Mmap_buf(size_t size, uint32_t offset)
{
  (void)size;
  (void)offset;
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
