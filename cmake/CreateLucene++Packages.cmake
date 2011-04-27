#Creates all the relevant packages

SET(CPACK_PACKAGE_VERSION_MAJOR ${LUCENE++_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${LUCENE++_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_REVISION ${LUCENE++_VERSION_REVISION})
SET(CPACK_PACKAGE_VERSION_PATCH ${LUCENE++_VERSION_MAJOR})

SET(CPACK_PACKAGE_VERSION ${LUCENE++_VERSION})
SET(CPACK_PACKAGE_SOVERSION ${LUCENE++_SOVERSION})

SET(CPACK_PACKAGE_VENDOR "Ben van Klinken")
SET(CPACK_PACKAGE_CONTACT "ustramooner@users.sourceforge.net")
SET(CPACK_PACKAGE_NAME "liblucene++")

SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.PACKAGE")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lucene++ is an up to date C++ port of the popular Java Lucene library, a high-performance, full-featured text search engine")

SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.PACKAGE")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/README.PACKAGE")

#so, what are we going to install?
SET(CPACK_INSTALL_CMAKE_PROJECTS
  "${CMAKE_BINARY_DIR};lucene++;ALL;/")
SET(CPACK_COMPONENTS_ALL development runtime)
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_PACKAGE_FILE_NAME "lucene++-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

IF( (WIN32 OR WIN64) AND NOT UNIX)
	SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSE( (WIN32 OR WIN64) AND NOT UNIX)
	SET(CPACK_SOURCE_GENERATOR "TBZ2;TGZ")
ENDIF( (WIN32 OR WIN64) AND NOT UNIX)
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "lucene++-${CPACK_PACKAGE_VERSION}-Source")

#specific packaging requirements:,
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.4), libgcc1 (>= 1:4.1.1-21), libstdc++6 (>= 4.1.1-21), libboost-date-time1.42.0, libboost-filesystem1.42.0, libboost-regex1.42.0, libboost-thread1.42.0, libboost-iostreams1.42.0")
SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
SET(CPACK_RPM_PACKAGE_LICENSE "Apache 2.0")
SET(CPACK_RPM_PACKAGE_GROUP "libs")
SET(CPACK_RPM_PACKAGE_REQUIRES "libboost-date-time1.42.0, libboost-filesystem1.42.0, libboost-regex1.42.0, libboost-thread1.42.0, libboost-iostreams1.42.0")

#don't include the current binary dir.
get_filename_component(lucene++-base_BINARY_DIR_name ${lucene++-base_BINARY_DIR} NAME)
SET(CPACK_SOURCE_IGNORE_FILES
  "/\\\\.svn/"
  "/\\\\.git/"
  "/\\\\.waf*/"
  "\\\\.swp$"
  "\\\\.#;/#"
  ".*~"
  ".*\\\\.tmp"
  ".*\\\\.save"
  "/${lucene++-base_BINARY_DIR_name}/"
)

IF( (WIN32 OR WIN64) AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
  #SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  #SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Lucene++ Library")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\lucene++.sourceforge.net")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\lucene++.sourceforge.net")
  SET(CPACK_NSIS_CONTACT "lucene++-developers@lists.sourceforge.net")
  #SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE( (WIN32 OR WIN64) AND NOT UNIX)
#  SET(CPACK_STRIP_FILES "bin/xxx")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF( (WIN32 OR WIN64) AND NOT UNIX)
#SET(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")


ADD_CUSTOM_TARGET(dist-package
    COMMAND rsync -avP -e ssh ${CPACK_PACKAGE_FILE_NAME}.* ustramooner@frs.sourceforge.net:uploads/
#    DEPENDS package
)
ADD_CUSTOM_TARGET(dist-package_source
    COMMAND rsync -avP -e ssh ${CPACK_SOURCE_PACKAGE_FILE_NAME}.* ustramooner@frs.sourceforge.net:uploads/
#    DEPENDS package_source
)

#this must be last
INCLUDE(CPack)
