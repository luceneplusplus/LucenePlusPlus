####################################
# init
####################################

cmake_minimum_required(VERSION 3.5)

project(lucene++)

set(lucene++_VERSION_MAJOR 3)
set(lucene++_VERSION_MINOR 0)
set(lucene++_VERSION_PATCH 8)
set(lucene++_SOVERSION "0")

set(lucene++_VERSION
  "${lucene++_VERSION_MAJOR}.${lucene++_VERSION_MINOR}.${lucene++_VERSION_PATCH}")


# set default build type as release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(LIB_DESTINATION
  "${CMAKE_INSTALL_FULL_LIBDIR}" CACHE STRING "Define lib output directory name")


####################################
# CMake Modules
####################################

# include specific modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(options.cmake)

# pre-compiled headers support
include(cotire)

# if setup using the Toolchain-llvm.cmake file, then use llvm...
if(ENABLE_LLVM)
  include(Toolchain-llvm)
endif()

# fetch dependencies
include(dependencies)

# build docs
include(Lucene++Docs)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################
# platform specific options
####################################
if(WIN32 OR WIN64)
set(CMAKE_DEBUG_POSTFIX "d")
endif()

if(MSVC)
# Disable automatic boost linking on Windows as libraries are added to the linker explicitly
add_definitions(-DBOOST_ALL_NO_LIB)

# enable exceptions, see http://msdn.microsoft.com/en-us/library/1deeycx5.aspx
add_definitions(-EHsc)

# Disable including too many Windows headers
add_definitions(-DWIN32_LEAN_AND_MEAN)

# Disable the min/max macros that conflict with std::min/std::max
add_definitions(-DNOMINMAX)
endif()

if(NOT WIN32 AND NOT CMAKE_SYSTEM MATCHES "SunOS-5*.")
  add_definitions(-fPIC)
endif()

if(CYGWIN)
  add_definitions(-D__LARGE64_FILES)
endif()

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endif()


####################################
# custom targets
####################################
configure_file(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
)

add_custom_target(
  uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  VERBATIM
)

if(ENABLE_PACKAGING)
  include(CreateLucene++Packages)
endif()


####################################
# bootstrap
####################################

include(TestCXXAcceptsFlag)
include(GNUInstallDirs)

add_subdirectory(include)
add_subdirectory(src)

message("\n\n** Build Summary **")
message("  Version:          ${lucene++_VERSION}")
message("  Prefix:           ${CMAKE_INSTALL_PREFIX}")
message("  Build Type:       ${CMAKE_BUILD_TYPE}")
message("  Architecture:     ${CMAKE_SYSTEM_PROCESSOR}")
message("  System:           ${CMAKE_SYSTEM_NAME}")
message("  Boost Include:    ${Boost_INCLUDE_DIRS}")
message("  Boost Libraries:  ${Boost_LIBRARY_DIRS}")
message("  Zlib Include:     ${ZLIB_INCLUDE_DIRS}")
message("  Zlib Library:     ${ZLIB_LIBRARY_RELEASE}")
