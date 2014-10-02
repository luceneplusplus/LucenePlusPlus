project(lucene++-lib)

include_directories(
  "${lucene++_SOURCE_DIR}/include"
  "${lucene++-lib_SOURCE_DIR}/include"
  ${Boost_INCLUDE_DIRS}
)

file(GLOB_RECURSE lucene_sources
  search/*.cpp
  analysis/*.cpp
  document/*.cpp
  index/*.cpp
  queryparser/*.cpp
  store/*.cpp
  util/*.c*)

file(GLOB_RECURSE lucene_internal_headers
  "${lucene++-lib_SOURCE_DIR}/include/*.h"
)

file(GLOB_RECURSE lucene_headers
  "${lucene++_SOURCE_DIR}/include/*.h"
)

add_definitions(-DLPP_BUILDING_LIB)

add_library(lucene++ SHARED
  ${lucene_sources}
)

target_link_libraries(lucene++
  ${CMAKE_THREAD_LIBS_INIT}
  ${lucene_boost_libs}
)

if(WIN32)
    target_link_libraries(lucene++ ws2_32)
endif()

set_target_properties(lucene++ PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "include/LuceneInc.h")
cotire(lucene++)

set_target_properties(lucene++ PROPERTIES
  VERSION ${lucene++_VERSION}
  SOVERSION ${lucene++_SOVERSION}
)

install(FILES ${lucene_headers}
  DESTINATION include/lucene++
  COMPONENT development
)

install(TARGETS lucene++
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime
)
