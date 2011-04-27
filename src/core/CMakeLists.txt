PROJECT (lucene++-lib)

####################################
# THE lucene++ library
####################################
file(GLOB_RECURSE lucene_sources
  ${lucene++-lib_SOURCE_DIR}/search/*.cpp
  ${lucene++-lib_SOURCE_DIR}/analysis/*.cpp
  ${lucene++-lib_SOURCE_DIR}/document/*.cpp
  ${lucene++-lib_SOURCE_DIR}/index/*.cpp
  ${lucene++-lib_SOURCE_DIR}/queryparser/*.cpp
  ${lucene++-lib_SOURCE_DIR}/store/*.cpp
  ${lucene++-lib_SOURCE_DIR}/util/*.cpp)

file(GLOB_RECURSE INTERN_HEADERS ${lucene++-libs_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE HEADERS ${lucene++-base_SOURCE_DIR}/include/*.h)

#C sources...
file(GLOB_RECURSE lucene_c_sources
  ${lucene++-lib_SOURCE_DIR}/util/*.c)

  
IF ( ENABLE_NEDMALLOC )
  ADD_DEFINITIONS(-DLPP_USE_NEDMALLOC)
ENDIF()

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${lucene++-base_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${lucene++-lib_SOURCE_DIR}/include)
ADD_DEFINITIONS(-DLPP_BUILDING_LIB)
ADD_LIBRARY(lucene++-c STATIC
	${lucene_c_sources}
)
install(FILES ${HEADERS}
		        DESTINATION include/lucene++
		        COMPONENT development)

#################################
# lucene++ shared library
#################################
ADD_LIBRARY(lucene++ SHARED
	${lucene_sources} ${HEADERS} ${INTERN_HEADERS}
)
SET(PCH_ADDITIONAL_COMPILER_FLAGS_lucene++ -DLPP_HAVE_DLL)
ADD_PRECOMPILED_HEADER(lucene++ ${lucene++-lib_SOURCE_DIR}/include/LuceneInc.h)
#set properties on the libraries
SET_TARGET_PROPERTIES(lucene++ PROPERTIES
    VERSION ${LUCENE++_VERSION}
    SOVERSION ${LUCENE++_SOVERSION}
)
TARGET_LINK_LIBRARIES(lucene++ 
  lucene++-c 
  ${CMAKE_THREAD_LIBS_INIT}
  ${LUCENE_BOOST_LIBS} )
install(TARGETS lucene++
	        DESTINATION ${LIB_DESTINATION} 
	        COMPONENT runtime)

#################################
# lucene++ static library
#################################
ADD_LIBRARY(lucene++-static STATIC EXCLUDE_FROM_ALL
	${lucene_sources} ${HEADERS} ${INTERN_HEADERS}
)
SET(PCH_ADDITIONAL_COMPILER_FLAGS_lucene++-static -DLPP_HAVE_DLL)
ADD_PRECOMPILED_HEADER(lucene++-static ${lucene++-lib_SOURCE_DIR}/include/LuceneInc.h)
#set properties on the libraries
SET_TARGET_PROPERTIES(lucene++-static PROPERTIES
    VERSION ${LUCENE++_VERSION}
    SOVERSION ${LUCENE++_SOVERSION}
)

