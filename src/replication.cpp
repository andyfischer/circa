// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "hashtable.h"
#include "list.h"
#include "replication.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

static void set_patch_replace(caValue* newValue, caValue* result)
{
    result->append()->set_list(2)->set_element_sym(0, sym_Replace)->set_element(1, newValue);
}

static void compute_patch_list(caValue* base, caValue* latest, caValue* patch, caValue* error)
{
    // Future: Look for insert/deletion

    for (int i=0; i < list_length(latest); i++) {
        if (i >= list_length(base)) {
            caValue* elementPatch = patch->append()->set_list(2);
            elementPatch->set_element_sym(0, sym_Append);
            elementPatch->set_element(1, latest->index(i));
        }
            
        else if (!strict_equals(base->index(i), latest->index(i))) {
            caValue* elementPatch = patch->append()->set_list(3);
            elementPatch->set_element_sym(0, sym_Element);
            elementPatch->set_element_int(1, i);
            compute_value_patch(base->index(i), latest->index(i),
                elementPatch->index(2), error);
            if (!is_null(error))
                return;
        }
    }

    if (list_length(latest) < list_length(base)) {
        caValue* p = patch->append()->set_list(2);
        p->set_element_sym(0, sym_Truncate);
        p->set_element_int(1, list_length(latest));
    }
}

static void compute_patch_hashtable(caValue* base, caValue* latest, caValue* patch, caValue* error)
{
    for (HashtableIterator it(latest); it; ++it) {
        Value* key = it.currentKey();
        Value* old = hashtable_get(base, key);

        if (old == NULL) {
            caValue* elementPatch = patch->append()->set_list(3);
            elementPatch->set_element_sym(0, sym_Insert);
            elementPatch->set_element(1, key);
            elementPatch->set_element(2, it.current());

        } else {
            caValue* elementPatch = patch->append()->set_list(3);
            elementPatch->set_element_sym(0, sym_Key);
            elementPatch->set_element(1, key);
            compute_value_patch(old, it.current(), elementPatch->index(2), error);
            if (!is_null(error))
                return;
        }
    }

    for (HashtableIterator it(base); it; ++it) {
        Value* key = it.currentKey();
        Value* newValue = hashtable_get(latest, key);
        if (newValue == NULL) {
            caValue* elementPatch = patch->append()->set_list(2);
            elementPatch->set_element_sym(0, sym_Delete);
            elementPatch->set_element(1, key);
        }
    }
}

void compute_value_patch(caValue* base, caValue* latest, caValue* patch, caValue* error)
{
    set_list(patch);

    if (strict_equals(base, latest))
        return;

    if (is_list_based(latest)) {
        if (!is_list_based(base)) {
            set_patch_replace(latest, patch);
            return;
        }

        compute_patch_list(base, latest, patch, error);
        return;
    }

    if (is_hashtable(latest)) {
        if (!is_hashtable(base)) {
            set_patch_replace(latest, patch);
            return;
        }
        compute_patch_hashtable(base, latest, patch, error);
        return;
    }

    if (is_string(latest)
            || is_bool(latest)
            || is_float(latest)
            || is_int(latest)
            || is_symbol(latest)) {
        set_patch_replace(latest, patch);
        return;
    }

    set_string(error, "Can't replicate value of type ");
    string_append(error, &latest->value_type->name);
}

void apply_patch(caValue* val, caValue* patch)
{
    if (is_null(patch))
        return;

    touch(val);

    for (int i=0; i < list_length(patch); i++) {
        Value* patchItem = patch->index(i);

        switch (first_symbol(patchItem)) {
        case sym_Replace:
            set_value(val, patchItem->index(1));
            break;
        case sym_Append:
            copy(patchItem->index(1), list_append(val));
            break;
        case sym_Truncate:
            list_resize(val, patchItem->index(1)->asInt());
            break;
        case sym_Insert:
            set_value(hashtable_insert(val, patchItem->index(1)), patchItem->index(2));
            break;
        case sym_Delete:
            hashtable_remove(val, patchItem->index(1));
            break;
        case sym_Element:
            apply_patch(val->index(patchItem->index(1)->asInt()), patchItem->index(2));
            break;
        case sym_Key:
            apply_patch(hashtable_get(val, patchItem->index(1)), patchItem->index(2));
            break;
        }
    }

}

} // namespace circa
