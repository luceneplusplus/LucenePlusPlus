project(lucene++-base)

####################################
# VERSION information
#These versions match the Lucene version
SET(LUCENE++_VERSION_MAJOR "3")
SET(LUCENE++_VERSION_MINOR "0")
SET(LUCENE++_VERSION_REVISION "3")
SET(LUCENE++_VERSION_PATCH "4")

# SOVERSION information
#Must be incremented for releases if the api is not backwards compatible
SET(LUCENE++_SOVERSION "0")

#derived versions
MATH(EXPR LUCENE++_INT_VERSION "(${LUCENE++_VERSION_MAJOR} * 1000000) + (${LUCENE++_VERSION_MINOR} * 10000) + (${LUCENE++_VERSION_REVISION} * 100) + (${LUCENE++_VERSION_PATCH} * 1)" )
SET(LUCENE++_VERSION "${LUCENE++_VERSION_MAJOR}.${LUCENE++_VERSION_MINOR}.${LUCENE++_VERSION_REVISION}.${LUCENE++_VERSION_PATCH}")
MESSAGE(${LUCENE++_INT_VERSION})
MESSAGE(${LUCENE++_VERSION})

####################################

####################################
# Build system options and includes
####################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
#build policies
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# include specific modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#if setup using the Toolchain-llvm.cmake file, then use llvm...
IF ( ENABLE_LLVM )
  INCLUDE (Toolchain-llvm)
ENDIF ( ENABLE_LLVM )

#define options...
INCLUDE (Lucene++Docs)
INCLUDE (FindThreads)
INCLUDE (TestCXXAcceptsFlag)
ENABLE_TESTING()

#Single output directory for building all executables and libraries.
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Executable Output Directory" FORCE)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Library Output Directory" FORCE)
####################################

####################################
#user specified build options
####################################
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ELSE(NOT CMAKE_BUILD_TYPE)
    MESSAGE( "Compiling as ${CMAKE_BUILD_TYPE}" )
ENDIF(NOT CMAKE_BUILD_TYPE)

OPTION(ENABLE_PACKAGING
  "create build scripts for creating lucene++ packages"
  OFF)
OPTION(ENABLE_NEDMALLOC
  "use nedmalloc for memory allocations"
  OFF)
OPTION(LUCENE_USE_STATIC_BOOST_LIBS
  "use static boost libraries "
  OFF)
OPTION(ENABLE_CYCLIC_CHECK
  "enable cyclic checking "
  OFF)

#install path options
SET(LIB_DESTINATION "lib" CACHE STRING "Define lib output directory name")

IF ( ENABLE_NEDMALLOC )
  ADD_DEFINITIONS(-DLPP_USE_NEDMALLOC)
ENDIF ( ENABLE_NEDMALLOC )
IF ( ENABLE_CYCLIC_CHECK )
  ADD_DEFINITIONS(-DLPP_USE_CYCLIC_CHECK)
ENDIF ( ENABLE_CYCLIC_CHECK )
####################################


####################################
# PLATFORM specific options
####################################
#add a debug build postfix
if(WIN32 OR WIN64)
 set(CMAKE_DEBUG_POSTFIX "d")
endif(WIN32 OR WIN64)

if(NOT MSVC AND NOT CMAKE_SYSTEM MATCHES "SunOS-5*.")
        add_definitions(-fPIC)
endif(NOT MSVC AND NOT CMAKE_SYSTEM MATCHES "SunOS-5*.")

INCLUDE(MacroCheckGccVisibility)
MACRO_CHECK_GCC_VISIBILITY(LPP_HAVE_GXXCLASSVISIBILITY)
if ( LPP_HAVE_GXXCLASSVISIBILITY )
	ADD_DEFINITIONS(-DLPP_HAVE_GXXCLASSVISIBILITY)
endif()

IF(CYGWIN)
  ADD_DEFINITIONS(-D__LARGE64_FILES)
ENDIF(CYGWIN)

#set ansi mode 
SET(ENABLE_ANSI_MODE OFF)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(ENABLE_ANSI_MODE ON)
  
  #exceptions:
  IF(MINGW OR CYGWIN)
    SET(ENABLE_ANSI_MODE OFF)
  ENDIF(MINGW OR CYGWIN)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
IF ( CMAKE_COMPILER_IS_GNUCC )
   IF( ENABLE_ANSI_MODE )
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi")
   ENDIF ( ENABLE_ANSI_MODE )
ENDIF(CMAKE_COMPILER_IS_GNUCC) 

####################################
#find boost
####################################
SET(Boost_USE_STATIC_LIBS   ${LUCENE_USE_STATIC_BOOST_LIBS})
SET(Boost_USE_MULTITHREADED ON)
#Boost 1.38 required for bug fixes in basic_streambuf. 
#The following line fails in earlier builds, so if altered, may allow older versions of boost:
#boost::gregorian::date date = parser.parse_date(paddedDate.c_str(), dateFormat->c_str(), svp);
find_package( Boost 1.38.0 COMPONENTS date_time filesystem iostreams regex system thread unit_test_framework REQUIRED)
IF (Boost_FOUND)
  MESSAGE( STATUS "boost found: includes in ${Boost_INCLUDE_DIRS}, library in ${Boost_LIBRARY_DIRS}")
  SET(LUCENE_BOOST_LIBS 
    ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    ${Boost_IOSTREAMS_LIBRARY_RELEASE}
    ${Boost_REGEX_LIBRARY_RELEASE}
    ${Boost_SYSTEM_LIBRARY_RELEASE}
    ${Boost_THREAD_LIBRARY_RELEASE})
ENDIF (Boost_FOUND)

####################################
# Pre-Compiled headers
####################################
INCLUDE(PCHSupport)

#todo: make this optional and make it possible to add more headers - like boost threads


####################################
# The subdirs
####################################
#include sub-projects
ADD_SUBDIRECTORY (src/core)
ADD_SUBDIRECTORY (src/contrib EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY (src/demo EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY (src/test)


#################################
# install pkg-config file
#################################
IF(NOT WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/liblucene++.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/liblucene++.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liblucene++.pc
          DESTINATION ${LIB_DESTINATION}/pkgconfig )
ENDIF(NOT WIN32)


####################################
# Custom targets
####################################
#add uninstall command
CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


####################################
# Finalise build script
####################################

#this must go last...
IF (ENABLE_PACKAGING)
  INCLUDE(CreateLucene++Packages)
ENDIF ( ENABLE_PACKAGING)
