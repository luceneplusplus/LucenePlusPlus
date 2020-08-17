project(core)

####################################
# create library target
####################################
if (LUCENE_BUILD_SHARED)
  add_library(lucene++ SHARED)
else()
  add_library(lucene++ STATIC)
endif()

add_library(lucene++::lucene++ ALIAS lucene++)


####################################
# src
####################################
file(GLOB_RECURSE lucene_sources
  "search/*.cpp"
  "analysis/*.cpp"
  "document/*.cpp"
  "index/*.cpp"
  "queryparser/*.cpp"
  "store/*.cpp"
  "util/*.c*"
)

file(GLOB_RECURSE lucene_internal_headers
  "${lucene++-lib_SOURCE_DIR}/include/*.h"
)


target_sources(lucene++
  PRIVATE
    ${lucene_sources}
    ${lucene_internal_headers})


####################################
# include directories
####################################
target_include_directories(lucene++
  PUBLIC
    $<BUILD_INTERFACE:${lucene++_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${lucene++_SOURCE_DIR}/include/lucene++>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${Boost_INCLUDE_DIRS})



####################################
# dependencies
####################################
target_link_libraries(lucene++
    Boost::boost
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::regex
    Boost::system
    Boost::thread
    ZLIB::ZLIB
)

if(WIN32)
    target_link_libraries(lucene++ ws2_32)
endif()

####################################
# link args
####################################
target_compile_options(lucene++ PRIVATE -DLPP_BUILDING_LIB)

set_target_properties(lucene++
  PROPERTIES
    COTIRE_CXX_PREFIX_HEADER_INIT "include/LuceneInc.h"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
    VERSION ${lucene++_VERSION}
    SOVERSION ${lucene++_SOVERSION})

cotire(lucene++)


install(TARGETS lucene++
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime)
