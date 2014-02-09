project(lucene++-tester)

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
  ${lucene++-tester_SOURCE_DIR}/../include/*.h
  ${lucene++-tester_SOURCE_DIR}/include/*.h
)

include_directories(
  ${lucene++_SOURCE_DIR}/include
  ${lucene++-lib_SOURCE_DIR}/include
  ${lucene++-contrib_SOURCE_DIR}/include
  ${lucene++-tester_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

add_definitions(-DLPP_EXPOSE_INTERNAL)

add_executable(lucene++-tester
  ${tester_sources}
  ${test_headers}
)

target_link_libraries(lucene++-tester
  ${CMAKE_THREAD_LIBS_INIT}
  lucene++
  lucene++-contrib
  ${lucene_boost_libs}
)

add_test(lucene++-tester
  ${EXECUTABLE_OUTPUT_PATH}/lucene++-tester -p --test_dir=${lucene++-tester_SOURCE_DIR}/testfiles
)
