####################################
# install headers
####################################

file(GLOB_RECURSE lucene_headers
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

install(
  FILES ${lucene_headers}
  DESTINATION include/lucene++ )
