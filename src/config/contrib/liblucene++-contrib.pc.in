prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}/bin
libdir=@LIB_DESTINATION@
includedir=${prefix}/include/lucene++
lib=lucene++-contrib

Name: liblucene++-contrib
Description: Contributions for Lucene++ - a C++ search engine, ported from the popular Apache Lucene
Version: @lucene++_VERSION@
Libs: -L@LIB_DESTINATION@ -l${lib}
Cflags: -I${includedir}
Requires: liblucene++ = @lucene++_VERSION@

