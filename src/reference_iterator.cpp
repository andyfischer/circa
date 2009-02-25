// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "reference_iterator.h"

namespace circa {

RefList reference_iterator_to_list(ReferenceIterator& iterator)
{
    RefList result;

    while (!iterator.finished()) {
        result.append(iterator.current());
        iterator.advance();
    }

    return result;
}

} // namespace circa
