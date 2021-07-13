#################################
# install pkconfig
#################################
if(NOT WIN32)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/liblucene++-contrib.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contrib.pc" @ONLY)
  
  install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contrib.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()


#################################
# install config.cmake
#################################
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/liblucene++-contribConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contribConfig.cmake"
  INSTALL_DESTINATION "${LIB_DESTINATION}/cmake/liblucene++-contrib"
  PATH_VARS contrib_libname PACKAGE_CMAKE_INSTALL_INCLUDEDIR PACKAGE_CMAKE_INSTALL_LIBDIR)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contribConfigVersion.cmake"
  VERSION ${lucene++_VERSION}
  COMPATIBILITY SameMajorVersion )

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contribConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++-contribConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/liblucene++-contrib")
