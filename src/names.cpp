// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

Term* find_named(Branch& branch, std::string const& name)
{
    assert(name != "");
    assert(name == "#out" || name[0] != '#'); // We shouldn't ever lookup hidden names

    if (branch.contains(name))
        return branch[name];

    Branch* outerScope = get_outer_scope(branch);

    if (outerScope == NULL)
        return get_global(name);
    else
        return find_named(*outerScope, name);
}

Term* get_named(Branch& branch, std::string const& name)
{
    if (branch.contains(name))
        return branch[name];
    else
        return NULL;
}

Term* get_dot_separated_name(Branch& branch, std::string const& name)
{
    int dotPos = -1;
    for (int i=0; name[i] != 0; i++) {
        if (name[i] == '.') {
            dotPos = i;
            break;
        }
    }

    if (dotPos == -1)
        return get_named(branch, name);

    if (dotPos == 0)
        return NULL;

    std::string head = name.substr(0, dotPos);
    std::string tail = name.substr(dotPos+1, name.length());

    if (!branch.contains(head))
        return NULL;

    Term* headTerm = branch[head];

    if (!is_branch(headTerm))
        return NULL;

    return get_dot_separated_name(as_branch(headTerm), tail);
}

void expose_all_names(Branch& source, Branch& destination)
{
    for (TermNamespace::iterator it = source.names.begin(); it != source.names.end(); ++it)
    {
        std::string const& name = it->first;
        Term* term = it->second;
        if (name == "") continue;
        if (name[0] == '#' && name != "#out") continue;

        destination.bindName(term, name);
    }
}

} // namespace circa
