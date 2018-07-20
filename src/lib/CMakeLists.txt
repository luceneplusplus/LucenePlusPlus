file(GLOB_RECURSE liblucene_sources
  ${CMAKE_SOURCE_DIR}/format/lucene.pb.cc
  *.cpp)

add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/format/lucene.pb.cc
  COMMAND ${PROTOBUF_EXECUTABLE} lucene.proto --cpp_out=.
  DEPENDS ${CMAKE_SOURCE_DIR}/format/lucene.proto
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/format)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/core/include)
include_directories(${CMAKE_SOURCE_DIR}/src/contrib/include)
include_directories(${CMAKE_SOURCE_DIR}/format)

set_property(SOURCE lib/LibLucene.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_SOURCE_DIR}/format/lucene.pb.cc)

add_library(lucenelib SHARED
  ${liblucene_sources}
)


target_link_libraries(lucenelib
  lucene++
  lucene++-contrib
  ${PROTOBUF_STATIC_LIB})

install(TARGETS lucenelib
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime
)
