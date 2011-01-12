#############################################################################
## Copyright (c) 2009-2011 Alan Wright. All rights reserved.
## Distributable under the terms of either the Apache License (Version 2.0)
## or the GNU Lesser General Public License.
#############################################################################

from TaskGen import feature, after
from copy import copy
import Task, ccroot
import os

cmd = '${CXX} ${CXXFLAGS} ${_CXXINCFLAGS} ${_CXXDEFFLAGS} ${SRC} -o ${TGT}'
cls = Task.simple_task_type('gch', cmd, before='cc cxx', shell = False)
cls.scan = ccroot.scan

def requires_pch(task, pch_name):
    """
    Determines if a task requires a PCH prefix header
    Assume that missing source files equate to "generated" sources
    and will require a PCH header.
    """
    def is_include_line(line):
        line = line.strip()
        return line.startswith("#include") or line.startswith("#import")
    generated_sources = False
    for source_file in task.inputs:
        if not os.path.isfile(source_file.abspath()):
            generated_sources = True
            continue
        source_path = source_file.abspath()
        if source_file.suffix() not in [".cpp", ".c"]:
            continue
        for line in open(source_path):
            if is_include_line(line):
                return line.find(pch_name) > 0
    return generated_sources


@feature('cxx')
@after('apply_link', 'apply_incpaths')
def process_pch(self):
    """
    Routine to add PCH generation if a pch header was specified
    for a target.
    """
    if not getattr(self, 'pch', None):
        return 
    node = self.path.find_resource(self.pch)
    if not node:
        raise Exception("Invalid PCH specified for %s" % self)
    output = node.parent.find_or_declare(node.name + '.gch')
    pch_task = self.create_task('gch')
    pch_task.set_inputs(node)
    pch_task.set_outputs(output)
    altered_envs = []
    for task in self.compiled_tasks:
        if not requires_pch(task, self.pch):
            continue   
        task.env = task.env.copy()
        task.env.prepend_value("CXXFLAGS", "-include%s" % node.name)

