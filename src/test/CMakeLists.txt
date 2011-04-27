project(lucene++-tester)

####################################
# THE lucene++tester library
####################################
file(GLOB_RECURSE lucene_sources
  ${lucene++-tester_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS ${lucene++-tester_SOURCE_DIR}/../include/*.h)
file(GLOB_RECURSE HEADERS ${lucene++-tester_SOURCE_DIR}/include/*.h)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${lucene++-base_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${lucene++-lib_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${lucene++-contrib_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${lucene++-tester_SOURCE_DIR}/include)
ADD_DEFINITIONS(-DLPP_EXPOSE_INTERNAL)

ADD_EXECUTABLE(lucene++-tester EXCLUDE_FROM_ALL
	${lucene_sources} ${HEADERS}
)

#set properties on the libraries
SET_TARGET_PROPERTIES(lucene++-tester PROPERTIES
    VERSION ${LUCENE++_VERSION}
    SOVERSION ${LUCENE++_SOVERSION}
)
TARGET_LINK_LIBRARIES(lucene++-tester 
  lucene++-static 
  lucene++-c 
  lucene++-contrib-static 
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE}
  ${LUCENE_BOOST_LIBS} )

ADD_TEST(${EXECUTABLE_OUTPUT_PATH}/lucene++-tester ${EXECUTABLE_OUTPUT_PATH}/lucene++-tester -p)

