#############################################################################
## Copyright (c) 2009-2011 Alan Wright. All rights reserved.
## Distributable under the terms of either the Apache License (Version 2.0)
## or the GNU Lesser General Public License.
#############################################################################

from TaskGen import feature
import Options
import sys


@feature('c')
def apply_clang(self):
    if self.env['HAVE_LLVM'] == False:
      return
    '''
    Replaced the default compiler with clang if required.
    '''
    if not getattr(self, 'clang', True) or Options.options.disable_clang:
        return
    self.env['CC'] = self.env['CLANG'] or self.env['CC']
    if sys.platform == "darwin":
        # workaround problems with non-static inline functions
        # http://clang.llvm.org/compatibility.html
        self.env['CCFLAGS'] += ['-std=gnu89']

@feature('c')
def apply_clang_cpp(self):
    if self.env['HAVE_LLVM'] == False:
      return
    '''
    Replaced the default compiler with clang if required.
    '''
    if not getattr(self, 'clang', True) or Options.options.disable_clang:
        return
    self.env['CPP'] = self.env['CLANGPP'] or self.env['CXX']
    self.env['CXX'] = self.env['CLANGPP'] or self.env['CXX']
    if sys.platform == "darwin":
        self.env['shlib_CXXFLAGS'] = ['-fPIC']

@feature('c')
def apply_clang_llvm(self):
    if self.env['HAVE_LLVM'] == False:
      return
    #self.env['AR'] = self.env['LLVM-AR'] or self.env['AR']
    self.env['LINK_CC'] = self.env['LLVM-LD'] or self.env['LINK_CC']
    self.env['LINK_CXX'] = self.env['LLVM-LD'] or self.env['LINK_CXX']
    self.env['STLIB_MARKER'] = ''
    self.env['SHLIB_MARKER'] = ''

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
    conf.find_program('clang', var='CLANG')
    conf.find_program('clang++', var='CLANGPP', path_list = search_paths)
    conf.find_program('llvm-ld', var='LLVM-LD', path_list = search_paths)
    conf.find_program('llvm-ar', var='LLVM-AR', path_list = search_paths)
    if conf.env['LLVM-LD'] == None or conf.env['LLVM-AR'] == None or conf.env['CLANG'] == None or conf.env['CLANGPP'] == None:
      conf.env['HAVE_LLVM'] = False
    else:
      conf.env['HAVE_LLVM'] = True      

