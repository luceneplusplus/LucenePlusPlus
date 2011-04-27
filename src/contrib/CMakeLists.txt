project(lucene++-contrib)

####################################
# THE lucene++-contrib library
####################################
file(GLOB_RECURSE lucene_sources
  ${lucene++-contrib_SOURCE_DIR}/*.cpp
  ${lucene++-contrib_SOURCE_DIR}/snowball/libstemmer_c/libstemmer/libstemmer_utf8.c
  ${lucene++-contrib_SOURCE_DIR}/snowball/libstemmer_c/src_c/*.c
  ${lucene++-contrib_SOURCE_DIR}/snowball/libstemmer_c/runtime/*.c)
file(GLOB_RECURSE HEADERS ${lucene++-contrib_SOURCE_DIR}/include/*.h)

ADD_DEFINITIONS(-DLPP_BUILDING_LIB)
INCLUDE_DIRECTORIES(${lucene++-base_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${lucene++-lib_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${lucene++-contrib_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

install(FILES HEADERS
		        DESTINATION include/lucene++
		        COMPONENT development-contrib)

#################################
# lucene++ static library
#################################
ADD_LIBRARY(lucene++-contrib-static STATIC EXCLUDE_FROM_ALL
	${lucene_sources} ${HEADERS}
)
#set properties on the libraries
SET_TARGET_PROPERTIES(lucene++-contrib-static PROPERTIES
    VERSION ${LUCENE++_VERSION}
    SOVERSION ${LUCENE++_SOVERSION}
)

#################################
# lucene++ shared library
#################################
SET(PCH_ADDITIONAL_COMPILER_FLAGS_lucene++-contrib -DLPP_HAVE_DLL)
ADD_LIBRARY(lucene++-contrib SHARED
	${lucene_sources} ${HEADERS}
)
#set properties on the libraries
SET_TARGET_PROPERTIES(lucene++-contrib PROPERTIES
    VERSION ${LUCENE++_VERSION}
    SOVERSION ${LUCENE++_SOVERSION}
    COMPILE_FLAGS -DLPP_HAVE_DLL
)
TARGET_LINK_LIBRARIES(lucene++-contrib 
  ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS lucene++-contrib
	        DESTINATION ${LIB_DESTINATION} 
	        COMPONENT runtime )

