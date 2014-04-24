#Creates all the relevant packages

set(CPACK_PACKAGE_VERSION_MAJOR ${lucene++_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${lucene++_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${lucene++_VERSION_MAJOR})

set(CPACK_PACKAGE_VERSION ${lucene++_VERSION})
set(CPACK_PACKAGE_SOVERSION ${lucene++_SOVERSION})

set(CPACK_PACKAGE_VENDOR "Alan Wright")
set(CPACK_PACKAGE_CONTACT "alanwright.home@googlemail.com")
set(CPACK_PACKAGE_NAME "liblucene++")

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.PACKAGE")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lucene++ is an up to date C++ port of the popular Java Lucene library, a high-performance, full-featured text search engine")

set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.PACKAGE")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

#so, what are we going to install?
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${CMAKE_BINARY_DIR};lucene++;ALL;/")
set(CPACK_COMPONENTS_ALL development runtime)
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "lucene++-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

if((WIN32 OR WIN64) AND NOT UNIX)
	set(CPACK_SOURCE_GENERATOR "ZIP")
else()
	set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ")
endif()
set(CPACK_SOURCE_PACKAGE_FILE_NAME "lucene++-${CPACK_PACKAGE_VERSION}-Source")

#specific packaging requirements:,
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.4), libgcc1 (>= 1:4.1.1-21), libstdc++6 (>= 4.1.1-21), libboost-date-time1.42.0, libboost-filesystem1.42.0, libboost-regex1.42.0, libboost-thread1.42.0, libboost-iostreams1.42.0")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_RPM_PACKAGE_LICENSE "Apache 2.0")
set(CPACK_RPM_PACKAGE_GROUP "libs")
set(CPACK_RPM_PACKAGE_REQUIRES "libboost-date-time1.42.0, libboost-filesystem1.42.0, libboost-regex1.42.0, libboost-thread1.42.0, libboost-iostreams1.42.0")

#don't include the current binary dir.
get_filename_component(lucene++_BINARY_DIR_name "${lucene++_BINARY_DIR}" NAME)
set(CPACK_SOURCE_IGNORE_FILES
  "/\\\\.svn/"
  "/\\\\.git/"
  "\\\\.swp$"
  "\\\\.#;/#"
  ".*~"
  ".*\\\\.tmp"
  ".*\\\\.save"
  "/${lucene++_BINARY_DIR_name}/"
)

if((WIN32 OR WIN64) AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
  #set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  #set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Lucene++ Library")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\lucene++.sourceforge.net")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\lucene++.sourceforge.net")
  set(CPACK_NSIS_CONTACT "lucene++-developers@lists.sourceforge.net")
  #set(CPACK_NSIS_MODIFY_PATH ON)
else()
#  set(CPACK_STRIP_FILES "bin/xxx")
  set(CPACK_SOURCE_STRIP_FILES "")
endif()
#set(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")


add_custom_target(dist-package
    COMMAND rsync -avP -e ssh "${CPACK_PACKAGE_FILE_NAME}.*" ustramooner@frs.sourceforge.net:uploads/
#    DEPENDS package
)
add_custom_target(dist-package_source
    COMMAND rsync -avP -e ssh "${CPACK_SOURCE_PACKAGE_FILE_NAME}.*" ustramooner@frs.sourceforge.net:uploads/
#    DEPENDS package_source
)

#this must be last
include(CPack)
