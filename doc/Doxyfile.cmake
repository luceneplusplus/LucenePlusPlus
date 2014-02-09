# Doxyfile 1.2.18

#---------------------------------------------------------------------------
# General configuration options
#---------------------------------------------------------------------------

PROJECT_NAME           = Lucene++
PROJECT_NUMBER         = @lucene++_SOVERSION@

OUTPUT_DIRECTORY       = @PROJECT_BINARY_DIR@/doc
OUTPUT_LANGUAGE        = English

EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = YES
EXTRACT_LOCAL_CLASSES  = YES
EXTRACT_LOCAL_METHODS  = YES
HIDE_UNDOC_MEMBERS     = NO
HIDE_UNDOC_CLASSES     = NO
HIDE_FRIEND_COMPOUNDS  = YES
HIDE_IN_BODY_DOCS      = NO
BRIEF_MEMBER_DESC      = YES
REPEAT_BRIEF           = YES
ALWAYS_DETAILED_SEC    = NO
INLINE_INHERITED_MEMB  = NO
FULL_PATH_NAMES        = NO
STRIP_FROM_PATH        =
INTERNAL_DOCS          = NO
STRIP_CODE_COMMENTS    = YES
CASE_SENSE_NAMES       = YES
SHORT_NAMES            = NO
HIDE_SCOPE_NAMES       = NO
VERBATIM_HEADERS       = YES
SHOW_INCLUDE_FILES     = YES
JAVADOC_AUTOBRIEF      = YES
MULTILINE_CPP_IS_BRIEF = YES
DETAILS_AT_TOP         = NO
INHERIT_DOCS           = YES
INLINE_INFO            = YES
SORT_MEMBER_DOCS       = YES
DISTRIBUTE_GROUP_DOC   = NO
TAB_SIZE               = 8
GENERATE_TODOLIST      = YES
GENERATE_TESTLIST      = YES
GENERATE_BUGLIST       = YES
GENERATE_DEPRECATEDLIST= YES
ALIASES                = "memory=\par Memory management:\n"
ENABLED_SECTIONS       =
MAX_INITIALIZER_LINES  = 30
OPTIMIZE_OUTPUT_FOR_C  = YES
OPTIMIZE_OUTPUT_JAVA   = NO
SHOW_USED_FILES        = YES
DIRECTORY_GRAPH        = YES
DOCSET_BUNDLE_ID       = org.doxygen.Project
DOCSET_FEEDNAME        = "Doxygen generated docs"
DOXYFILE_ENCODING      = UTF-8
FORMULA_FONTSIZE       = 10
TYPEDEF_HIDES_STRUCT   = YES
ABBREVIATE_BRIEF       = "The $name class" \
                         "The $name widget" \
                         "The $name file" \
                         is \
                         provides \
                         specifies \
                         contains \
                         represents \
                         a \
                         an \
                         the
#---------------------------------------------------------------------------
# configuration options related to warning and progress messages
#---------------------------------------------------------------------------
QUIET                  = NO
WARNINGS               = YES
WARN_IF_UNDOCUMENTED   = YES
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = NO
WARN_FORMAT            = "$file:$line: $text"
WARN_LOGFILE           = @PROJECT_BINARY_DIR@/doc/doxygen.warnings.log

#---------------------------------------------------------------------------
# configuration options related to the input files
#---------------------------------------------------------------------------

INPUT                  = @PROJECT_SOURCE_DIR@/include
FILE_PATTERNS          = *.h
RECURSIVE              = YES
EXCLUDE_SYMLINKS       = NO
EXCLUDE_PATTERNS       = "**/.svn/**" \
                         "**/.git/**" \
                         "**/Lucene.h"
                         "*/test/*" \
                         "*/md5/*" \
                         "*/nedmalloc/*" \
                         "*/utf8/*"
EXAMPLE_PATH           =
EXAMPLE_PATTERNS       =
EXAMPLE_RECURSIVE      = NO
IMAGE_PATH             =
INPUT_FILTER           =
FILTER_SOURCE_FILES    = NO

#---------------------------------------------------------------------------
# configuration options related to source browsing
#---------------------------------------------------------------------------

SOURCE_BROWSER         = NO
INLINE_SOURCES         = NO
REFERENCED_BY_RELATION = YES
REFERENCES_RELATION    = YES

#---------------------------------------------------------------------------
# configuration options related to the alphabetical class index
#---------------------------------------------------------------------------

ALPHABETICAL_INDEX     = NO
COLS_IN_ALPHA_INDEX    = 5
IGNORE_PREFIX          =

#---------------------------------------------------------------------------
# configuration options related to the HTML output
#---------------------------------------------------------------------------

