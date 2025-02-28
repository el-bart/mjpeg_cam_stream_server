cmake_minimum_required(VERSION 3.25)
project(mjpeg_cam_stream_server)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(show_camera_feed show_camera_feed.cpp)
target_link_libraries(show_camera_feed ${OpenCV_LIBS})

add_executable(write_to_jpeg write_to_jpeg.cpp)
target_link_libraries(write_to_jpeg ${OpenCV_LIBS})

add_executable(check_resolutions check_resolutions.cpp)
