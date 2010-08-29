#############################################################################
## Copyright (c) 2009-2010 Alan Wright. All rights reserved.
## Distributable under the terms of either the Apache License (Version 2.0)
## or the GNU Lesser General Public License.
#############################################################################

import sys
import os
from copy import copy
import Options
from Configure import conf
from TaskGen import feature, after
import Task, ccroot

APPNAME='Lucene++'
VERSION='3.0.2'

srcdir = '.'
blddir = 'bin'

lucene_source_dirs = [
    'analysis',
    'analysis/standard',
    'analysis/tokenattributes',
    'document',
    'index',
    'queryparser',
    'search',
    'search/function',
    'search/payloads',
    'search/spans',
    'store',
    'util',
    'util/md5',
    'util/utf8',
    'util/zlib'
]

nedmalloc_source_file = 'util/nedmalloc/nedmalloc.c'

lucene_include_dirs = [
    'include',
    'util/md5',
    'util/nedmalloc',
    'util/utf8',
    'util/zlib'
]

tester_source_dirs = [
    'test',
    'test/analysis',
    'test/analysis/standard',
    'test/analysis/tokenattributes',
    'test/document',
    'test/index',
    'test/queryparser',
    'test/search',
    'test/search/function',
    'test/search/payloads',
    'test/search/spans',
    'test/store',
    'test/util'
]

tester_include_dirs = [
    'include',
    'test/include'
]


def set_options(opt):
    opt.tool_options("boost")
    opt.tool_options('compiler_cxx')
    opt.tool_options('clang', tooldir = 'build')
    opt.add_option(
        '--debug', 
        default = False,
        action = "store_true",
        help ='debug build no optimization, etc...', 
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
    conf.check_cc(lib = 'pthread', mandatory = True)
    conf.check_tool('boost')
    conf.check_tool('clang', 'build')
    conf.check_boost(
        static = 'onlystatic',
        lib = ['filesystem', 'thread', 'regex', 'system', 'date_time', 'iostreams', 'unit_test_framework']
    )

            
def build(bld):
    target_type = 'cstaticlib' if Options.options.static else 'cshlib'
    link_flags = ['-m32']
    if Options.options.debug:
         compile_flags = ['-O0', '-g', '-m32'] 
    else:
         compile_flags = ['-O2', '-m32']
    debug_define = '_DEBUG' if Options.options.debug else 'NDEBUG' 
    lucene_sources = []
    for source_dir in lucene_source_dirs:
        for source_file in bld.path.ant_glob(source_dir + '/*.c*').split():
            source_path = bld.path.find_resource(source_file)
            lucene_sources.append(source_path)
    lucene_sources.append(bld.path.find_resource(nedmalloc_source_file))     
    
    bld(
        name = 'lucene++',
        features = ['cxx', 'cc'] + [target_type],
        source = [source.relpath_gen(bld.path) for source in lucene_sources],
        target = 'lucene++',
        includes = lucene_include_dirs + [bld.env["CPPPATH_BOOST"]],
        ccflags = compile_flags,
        cxxflags = compile_flags,
        linkflags = link_flags,
        defines = ['LPP_BUILDING_LIB', 'LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAM PTHREAD',
        
        )
    
    tester_sources = []
    for source_dir in tester_source_dirs:
        for source_file in bld.path.ant_glob(source_dir + '/*.c*').split():
            source_path = bld.path.find_resource(source_file)
            tester_sources.append(source_path)
    
    bld(
        name = 'lucene_tester',
        features = ['cxx', 'cc', 'cprogram'],
        source = [source.relpath_gen(bld.path) for source in tester_sources],
        target = 'lucene_tester',
        includes = tester_include_dirs + [bld.env["CPPPATH_BOOST"]],
        ccflags = compile_flags,
        cxxflags = compile_flags,
        linkflags = link_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS BOOST_UNIT_TEST_FRAMEWORK PTHREAD',
        uselib_local = 'lucene++',
        )
    
    bld(
        name = 'deletefiles',
        features = ['cxx', 'cc', 'cprogram'],
        source = bld.path.find_resource('demo/deletefiles/main.cpp').relpath_gen(bld.path),
        target = 'deletefiles',
        includes = ['include'] + [bld.env["CPPPATH_BOOST"]],
        ccflags = compile_flags,
        cxxflags = compile_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD',
        uselib_local = 'lucene++',
        )

    bld(
        name = 'indexfiles',
        features = ['cxx', 'cc', 'cprogram'],
        source = bld.path.find_resource('demo/indexfiles/main.cpp').relpath_gen(bld.path),
        target = 'indexfiles',
        includes = ['include'] + [bld.env["CPPPATH_BOOST"]],
        ccflags = compile_flags,
        cxxflags = compile_flags,
        linkflags = link_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD',
        uselib_local = 'lucene++',
        )

    bld(
        name = 'searchfiles',
        features = ['cxx', 'cc', 'cprogram'],
        source = bld.path.find_resource('demo/searchfiles/main.cpp').relpath_gen(bld.path),
        target = 'searchfiles',
        includes = ['include'] + [bld.env["CPPPATH_BOOST"]],
        ccflags = compile_flags,
        cxxflags = compile_flags,
        linkflags = link_flags,
        defines = ['LPP_HAVE_GXXCLASSVISIBILITY'] + [debug_define],
        uselib = 'BOOST_FILESYSTEM BOOST_THREAD BOOST_REGEX BOOST_SYSTEM BOOST_DATE_TIME BOOST_IOSTREAMS PTHREAD',
        uselib_local = 'lucene++'
        )

