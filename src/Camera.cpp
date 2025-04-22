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


namespace
{
auto to_jpeg(cv::Mat& frame, std::vector<int> const& params)
{
  // ensure correct encoding
  if (frame.type() != CV_8UC3)
  {
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB); // RGB
    frame.convertTo(frame, CV_8UC3); // 8-bit channels
  }

  auto jpeg = std::make_shared<Jpeg>();
  std::vector<uchar> buffer;
  cv::imencode(".jpg", frame, jpeg->data_, params);
  return jpeg;
}
}

JpegPtr Camera::capture()
{
  if( not dev_.read(buffer_) )
    throw std::runtime_error{"Camera::capture(): failed to capture frame"};
  return to_jpeg(buffer_, params_);
}
