#############################################################################
## Copyright (c) 2009-2011 Alan Wright. All rights reserved.
## Distributable under the terms of either the Apache License (Version 2.0)
## or the GNU Lesser General Public License.
#############################################################################

#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2006 (ita)

"""
for some obscure reason, the precompiled header will not be taken if
all.h is in the same directory as main.cpp
we recommend to add the header to compile in a separate directory without any sources

Note: the #warning will come once when the .h is compiled, it will not come when the .cpp is compiled
Note: do not forget to set the include paths (include=...)
"""

from waflib.TaskGen import feature, after
from waflib.Task import Task
from waflib.Tools import c_preproc

#@feature('cxx') <- python >= 2.4
#@after('apply_link')
def process_pch(self):
    if getattr(self, 'pch', ''):
        nodes = self.to_nodes(self.pch)
        for x in nodes:
            self.create_task('gchx', x, x.change_ext('.gch'))
feature('cxx')(process_pch)
after('apply_link')(process_pch)

class gchx(Task):
    run_str = '${CXX} ${CXXFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${CXX_SRC_F}${SRC} ${CXX_TGT_F}${TGT}'
    scan    = c_preproc.scan
    ext_out = ['.h']
    color   = 'BLUE'

