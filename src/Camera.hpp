#pragma once
#include "program_options.hpp"
#include "Jpeg.hpp"
#include <memory>
#include <opencv2/opencv.hpp>

struct Camera final
{
  explicit Camera(Camera_config const& cfg);

  Camera(Camera&&) = default;
  Camera& operator=(Camera&&) = default;

  Camera(Camera const&) = delete;
  Camera& operator=(Camera const&) = delete;

  // returns next frame from the camera in the JPEG format
  JpegPtr capture();

private:
  cv::VideoCapture dev_;
  cv::Mat buffer_;
  std::vector<int> const params_{cv::IMWRITE_JPEG_QUALITY, 90}; // JPEG quality
};
