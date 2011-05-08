// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "term_namespace.h"

namespace circa {

void TermNamespace::bind(Term* term, std::string name)
{
    ca_assert(term != NULL);
    ca_assert(name != "");
    _map[name] = term;
}

void TermNamespace::remapPointers(TermMap const& remapping)
{
    StringToTermMap::iterator it;
    for (it = _map.begin(); it != _map.end(); ) {
        Term* replacement = remapping.getRemapped(it->second);
        if (replacement != it->second) {
            if (replacement == NULL) {
                _map.erase(it++);
                continue;
            }
            else {
                _map[it->first] = replacement;
            }
        }
        ++it;
    }
}

} // namespace circa
