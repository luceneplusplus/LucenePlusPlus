#############################################################################
## Copyright (c) 2009-2011 Alan Wright. All rights reserved.
## Distributable under the terms of either the Apache License (Version 2.0)
## or the GNU Lesser General Public License.
#############################################################################

import sys
import os
from copy import copy
import Options
from Configure import conf
from TaskGen import feature, after
import Task

APPNAME='Lucene++'
VERSION='3.0.3.4'

top = '.'
out = 'bin'

source_patterns = [
    '**/*.c',
    '**/*.cpp'
]

lucene_source_dirs = [
    'src/core/analysis',
    'src/core/document',
    'src/core/index',
    'src/core/queryparser',
    'src/core/search',
    'src/core/store',
    'src/core/util'
]

lucene_contrib_source_dirs = [
    'src/contrib/analyzers',
    'src/contrib/highlighter',
    'src/contrib/memory',
    'src/contrib/snowball'
]

lucene_include_dirs = [
    'include',
    'src/core/include',
    'src/contrib/include',
    'src/contrib/snowball/libstemmer_c/include',
    'src/core/util/md5',
    'src/core/util/nedmalloc',
    'src/core/util/unicode'
]

tester_source_dirs = [
    'src/test/analysis',
    'src/test/contrib',
    'src/test/document',
    'src/test/index',
    'src/test/queryparser',
    'src/test/search',
    'src/test/store',
    'src/test/util',
    'src/test/main'
]

tester_include_dirs = [
    'include',
    'src/core/include',
    'src/contrib/include',
    'src/test/include'
]


def options(opt):
    opt.tool_options("boost")
    opt.tool_options('compiler_cxx')
    opt.tool_options('clang', tooldir = 'build')
    opt.tool_options('gch', tooldir = 'build')
    opt.add_option(
        '--debug', 
        default = False,
        action = "store_true",
        help ='debug build no optimization, etc.', 
        dest = 'debug')

    opt.add_option(
        '--static', 
        default = False,
        action = "store_true",
        help ='fully static build', 
        dest = 'static')


def configure(conf):
    conf.check_tool('g++')
    conf.check_tool('gcc')
    conf.check_cc(lib = 'z', mandatory = True)
    conf.check_cc(lib = 'pthread', mandatory = True)
    conf.check_tool('boost')
    conf.check_tool('clang', 'build')
    conf.check_tool('gch', 'build')
    conf.check_boost(
        static = 'onlystatic',
        lib = ['filesystem', 'thread', 'regex', 'system', 'date_time', 'iostreams', 'unit_test_framework']
    )

            
def build(bld):
    target_type = 'cstlib' if Options.options.static else 'cshlib'
    debug_define = '_DEBUG' if Options.options.debug else 'NDEBUG'
    if Options.options.debug:
         compile_flags = ['-O0', '-g'] 
    else:
         compile_flags = ['-O2']
    lucene_sources = []
    for source_dir in lucene_source_dirs:
        source_dir = bld.path.find_dir(source_dir)
        lucene_sources.extend(source_dir.ant_glob(source_patterns))
    
    bld(
        name = 'lucene++',
        features = ['cxx', 'c'] + [target_type],
        source = [source.relpath_gen(bld.path) for source in lucene_sources],
        target = 'lucene++',
        pch = 'src/core/include/LuceneInc.h',
        includes = lucene_include_dirs + bld.env["CPPPATH_BOOST"],
        cflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_BUILDING_LIB', 'LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD Z'
        )
    
    lucene_contrib_sources = []
    for source_dir in lucene_contrib_source_dirs:
        source_dir = bld.path.find_dir(source_dir)
        lucene_contrib_sources.extend(source_dir.ant_glob(source_patterns))
    
    bld(
        name = 'lucene_contrib',
        features = ['cxx', 'c'] + [target_type],
        source = [source.relpath_gen(bld.path) for source in lucene_contrib_sources],
        target = 'lucene_contrib',
        pch = 'src/contrib/include/ContribInc.h',
        includes = lucene_include_dirs + bld.env["CPPPATH_BOOST"],
        cflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_BUILDING_LIB', 'LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD Z',
        use = 'lucene++'
        )
    
    tester_sources = []
    for source_dir in tester_source_dirs:
        source_dir = bld.path.find_dir(source_dir)
        tester_sources.extend(source_dir.ant_glob(source_patterns))
    
    bld(
        name = 'lucene_tester',
        features = ['cxx', 'c', 'cprogram'],
        source = [source.relpath_gen(bld.path) for source in tester_sources],
        target = 'lucene_tester',
        pch = 'src/test/include/TestInc.h',
        includes = tester_include_dirs + bld.env["CPPPATH_BOOST"],
        cflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + ['LPP_EXPOSE_INTERNAL'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS BOOST_UNIT_TEST_FRAMEWORK PTHREAD Z',
        use = 'lucene++ lucene_contrib'
        )
    
    bld(
        name = 'deletefiles',
        features = ['cxx', 'c', 'cprogram'],
        source = bld.path.find_resource('src/demo/deletefiles/main.cpp').relpath_gen(bld.path),
        target = 'deletefiles',
        includes = ['include'] + bld.env["CPPPATH_BOOST"],
        cflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD Z',
        use = 'lucene++'
        )

    bld(
        name = 'indexfiles',
        features = ['cxx', 'c', 'cprogram'],
        source = bld.path.find_resource('src/demo/indexfiles/main.cpp').relpath_gen(bld.path),
        target = 'indexfiles',
        includes = ['include'] + bld.env["CPPPATH_BOOST"],
        cflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD Z',
        use = 'lucene++'
        )

    bld(
        name = 'searchfiles',
        features = ['cxx', 'c', 'cprogram'],
        source = bld.path.find_resource('src/demo/searchfiles/main.cpp').relpath_gen(bld.path),
        target = 'searchfiles',
        includes = ['include'] + bld.env["CPPPATH_BOOST"],
        cflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD Z',
        use = 'lucene++'
        )

