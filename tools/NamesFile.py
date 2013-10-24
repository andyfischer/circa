#!/usr/bin/env python3

"""
This script generates the contents of src/names_builtin.h and src/names_builtin.cpp, using
src/names.txt as source.

In Circa land, a 'name' is an integer that has a human-readable string name. Each name can be
converted to and from a string. (these conversion functions are part of what gets
auto-generated).
"""

import os

SymbolList = 'src/names.txt'

GeneratedHeader = 'src/names_builtin.h'
GeneratedImpl = 'src/names_builtin.cpp'

def get_c_name(name):
    if name.startswith('tok_') or name.startswith('stat_') or name.startswith('op_'):
        return name
    else:
        return "sym_" + name

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

    yield ('LastBuiltinName', 'sym_LastBuiltinName', index)

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

    for prefix,contains in sorted(by_prefix.items(), key=str):
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
    lines.append('const char* builtin_symbol_to_string(int name);')
    lines.append('int builtin_symbol_from_string(const char* str);')
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
    lines.append('const char* builtin_symbol_to_string(int name)')
    lines.append('{')
    lines.append('    if (name >= sym_LastBuiltinName)')
    lines.append('        return NULL;')
    lines.append('')
    lines.append('    switch (name) {')

    for (name,cname,_) in every_symbol():
        lines.append('    case '+cname+': return "'+name+'";')

    lines.append('    default: return NULL;')
    lines.append('    }')
    lines.append('}')
    lines.append('')
    lines.append('int builtin_symbol_from_string(const char* str)')
    lines.append('{')

    def write_name_lookup_switch(trie, dist):
        yield "switch (str[" + str(dist) + "]) {"
        for prefix,sub in sorted(trie.items(), key=str):
            yield "case " + ('0' if prefix == 0 else "'" + prefix + "'") + ':'
            if type(sub) == str:
                # This leaf only has a string value, meaning that it can be only one possible thing.

                if (dist+1) >= len(sub):
                    # We've already checked every character.
                    yield "        return " + get_c_name(sub) + ";"
                else:
                    # There is still the remainder of this match to check. We'll either
                    # match this string or match nothing.
                    yield "    if (strcmp(str + " + str(dist+1) + ", \"" + sub[dist+1:] + "\") == 0)"
                    yield "        return " + get_c_name(sub) + ";"
                    yield "    break;"
            else:
                for line in write_name_lookup_switch(sub, dist + 1):
                    yield line
        yield "default: return -1;"
        yield "}"

    for line in write_name_lookup_switch(names_to_trie([name for (name,_,_) in every_symbol()], 0), 0):
        lines.append('    ' + line)

    lines.append('')
    lines.append('    return -1;')
    lines.append('}')
    lines.append('} // namespace circa')

def read_text_file(path):
    if not os.path.exists(path):
        return ""
    f = open(path, 'r')
    return f.read()[:-1]

def save(lines, filename):
    newContents = '\n'.join(lines)
    if newContents == read_text_file(filename):
        return
    f = open(filename, 'w')
    f.write(newContents)
    f.write("\n")

def regenerate_file():
    l = []
    write_header(l)
    save(l, GeneratedHeader)

    l = []
    write_impl(l)
    save(l, GeneratedImpl)

