####################################
# Set config vars
####################################
set(core_libname, "lucene++")
set(contrib_libname, "lucene++-contrib")

set(
    PACKAGE_CMAKE_INSTALL_INCLUDEDIR,
    "${lucene++_INCLUDE_DIR}/lucene++/")

set(
    PACKAGE_CMAKE_INSTALL_LIBDIR,
    "${LIB_INSTALL_DIR}/cmake")


####################################
# include CMakePackageConfigHelpers
####################################
include(CMakePackageConfigHelpers)


####################################
# include directories
####################################
add_subdirectory(core)
add_subdirectory(contrib)