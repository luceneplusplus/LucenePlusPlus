# - Lucene++Docs.cmake
# This file provides support for building the Lucene++ Documentation.
# To build the documention, you will have to enable it
# and then do the equivalent of "make doc".

MACRO(SET_YESNO)
    FOREACH(param ${ARGV})
    	IF ( ${param} )
    	    SET(${param} "YES")
        ELSE ( ${param} )
            SET(${param} "NO")
    	ENDIF ( ${param} )
    ENDFOREACH(param)
ENDMACRO(SET_YESNO)
MACRO(SET_BLANK)
    FOREACH(param ${ARGV})
    	IF ( NOT ${param} )
    	    SET(${param} "")
    	ENDIF ( NOT ${param} )
    ENDFOREACH(param)
ENDMACRO(SET_BLANK)

IF (ENABLE_DOCS)
    OPTION(DOCS_HTML_HELP
        "Doxygen should compile HTML into a Help file (CHM)." NO)

    OPTION(DOCS_HTML
        "Doxygen should build HTML documentation." YES)
    OPTION(DOCS_XML
        "Doxygen should build XML documentation." NO)
    OPTION(DOCS_RTF
        "Doxygen should build RTF documentation." NO)
    OPTION(DOCS_MAN
        "Doxygen should build man documentation." NO)
    OPTION(DOCS_TAGFILE
        "Doxygen should build a tagfile." NO)

    OPTION(DOCS_LATEX
        "Doxygen should build Latex documentation." NO )

    MARK_AS_ADVANCED(
        DOCS_HTML_HELP
        DOCS_LATEX
        DOCS_XML
        DOCS_HTML
        DOCS_RTF
        DOCS_MAN
        DOCS_TAGFILE
    )

    #
    # Check for the tools
    #
    FIND_PACKAGE(Doxygen)

    IF ( DOXYGEN_FOUND )
        # This creates a new target to build documentation.
        # It runs ${DOXYGEN_EXECUTABLE} which is the full path and executable to
        # Doxygen on your system, set by the FindDoxygen.cmake module
        # (called by FindDocumentation.cmake).
        # It runs the final generated Doxyfile against it.
        # The DOT_PATH is substituted into the Doxyfile.
        ADD_CUSTOM_TARGET(doc
            "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/doc/doxyfile"
            VERBATIM
        )

        IF ( DOCS_HTML_HELP )
            IF ( NOT DOCS_HTML )
                MESSAGE ( FATAL_ERROR "DOCS_HTML is required to buidl DOCS_HTML_HELP" )
            ENDIF ( NOT DOCS_HTML )
            FIND_PACKAGE(HTMLHelp)
            IF ( NOT HTML_HELP_COMPILER )
                MESSAGE(FATAL_ERROR "HTML Help compiler not found, turn DOCS_HTML_HELP off to proceed")
            ENDIF ( NOT HTML_HELP_COMPILER )

            #make cygwin work with hhc...
            IF ( CYGWIN )
                EXECUTE_PROCESS ( COMMAND cygpath "${HTML_HELP_COMPILER}"
                    OUTPUT_VARIABLE HTML_HELP_COMPILER_EX )
                STRING ( REPLACE "\n" "" HTML_HELP_COMPILER_EX "${HTML_HELP_COMPILER_EX}" )
                STRING ( REPLACE "\r" "" HTML_HELP_COMPILER_EX "${HTML_HELP_COMPILER_EX}" )
                SET ( HTML_HELP_COMPILER_EX "\"${HTML_HELP_COMPILER_EX}\"" )
            ELSE ( CYGWIN )
                SET ( HTML_HELP_COMPILER_EX "${HTML_HELP_COMPILER}" )
            ENDIF ( CYGWIN )
        ENDIF ( DOCS_HTML_HELP )

        IF ( DOCS_LATEX )
            FIND_PACKAGE(LATEX)
            IF ( NOT LATEX_COMPILER )
                MESSAGE(FATAL_ERROR "Latex compiler not found, turn DOCS_LATEX off to proceed")
            ENDIF ( NOT LATEX_COMPILER )
        ENDIF ( DOCS_LATEX )

        FIND_PACKAGE(Perl)

        IF ( DOXYGEN_DOT_EXECUTABLE )
            SET ( HAVE_DOT "YES" )
        ELSE ( DOXYGEN_DOT_EXECUTABLE )
            SET ( HAVE_DOT "NO" )
        ENDIF ( DOXYGEN_DOT_EXECUTABLE )

        #doxygen expects YES/NO parameters
        SET_YESNO(
            DOCS_HTML_HELP
            DOCS_LATEX
            DOCS_XML
            DOCS_HTML
            DOCS_RTF
            DOCS_MAN
        )
        #empty out paths if not found
        SET_BLANK(
            PERL_EXECUTABLE
            DOXYGEN_DOT_EXECUTABLE
            HTML_HELP_COMPILER
            LATEX_COMPILER
        )

        IF ( DOCS_TAGFILE )
            SET ( DOCS_TAGFILE_LOCATION "${PROJECT_BINARY_DIR}/doc/tag/lucene++.tag"  )
        ENDIF ( DOCS_TAGFILE )

        # This processes our Doxyfile.cmake and substitutes paths to generate a final Doxyfile
        CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/doc/doxygen/Doxyfile.cmake" "${PROJECT_BINARY_DIR}/doc/doxyfile")
        CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/doc/doxygen/helpheader.htm.cmake" "${PROJECT_BINARY_DIR}/doc/helpheader.htm")
        CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/doc/doxygen/helpfooter.htm.cmake" "${PROJECT_BINARY_DIR}/doc/helpfooter.htm")
        CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/doc/doxygen/doxygen.css.cmake" "${PROJECT_BINARY_DIR}/doc/html/doxygen.css")

        #create a target for tar.gz html help
        FIND_PACKAGE(UnixCommands)
        IF ( TAR AND GZIP )
            ADD_CUSTOM_TARGET(doc-tarz
                COMMAND "${TAR}" "-czf" "${PROJECT_BINARY_DIR}/doc/lucene++-doc.tar.gz" ./
                WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/doc/html/"
                #DEPENDS doc
                VERBATIM
            )
        ENDIF ( TAR AND GZIP )

	#install HTML pages if they were built
	IF ( DOCS_HTML AND NOT WIN32 )
            INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/doc/html/" DESTINATION share/doc/lucene++-doc/html)
        ENDIF ( DOCS_HTML AND NOT WIN32 )

        #install man pages if they were built
        IF ( DOCS_MAN )
            INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/doc/man/" DESTINATION man)
        ENDIF ( DOCS_MAN )

    ELSE ( DOXYGEN_FOUND )
        MESSAGE(FATAL_ERROR "Doxygen not found, turn ENABLE_DOCS off to proceed")
    ENDIF ( DOXYGEN_FOUND )


ENDIF (ENABLE_DOCS)
