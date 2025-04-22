cmake_minimum_required(VERSION 3.25)
project(mjpeg_cam_stream_server)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(show_camera_feed mtest/show_camera_feed.cpp)
target_link_libraries(show_camera_feed ${OpenCV_LIBS})

add_executable(write_to_jpeg mtest/write_to_jpeg.cpp)
target_link_libraries(write_to_jpeg ${OpenCV_LIBS})

add_executable(check_resolutions mtest/check_resolutions.cpp)

add_library(common
  program_options.cpp
)
target_link_libraries(common
  ${OpenCV_LIBS}
  boost_program_options
)

add_executable(mjpeg_cam_stream_server main.cpp)
target_link_libraries(mjpeg_cam_stream_server common)
