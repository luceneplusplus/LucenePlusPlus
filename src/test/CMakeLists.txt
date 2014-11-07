project(lucene++-tester)

add_subdirectory(gtest)

include_directories(
  "${gtest_SOURCE_DIR}/include"
  "${lucene++_SOURCE_DIR}/include"
  "${lucene++-lib_SOURCE_DIR}/include"
  "${lucene++-contrib_SOURCE_DIR}/include"
  "${lucene++-tester_SOURCE_DIR}/include"
  ${Boost_INCLUDE_DIRS}
)

file(GLOB_RECURSE tester_sources
  analysis/*.cpp
  contrib/*.cpp
  document/*.cpp
  index/*.cpp
  main/*.cpp
  queryparser/*.cpp
  search/*.cpp
  store/*.cpp
  util/*.cpp
)

file(GLOB_RECURSE test_headers
  "${lucene++-tester_SOURCE_DIR}/../include/*.h"
  "${lucene++-tester_SOURCE_DIR}/include/*.h"
)

add_definitions(-DLPP_EXPOSE_INTERNAL)

link_directories(
  "${GTEST_LIB_DIR}"
)

add_executable(lucene++-tester
  ${tester_sources}
  ${test_headers}
)

add_dependencies(lucene++-tester
  gtest
)

target_link_libraries(lucene++-tester
  ${CMAKE_THREAD_LIBS_INIT}
  lucene++
  lucene++-contrib
  gtest
  ${lucene_boost_libs}
)

cotire(lucene++-tester)
