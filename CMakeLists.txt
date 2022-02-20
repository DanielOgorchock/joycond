cmake_minimum_required(VERSION 3.13)
project(joycond)

set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(PkgConfig)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(LIBUDEV REQUIRED libudev)

add_executable(joycond "")
target_compile_options(joycond PRIVATE -Wall -Werror)
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

install(TARGETS joycond DESTINATION /usr/bin/
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
execute_process(COMMAND pkg-config --variable=udevdir udev OUTPUT_VARIABLE UDEV_RULES_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
install(FILES udev/89-joycond.rules udev/72-joycond.rules DESTINATION ${UDEV_RULES_PATH}/rules.d/
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ 
        )
execute_process(COMMAND systemd-path systemd-system-unit OUTPUT_VARIABLE SYSTEMD_SYSTEM_UNIT_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
install(FILES systemd/joycond.service DESTINATION ${SYSTEMD_SYSTEM_UNIT_PATH}
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )
execute_process(COMMAND systemd-path modules-load OUTPUT_VARIABLE SYSTEMD_MODULES_LOAD_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
install(FILES systemd/joycond.conf DESTINATION ${SYSTEMD_MODULES_LOAD_PATH}
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )
