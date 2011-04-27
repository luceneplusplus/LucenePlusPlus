#############################################################################
## Copyright (c) 2009-2011 Alan Wright. All rights reserved.
## Distributable under the terms of either the Apache License (Version 2.0)
## or the GNU Lesser General Public License.
#############################################################################

from TaskGen import feature
import Options
import sys


@feature('cc')
def apply_clang(self):
    '''
    Replaced the default compiler with clang if required.
    '''
    if not getattr(self, 'clang', False) or Options.options.disable_clang:
        return
    self.env['CC'] = self.env['CLANG'] or self.env['CC']
    if sys.platform == "darwin":
        # workaround problems with non-static inline functions
        # http://clang.llvm.org/compatibility.html
        self.env['CCFLAGS'] += ['-std=gnu89']


@feature('cc')
def apply_clang_cpp(self):
    '''
    Replaced the default compiler with clang if required.
    '''
    if not getattr(self, 'clang', False) or Options.options.disable_clang:
        return
    self.env['CPP'] = self.env['CLANGPP'] or self.env['CXX']
    self.env['CXX'] = self.env['CLANGPP'] or self.env['CXX']
    if sys.platform == "darwin":
        self.env['shlib_CXXFLAGS'] = ['-fPIC']


def options(opt):
    """
    Add options specific the codehash tool
    """
    opt.add_option('--noclang', 
        dest = 'disable_clang', 
        action = 'store_true',
        default = False,
        help = 'disable the clang compiler if it is available')


def configure(conf):
    search_paths = ['/Xcode4/usr/bin/'] if sys.platform == "darwin" else []
    if not getattr(conf, 'clang', False) or Options.options.disable_clang:
        return
    conf.find_program('clang', var='CLANG')
    conf.find_program('clang++', var='CLANGPP', path_list = search_paths)
