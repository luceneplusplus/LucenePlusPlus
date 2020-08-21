project(tester)


####################################
# create test bin target
####################################
add_executable(lucene++-tester)


####################################
# configure GTest
####################################
if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

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
    "${lucene++-tester_SOURCE_DIR}/include/*.h")

target_sources(lucene++-tester
  PRIVATE
    ${tester_sources})


####################################
# include directories
####################################
target_include_directories(lucene++-tester
  PUBLIC
    $<BUILD_INTERFACE:${googletest_SOURCE_DIR}/googletest/include>
    $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${lucene++_SOURCE_DIR}/include/lucene++>
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${core_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${contrib_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)


####################################
# dependencies
####################################
target_link_libraries(lucene++-tester
  PRIVATE
    Boost::boost
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::regex
    Boost::system
    Boost::thread
    ZLIB::ZLIB
    gtest_main
    gtest
    
    lucene++::lucene++
    lucene++::lucene++-contrib)


####################################
# link args
####################################
target_compile_options(lucene++-tester PRIVATE -DLPP_EXPOSE_INTERNAL)

cotire(lucene++-tester)
