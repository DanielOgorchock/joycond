cmake_minimum_required(VERSION 3.13)
project(joycond)

include(GNUInstallDirs)

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

install(TARGETS joycond DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

pkg_get_variable(UDEV_RULES_PATH udev udev_dir)
if(NOT UDEV_RULES_PATH)
    set(UDEV_RULES_PATH "${CMAKE_INSTALL_PREFIX}/lib/udev")
endif()
install(FILES udev/89-joycond.rules udev/72-joycond.rules DESTINATION "${UDEV_RULES_PATH}/rules.d/"
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )

pkg_get_variable(SYSTEMD_SYSTEM_UNIT_PATH systemd systemd_system_unit_dir)
if(NOT SYSTEMD_SYSTEM_UNIT_PATH)
    execute_process(COMMAND systemd-path systemd-system-unit OUTPUT_VARIABLE SYSTEMD_SYSTEM_UNIT_PATH OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE UNIT_RETVAL)
    if(NOT UNIT_RETVAL EQUAL 0)
        set(SYSTEMD_SYSTEM_UNIT_PATH "${CMAKE_INSTALL_PREFIX}/lib/systemd/system")
    endif()
endif()
install(FILES systemd/joycond.service DESTINATION "${SYSTEMD_SYSTEM_UNIT_PATH}"
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )

pkg_get_variable(SYSTEMD_MODULES_LOAD_PATH systemd modules_load_dir)
if(NOT SYSTEMD_MODULES_LOAD_PATH)
    execute_process(COMMAND systemd-path modules-load OUTPUT_VARIABLE SYSTEMD_MODULES_LOAD_PATH OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE MODULES_RETVAL)
    if(NOT MODULES_RETVAL EQUAL 0)
        set(SYSTEMD_MODULES_LOAD_PATH "${CMAKE_INSTALL_PREFIX}/lib/modules-load.d")
    endif()
endif()
install(FILES systemd/joycond.conf DESTINATION "${SYSTEMD_MODULES_LOAD_PATH}"
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )

install(FILES data/com.github.DanielOgorchock.joycond.metainfo.xml
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        )
