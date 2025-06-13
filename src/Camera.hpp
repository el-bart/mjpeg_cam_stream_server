#pragma once
#include "program_options.hpp"
#include "Jpeg.hpp"
#include "Logger.hpp"
#include <But/Exception.hpp>
#include <But/System/Descriptor.hpp>
#include <vector>

struct Camera final
{
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "Camera error");
  Camera(Logger log, Camera_config const& cfg);
  ~Camera();

  Camera(Camera&&) = default;
  Camera& operator=(Camera&&) = default;

  Camera(Camera const&) = delete;
  Camera& operator=(Camera const&) = delete;

  // returns next frame from the camera in the JPEG format
  JpegPtr capture();

private:
  struct Mmap_buf
  {
    Mmap_buf(But::System::Descriptor const& fd, size_t size, uint32_t offset);
    ~Mmap_buf() { close(); }
    Mmap_buf(Mmap_buf const&) = delete;
    Mmap_buf& operator=(Mmap_buf const&) = delete;
    Mmap_buf(Mmap_buf &&);
    Mmap_buf& operator=(Mmap_buf &&);

    void close() noexcept;

    size_t size_{0};
    void *ptr_{nullptr};
  };

  Logger log_;
  But::System::Descriptor fd_;
  std::vector<Mmap_buf> buffers_;
};
