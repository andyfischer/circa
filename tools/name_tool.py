
SymbolList = 'src/names.txt'

GeneratedHeader = 'src/names_builtin.h'
GeneratedImpl = 'src/names_builtin.cpp'

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

        yield (line,index)
        index += 1

    yield ('LastBuiltinName', index)

def write_header(lines):
    lines.append('// Copyright (c) Andrew Fischer. See LICENSE file for license terms.')
    lines.append('')
    lines.append('// This file was generated using name_tool.py')
    lines.append('')
    lines.append('#pragma once')
    lines.append('')
    lines.append('namespace circa {')
    lines.append('')

    for (name,index) in every_symbol():
        lines.append('const int name_' + name + ' = ' + str(index) + ';')
    
    lines.append('')
    lines.append('const char* builtin_name_to_string(int name);')
    lines.append('')
    lines.append('} // namespace circa')

def write_impl(lines):
    lines.append('// Copyright (c) Andrew Fischer. See LICENSE file for license terms.')
    lines.append('')
    lines.append('// This file was generated using name_tool.py')
    lines.append('')
    lines.append('namespace circa {')
    lines.append('')
    lines.append('const char* builtin_name_to_string(int name)')
    lines.append('{')
    lines.append('    if (name >= name_LastBuiltinName)')
    lines.append('        return NULL;')
    lines.append('')
    lines.append('    switch (name) {')

    for (name,_) in every_symbol():
        lines.append('    case name_'+name+': return "'+name+'";')

    lines.append('    default: return NULL;')
    lines.append('    }')
    lines.append('}')
    lines.append('')
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


