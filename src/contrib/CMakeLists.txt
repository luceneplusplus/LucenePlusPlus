project(lucene++-contrib)

file(GLOB_RECURSE contrib_sources
  analyzers/*.cpp
  highlighter/*.cpp
  memory/*.cpp
  snowball/*.cpp
  snowball/libstemmer_c/libstemmer/libstemmer_utf8.c
  snowball/libstemmer_c/src_c/*.c
  snowball/libstemmer_c/runtime/*.c
)

file(GLOB_RECURSE contrib_headers
  "${lucene++-contrib_SOURCE_DIR}/include/*.h"
)

add_definitions(-DLPP_BUILDING_LIB)

include_directories(
  "${lucene++_SOURCE_DIR}/include"
  "${lucene++-lib_SOURCE_DIR}/include"
  "${lucene++-contrib_SOURCE_DIR}/include"
  "${lucene++-contrib_SOURCE_DIR}/snowball/libstemmer_c/include"
  ${Boost_INCLUDE_DIRS}
)

link_directories(
  ${Boost_LIBRARY_DIRS}
)

install(FILES ${contrib_headers}
  DESTINATION
  include/lucene++
  COMPONENT development-contrib
)

add_library(lucene++-contrib SHARED
  ${contrib_sources}
  ${contrib_headers}
)

set_target_properties(lucene++-contrib PROPERTIES
  VERSION ${lucene++_VERSION}
  SOVERSION ${lucene++_SOVERSION}
  COMPILE_FLAGS -DLPP_HAVE_DLL
)

target_link_libraries(lucene++-contrib
  ${CMAKE_THREAD_LIBS_INIT}
  lucene++
)

cotire(lucene++-contrib)

install(TARGETS lucene++-contrib
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime
)
