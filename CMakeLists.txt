cmake_minimum_required(VERSION 3.14)
project(joycond)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(LIBUDEV REQUIRED libudev)

add_executable(joycond "")
include_directories(
    include/
    ${LIBEVDEV_INCLUDE_DIRS}
    ${LIBUDEV_INCLUDE_DIRS}
    )
target_link_libraries(
    joycond
    ${LIBEVDEV_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    )

add_subdirectory(src)

