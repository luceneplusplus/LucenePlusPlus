project(lucene++)

cmake_minimum_required(VERSION 2.8.6)

set(lucene++_VERSION_MAJOR 3)
set(lucene++_VERSION_MINOR 0)
set(lucene++_VERSION_PATCH 7)

set(lucene++_SOVERSION "0")

set(lucene++_VERSION
  "${lucene++_VERSION_MAJOR}.${lucene++_VERSION_MINOR}.${lucene++_VERSION_PATCH}"
)

# include specific modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

####################################
# pre-compiled headers support
####################################
include(cotire)

# if setup using the Toolchain-llvm.cmake file, then use llvm...
if(ENABLE_LLVM)
  include(Toolchain-llvm)
endif()

####################################
# user specified build options
####################################
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

option(ENABLE_PACKAGING
  "Create build scripts for creating lucene++ packages"
  OFF
)
option(LUCENE_USE_STATIC_BOOST_LIBS
  "Use static boost libraries"
  OFF
)
option(ENABLE_CYCLIC_CHECK
  "Enable cyclic checking"
  OFF
)
option(ENABLE_TEST
  "Enable the tests"
  ON
)
option(ENABLE_DEMO
  "Enable building demo applications"
  ON
)

####################################
# bootstrap
####################################

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS
  date_time
  filesystem
  iostreams
  regex
  system
  thread
  REQUIRED
)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ${LUCENE_USE_STATIC_BOOST_LIBS})

set(lucene_boost_libs
  ${Boost_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARIES}
  ${Boost_IOSTREAMS_LIBRARIES}
  ${Boost_REGEX_LIBRARIES}
  ${Boost_SYSTEM_LIBRARIES}
  ${Boost_THREAD_LIBRARIES}
)

include(Lucene++Docs)
include(TestCXXAcceptsFlag)
include(GNUInstallDirs)

set(LIB_DESTINATION
  "${CMAKE_INSTALL_FULL_LIBDIR}" CACHE STRING "Define lib output directory name"
)

if(ENABLE_CYCLIC_CHECK)
  set(DEFINE_USE_CYCLIC_CHECK "define")
else()
  set(DEFINE_USE_CYCLIC_CHECK "undef")
endif()

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

#################################
# generate Config.h
#################################
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/Config.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/include/Config.h" @ONLY
)

include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}/include"
)


add_subdirectory(src/core)
add_subdirectory(src/contrib)

if(ENABLE_DEMO)
  add_subdirectory(src/demo)
endif()

if(ENABLE_TEST)
  enable_testing()
  add_subdirectory(src/test)
endif()

#################################
# install pkg-config file
#################################
if(NOT WIN32)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/liblucene++.pc.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++.pc" @ONLY)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/liblucene++-contrib.pc.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contrib.pc" @ONLY)
  install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++.pc"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contrib.pc"
    DESTINATION "${LIB_DESTINATION}/pkgconfig")
endif()

#################################
# install Config.h
#################################
install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/include/Config.h"
  DESTINATION include/lucene++)

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

message("** Build Summary **")
message("  Version:        ${lucene++_VERSION}")
message("  Prefix:         ${CMAKE_INSTALL_PREFIX}")
message("  Build Type:     ${CMAKE_BUILD_TYPE}")
message("  Architecture:   ${CMAKE_SYSTEM_PROCESSOR}")
message("  System:         ${CMAKE_SYSTEM_NAME}")
message("  Boost Include:  ${Boost_INCLUDE_DIRS}")
message("  Boost Library:  ${Boost_LIBRARY_DIRS}")
