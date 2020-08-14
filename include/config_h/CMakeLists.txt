#################################
# set Config.h vars
#################################
if(ENABLE_BOOST_INTEGER)
  set(USE_BOOST_INTEGER "define")
else()
  set(USE_BOOST_INTEGER "undef")
endif()

if(ENABLE_CYCLIC_CHECK)
  set(USE_CYCLIC_CHECK "define")
else()
  set(USE_CYCLIC_CHECK "undef")
endif()


if(LUCENE_BUILD_SHARED)
  set(LPP_SHARED_DLL "define")
else()
  set(LPP_SHARED_DLL "undef")
endif()


#################################
# generate Config.h
#################################
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
  "${lucene++_BINARY_DIR}/include/Config.h" @ONLY
)


#################################
# install Config.h
#################################
install(
  FILES 
    "${lucene++_BINARY_DIR}/include/Config.h"
  DESTINATION include/lucene++ )
