include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/core/include)
include_directories(${CMAKE_SOURCE_DIR}/src/contrib/include)
include_directories(${CMAKE_SOURCE_DIR}/format)
include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${CMAKE_SOURCE_DIR}/format/lucene.proto)
set_property(SOURCE lib/LibLucene.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_SOURCE_DIR}/format/lucene.pb.cc)

add_library(lucenelib SHARED
  ${liblucene_sources}
  ${PROTO_HEADER} ${PROTO_SRC}
)

target_link_libraries(lucenelib
  lucene++
  lucene++-contrib
  ${PROTOBUF_LIB})

install(TARGETS lucenelib
  DESTINATION "${LIB_DESTINATION}"
  COMPONENT runtime
)
