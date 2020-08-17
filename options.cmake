# lucene++ project build options
#========================================================================


# linker args
#========================================================================

option(LUCENE_BUILD_SHARED
"Build shared library"
ON
)

option(ENABLE_PACKAGING
"Create build scripts for creating lucene++ packages"
OFF
)

option(LUCENE_USE_STATIC_BOOST_LIBS
"Use static boost libraries"
OFF
)

option(ENABLE_BOOST_INTEGER
"Enable boost integer types"
OFF
)

option(ENABLE_CYCLIC_CHECK
"Enable cyclic checking"
OFF
)



# build options
#========================================================================

option(
    ENABLE_TEST
    "Enable the tests"
    ON)

option(
    ENABLE_DEMO
    "Enable building demo applications"
    ON)

OPTION(
    ENABLE_DOCS
    "Build the Lucene++ documentation."
    OFF)


# documentation options
#========================================================================

OPTION(
    DOCS_HTML_HELP
    "Doxygen should compile HTML into a Help file (CHM)."
    NO )

OPTION(
    DOCS_HTML
    "Doxygen should build HTML documentation."
    YES )

OPTION(
    DOCS_XML
    "Doxygen should build XML documentation."
    NO )

OPTION(
    DOCS_RTF
    "Doxygen should build RTF documentation."
    NO )

OPTION(
    DOCS_MAN
    "Doxygen should build man documentation."
    NO )

OPTION(
    DOCS_TAGFILE
    "Doxygen should build a tagfile."
    NO )

OPTION(
    DOCS_LATEX
    "Doxygen should build Latex documentation."
    NO )