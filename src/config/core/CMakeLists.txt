#################################
# install pkconfig
#################################
if(NOT WIN32)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/liblucene++.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++.pc" @ONLY)
  
  install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/liblucene++.pc"
    DESTINATION "include/pkgconfig")
endif()


#################################
# install config.cmake
#################################
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/liblucene++Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/liblucene++Config.cmake"
  INSTALL_DESTINATION "${LIB_DESTINATION}/cmake")

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/liblucene++ConfigVersion.cmake
  VERSION ${lucene++_VERSION}
  COMPATIBILITY SameMajorVersion )

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/liblucene++ConfigVersion.cmake"
  DESTINATION "include/cmake")
