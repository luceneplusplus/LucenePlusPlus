project(searchfiles)


####################################
# src
####################################
file(GLOB_RECURSE searchfiles_sources
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

file(GLOB_RECURSE demo_headers
  "${searchfiles_SOURCE_DIR}/../include/*.h")


####################################
# create executable target
####################################
add_executable(searchfiles
  ${searchfiles_sources})


####################################
# include directories
####################################
target_include_directories(searchfiles
  PRIVATE
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${lucene++_SOURCE_DIR}/include/lucene++>
    ${Boost_INCLUDE_DIRS})


####################################
# dependencies
####################################
target_link_libraries(searchfiles
  PRIVATE
    Boost::boost
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::regex
    Boost::system
    Boost::thread
    ZLIB::ZLIB
    lucene++::lucene++
    lucene++::lucene++-contrib)
