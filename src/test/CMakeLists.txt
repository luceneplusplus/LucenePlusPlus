project(tester)

####################################
# create test bin target
####################################
add_executable(lucene++-tester)

add_subdirectory(gtest)

####################################
# src
####################################
file(GLOB_RECURSE tester_sources
  "analysis/*.cpp"
  "contrib/*.cpp"
  "document/*.cpp"
  "index/*.cpp"
  "main/*.cpp"
  "queryparser/*.cpp"
  "search/*.cpp"
  "store/*.cpp"
  "util/*.cpp")

file(GLOB_RECURSE test_headers
    "${lucene++-tester_SOURCE_DIR}/../include/*.h"
    "${lucene++-tester_SOURCE_DIR}/include/*.h")

target_sources(lucene++-tester
  PRIVATE
    ${tester_sources}
    ${test_headers})


####################################
# include directories
####################################
target_include_directories(lucene++-tester
  PUBLIC
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    ${lucene++_SOURCE_DIR}/include/lucene++>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

    ${gtest_SOURCE_DIR}/include/gtest>
    ${gtest_SOURCE_DIR}/include/gtest/internal>
    ${core_SOURCE_DIR}/include>
    ${contrib_SOURCE_DIR}/include>
    ${Boost_INCLUDE_DIRS}>)


####################################
# dependencies
####################################

target_link_libraries(lucene++-tester
    ${lucene_boost_libs}
    ZLIB::ZLIB
    gtest
    lucene++::lucene++
    lucene++::lucene++-contrib)


####################################
# link args
####################################
target_compile_options(lucene++-tester PRIVATE -DLPP_EXPOSE_INTERNAL)
target_compile_options(lucene++-tester PRIVATE -DLPP_BUILDING_LIB)

cotire(lucene++-tester)
