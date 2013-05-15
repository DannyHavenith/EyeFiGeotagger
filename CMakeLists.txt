
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)


cmake_minimum_required(VERSION 2.6)

project( geotagger)

add_definitions(-D_SCL_SECURE_NO_WARNINGS)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

FIND_PACKAGE( Boost COMPONENTS system filesystem regex)

# NOTE: I assume that exiv2 (exiv2-dev) is available to the compiler and linker 
# (on linux: just install libexiv2-dev using your package manager).
# cpp-netlib is pulled in as a git submodule
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/cpp-netlib)

add_executable( geotagger
	geotagger.cpp
	exif_tagging.cpp
	location_http_request.cpp

# add header files for visual studio, cmake will understand.
	exif_tagging.hpp
	location_http_request.hpp
	)

target_link_libraries( geotagger ${Boost_LIBRARIES} exiv2)

set(CPACK_PACKAGE_NAME "geotagger")
set(CPACK_PACKAGE_VENDOR "havenith.net")
set(CPACK_PACKAGE_CONTACT "dh.geotagger@havenith.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "geotagger, add geo tags to jpegs using EyeFi access point logs")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "geotagger")
set(CPACK_PACKAGE_EXECUTABLES "geotagger;napl executables")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE_1_0.txt")
include(InstallRequiredSystemLibraries)
include(CPack)
