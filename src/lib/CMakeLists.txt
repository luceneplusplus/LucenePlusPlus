file(GLOB_RECURSE liblucene_sources
  *.cpp)

add_custom_target(lucene_pb)

add_custom_command(TARGET lucene_pb
  PRE_BUILD
  COMMAND ${PROTOBUF_EXECUTABLE} lucene.proto --cpp_out=.
  DEPENDS ${CMAKE_SOURCE_DIR}/format/lucene.proto
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/format)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/core/include)
include_directories(${CMAKE_SOURCE_DIR}/src/contrib/include)
include_directories(${CMAKE_SOURCE_DIR}/format)

add_library(lucenelib SHARED
  ${liblucene_sources}
  ${CMAKE_SOURCE_DIR}/format/lucene.pb.cc
)

add_dependencies(lucenelib lucene_pb)

target_link_libraries(lucenelib
  lucene++
  lucene++-contrib
  ${PROTOBUF_STATIC_LIB})

install(TARGETS lucenelib
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime
)
