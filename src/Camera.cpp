#include "Camera.hpp"
#include <sstream>

Camera::Camera(Camera_config const& cfg):
  dev_{cfg.video_device_}
{
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
}

std::shared_ptr<cv::Mat> Camera::capture()
{
  auto frame = std::make_shared<cv::Mat>();
  if( not dev_.read(*frame) )
    throw std::runtime_error{"Camera::capture(): failed to capture frame"};
  return frame;
}
