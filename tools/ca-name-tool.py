#!/usr/bin/env python

"""
This script generates the contents of src/names_builtin.h and src/names_builtin.cpp, using
src/names.txt as source.

In Circa land, a 'name' is an integer that has a human-readable string name. Each name can be
converted to and from a string. (these conversion functions are part of what gets
auto-generated).
"""

SymbolList = 'src/names.txt'

GeneratedHeader = 'src/names_builtin.h'
GeneratedImpl = 'src/names_builtin.cpp'

def get_c_name(name):
    if name.startswith('tok_') or name.startswith('stat_') or name.startswith('op_'):
        return name
    else:
        return "name_" + name

def every_symbol():
    f = open(SymbolList, 'r')
    index = 0

    while True:
        line = f.readline()
        if not line:
            break

        line = line[:-1]

        if not line:
            continue

        if line[0] == '#':
            continue

        yield (line, get_c_name(line), index)
        index += 1

    yield ('LastBuiltinName', 'name_LastBuiltinName', index)

def names_to_trie(names, i):
    by_prefix = {}

    for name in names:

        if i >= len(name):
            c = 0
        else:
            c = name[i]

        if c not in by_prefix:
            by_prefix[c] = []

        by_prefix[c].append(name)

    result = {}
    for prefix,contains in by_prefix.items():
        if len(contains) == 1:
            assert len(contains) == 1
            result[prefix] = contains[0]
        else:
            result[prefix] = names_to_trie(contains, i + 1)

    return result

def write_header(lines):
    lines.append('// Copyright (c) Andrew Fischer. See LICENSE file for license terms.')
    lines.append('')
    lines.append('// This file was generated using name_tool.py')
    lines.append('')
    lines.append('#pragma once')
    lines.append('')
    lines.append('namespace circa {')
    lines.append('')

    for (name,cname,index) in every_symbol():
        lines.append('const int ' + cname + ' = ' + str(index) + ';')
    
    lines.append('')
    lines.append('const char* builtin_name_to_string(int name);')
    lines.append('int builtin_name_from_string(const char* str);')
    lines.append('')
    lines.append('} // namespace circa')

def write_impl(lines):
    lines.append('// Copyright (c) Andrew Fischer. See LICENSE file for license terms.')
    lines.append('')
    lines.append('// This file was generated using name_tool.py')
    lines.append('')
    lines.append('#include "common_headers.h"')
    lines.append('#include "names_builtin.h"')
    lines.append('')
    lines.append('namespace circa {')
    lines.append('')
    lines.append('const char* builtin_name_to_string(int name)')
    lines.append('{')
    lines.append('    if (name >= name_LastBuiltinName)')
    lines.append('        return NULL;')
    lines.append('')
    lines.append('    switch (name) {')

    for (name,cname,_) in every_symbol():
        lines.append('    case '+cname+': return "'+name+'";')

    lines.append('    default: return NULL;')
    lines.append('    }')
    lines.append('}')
    lines.append('')
    lines.append('int builtin_name_from_string(const char* str)')
    lines.append('{')

    def write_name_lookup_switch(trie, dist):
        yield "switch (str[" + str(dist) + "]) {"
        yield "default: return -1;"
        for prefix,sub in trie.items():
            yield "case " + ('0' if prefix == 0 else "'" + prefix + "'") + ':'
            if type(sub) == str:
                yield "    if (strcmp(str + " + str(dist+1) + ", \"" + sub[dist+1:] + "\") == 0)"
                yield "        return " + get_c_name(sub) + ";"
                yield "    break;"
            else:
                for line in write_name_lookup_switch(sub, dist + 1):
                    yield line
        yield "}"

    for line in write_name_lookup_switch(names_to_trie([name for (name,_,_) in every_symbol()], 0), 0):
        lines.append('    ' + line)

    lines.append('')
    lines.append('    return -1;')
    lines.append('}')
    lines.append('} // namespace circa')

def save(lines, filename):
    f = open(filename, 'w')
    for line in lines:
        f.write(line + '\n')
    f.close()

l = []
write_header(l)
save(l, GeneratedHeader)

l = []
write_impl(l)
save(l, GeneratedImpl)


