cmake_minimum_required(VERSION 3.25)
project(mjpeg_cam_stream_server)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Werror)

option(ENABLE_PROFILING "Enable GNU profiling with gprof" OFF)
if(ENABLE_PROFILING)
  message(STATUS "MJPEG server: enabling profiling")
  add_compile_options(-pg)
  add_link_options(-pg)
  add_compile_definitions(GPROF=1)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(show_camera_feed mtest/show_camera_feed.cpp)
add_executable(write_to_jpeg mtest/write_to_jpeg.cpp)
add_executable(check_resolutions mtest/check_resolutions.cpp)

add_library(common
  Camera.cpp
  Client_handler.cpp
  Server.cpp
  program_options.cpp
)
target_link_libraries(common
  but
  boost_program_options
)

add_executable(mjpeg_cam_stream_server main.cpp)
target_link_libraries(mjpeg_cam_stream_server common)
