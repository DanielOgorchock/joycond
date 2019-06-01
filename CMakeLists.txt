cmake_minimum_required(VERSION 3.14)
project(joycond)

set(CMAKE_CXX_STANDARD 17)

add_executable(joycond "")
include_directories(include/)
target_link_libraries(
    joycond
    evdev
    )

add_subdirectory(src)

