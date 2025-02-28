cmake_minimum_required(VERSION 3.25)
project(mjpeg_cam_stream_server)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(show_camera_feed show_camera_feed.cpp)
target_link_libraries(show_camera_feed ${OpenCV_LIBS})
