project(lucene++-demo)

file(GLOB_RECURSE
  demo_headers
  "${lucene++-demo_SOURCE_DIR}/../include/*.h"
)

add_definitions(-DLPP_HAVE_DLL)

include_directories(
  ${Boost_INCLUDE_DIRS}
)
include_directories(
  "${lucene++_SOURCE_DIR}/include"
)

add_executable(indexfiles
  "${lucene++-demo_SOURCE_DIR}/indexfiles/main.cpp"
  ${demo_headers}
)
target_link_libraries(indexfiles
  lucene++
)

add_executable(searchfiles
  "${lucene++-demo_SOURCE_DIR}/searchfiles/main.cpp"
  ${demo_headers}
)
target_link_libraries(searchfiles
  lucene++
)

add_executable(deletefiles
  "${lucene++-demo_SOURCE_DIR}/deletefiles/main.cpp"
  ${demo_headers}
)
target_link_libraries(deletefiles
  lucene++
)
