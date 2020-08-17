project(contrib)

####################################
# create library target
####################################
if (LUCENE_BUILD_STATIC)
  add_library(lucene++-contrib STATIC)
else()
  add_library(lucene++-contrib SHARED)
endif()

add_library(lucene++::lucene++-contrib ALIAS lucene++-contrib)


####################################
# src
####################################
file(GLOB_RECURSE contrib_sources
  "analyzers/*.cpp"
  "highlighter/*.cpp"
  "memory/*.cpp"
  "index/*.cpp"
  "mevc/*.cpp"
  "snowball/*.cpp"
  "snowball/libstemmer_c/libstemmer/libstemmer_utf8.c"
  "snowball/libstemmer_c/src_c/*.c"
  "snowball/libstemmer_c/runtime/*.c"
)

file(GLOB_RECURSE contrib_headers
  "${lucene++-lib_SOURCE_DIR}/include/*.h"
)



target_sources(lucene++-contrib
  PRIVATE
    ${contrib_sources})


####################################
# include directories
####################################
target_include_directories(lucene++-contrib
  PUBLIC
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${lucene++_SOURCE_DIR}/include/lucene++>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${lucene++-contrib_SOURCE_DIR}/snowball/libstemmer_c/include>
    ${Boost_INCLUDE_DIRS}
)



####################################
# dependencies
####################################
target_link_libraries(lucene++-contrib
    Boost::boost
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::regex
    Boost::system
    Boost::thread
    ZLIB::ZLIB
    lucene++::lucene++
)

if(WIN32)
    target_link_libraries(lucene++-contrib ws2_32)
endif()


####################################
# link args
####################################
target_compile_options(lucene++-contrib PUBLIC -DLPP_BUILDING_LIB)

set_target_properties(lucene++
  PROPERTIES
    COTIRE_CXX_PREFIX_HEADER_INIT "include/ContribInc.h")

cotire(lucene++-contrib)

set_target_properties(lucene++-contrib PROPERTIES
  VERSION ${lucene++_VERSION}
  SOVERSION ${lucene++_SOVERSION})

install(TARGETS lucene++-contrib
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime)

install(
  FILES ${contrib_headers}
  DESTINATION "include/lucene++"
  COMPONENT development-contrib)
