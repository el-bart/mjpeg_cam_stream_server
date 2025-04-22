#pragma once
#include "program_options.hpp"
#include <memory>
#include <opencv2/opencv.hpp>

struct Camera final
{
  explicit Camera(Camera_config const& cfg);

  Camera(Camera&&) = default;
  Camera& operator=(Camera&&) = default;

  Camera(Camera const&) = delete;
  Camera& operator=(Camera const&) = delete;

  std::shared_ptr<cv::Mat> capture();

private:
  cv::VideoCapture dev_;
};
