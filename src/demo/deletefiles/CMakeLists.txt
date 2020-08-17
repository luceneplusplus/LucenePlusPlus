project(deletefiles)


####################################
# src
####################################
file(GLOB_RECURSE deletefiles_sources
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

file(GLOB_RECURSE demo_headers
  "${deletefiles_SOURCE_DIR}/../include/*.h")


####################################
# create executable target
####################################
add_executable(deletefiles
  ${deletefiles_sources})


####################################
# include directories
####################################
target_include_directories(deletefiles
  PRIVATE
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${lucene++_SOURCE_DIR}/include/lucene++>
    ${Boost_INCLUDE_DIRS})


####################################
# dependencies
####################################
target_link_libraries(deletefiles
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