GENERATE_HTML          = @DOCS_HTML@
HTML_OUTPUT            = html
HTML_FILE_EXTENSION    = .html
HTML_HEADER            = @PROJECT_BINARY_DIR@/doc/helpheader.htm
HTML_FOOTER            = @PROJECT_BINARY_DIR@/doc/helpfooter.htm
HTML_STYLESHEET        =
HTML_ALIGN_MEMBERS     = YES
HTML_DYNAMIC_SECTIONS  = YES

GENERATE_HTMLHELP      = @DOCS_HTML_HELP@
CHM_FILE               = ../lucene++.chm
HHC_LOCATION           = @HTML_HELP_COMPILER_EX@
GENERATE_CHI           = YES
BINARY_TOC             = YES
TOC_EXPAND             = NO
DISABLE_INDEX          = NO
ENUM_VALUES_PER_LINE   = 4
GENERATE_TREEVIEW      = NO
TREEVIEW_WIDTH         = 250

#---------------------------------------------------------------------------
# configuration options related to the LaTeX output
#---------------------------------------------------------------------------

GENERATE_LATEX         = @DOCS_LATEX@
LATEX_OUTPUT           = latex
LATEX_CMD_NAME         = @LATEX_COMPILER@
MAKEINDEX_CMD_NAME     = makeindex
COMPACT_LATEX          = NO
PAPER_TYPE             = a4wide
EXTRA_PACKAGES         =
LATEX_HEADER           =
PDF_HYPERLINKS         = YES
USE_PDFLATEX           = NO
LATEX_BATCHMODE        = NO

#---------------------------------------------------------------------------
# configuration options related to the RTF output
#---------------------------------------------------------------------------

GENERATE_RTF           = @DOCS_RTF@
RTF_OUTPUT             = rtf
COMPACT_RTF            = NO
RTF_HYPERLINKS         = NO
RTF_STYLESHEET_FILE    =
RTF_EXTENSIONS_FILE    =

#---------------------------------------------------------------------------
# configuration options related to the man page output
#---------------------------------------------------------------------------
GENERATE_MAN           = @DOCS_MAN@
MAN_OUTPUT             = man
MAN_EXTENSION          = .3
MAN_LINKS              = NO

#---------------------------------------------------------------------------
# configuration options related to the XML output
#---------------------------------------------------------------------------

GENERATE_XML           = @DOCS_XML@
XML_SCHEMA             =
XML_DTD                =
XML_OUTPUT             = xml
XML_PROGRAMLISTING     = YES

#---------------------------------------------------------------------------
# configuration options for the AutoGen Definitions output
#---------------------------------------------------------------------------

GENERATE_AUTOGEN_DEF   = NO

#---------------------------------------------------------------------------
# Configuration options related to the preprocessor
#---------------------------------------------------------------------------

ENABLE_PREPROCESSING   = YES
MACRO_EXPANSION        = YES
EXPAND_ONLY_PREDEF     = NO
SEARCH_INCLUDES        = YES
INCLUDE_PATH           =
INCLUDE_FILE_PATTERNS  =

PREDEFINED             = ""

EXPAND_AS_DEFINED      =
SKIP_FUNCTION_MACROS   = YES

#---------------------------------------------------------------------------
# Configuration::addtions related to external references
#---------------------------------------------------------------------------

TAGFILES               =
GENERATE_TAGFILE       = @DOCS_TAGFILE_LOCATION@
ALLEXTERNALS           = NO
EXTERNAL_GROUPS        = YES
PERL_PATH              = @PERL_EXECUTABLE@

#---------------------------------------------------------------------------
# Configuration options related to the dot tool
#---------------------------------------------------------------------------

CLASS_DIAGRAMS         = YES
HIDE_UNDOC_RELATIONS   = YES
HAVE_DOT               = @HAVE_DOT@
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
TEMPLATE_RELATIONS     = NO
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
GRAPHICAL_HIERARCHY    = YES
DOT_IMAGE_FORMAT       = png
DOT_PATH               = @DOXYGEN_DOT_EXECUTABLE@
DOTFILE_DIRS           =
GENERATE_LEGEND        = YES
DOT_CLEANUP            = YES
DOT_FONTNAME           = FreeSans
DOT_FONTPATH           =
DOT_FONTSIZE           = 10
DOT_GRAPH_MAX_NODES    = 50
DOT_MULTI_TARGETS      = NO
DOT_TRANSPARENT        = NO

#---------------------------------------------------------------------------
# Configuration::addtions related to the search engine
#---------------------------------------------------------------------------

SEARCHENGINE           = YES
