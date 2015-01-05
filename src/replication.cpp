// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "hashtable.h"
#include "list.h"
#include "replication.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

static void set_patch_replace(Value* newValue, Value* result)
{
    result->append()->set_list(2)->set_element_sym(0, s_Replace)->set_element(1, newValue);
}

static void compute_patch_list(Value* base, Value* latest, Value* patch, Value* error)
{
    // Future: Look for insert/deletion

    for (int i=0; i < list_length(latest); i++) {
        if (i >= list_length(base)) {
            Value* elementPatch = patch->append()->set_list(2);
            elementPatch->set_element_sym(0, s_Append);
            elementPatch->set_element(1, latest->index(i));
        }
            
        else if (!strict_equals(base->index(i), latest->index(i))) {
            Value* elementPatch = patch->append()->set_list(3);
            elementPatch->set_element_sym(0, s_Element);
            elementPatch->set_element_int(1, i);
            compute_value_patch(base->index(i), latest->index(i),
                elementPatch->index(2), error);
            if (!is_null(error))
                return;
        }
    }

    if (list_length(latest) < list_length(base)) {
        Value* p = patch->append()->set_list(2);
        p->set_element_sym(0, s_Truncate);
        p->set_element_int(1, list_length(latest));
    }
}

static void compute_patch_hashtable(Value* base, Value* latest, Value* patch, Value* error)
{
    for (HashtableIterator it(latest); it; ++it) {
        Value* key = it.currentKey();
        Value* old = hashtable_get(base, key);

        if (old == NULL) {
            Value* elementPatch = patch->append()->set_list(3);
            elementPatch->set_element_sym(0, s_Insert);
            elementPatch->set_element(1, key);
            elementPatch->set_element(2, it.current());

        } else {
            Value* elementPatch = patch->append()->set_list(3);
            elementPatch->set_element_sym(0, s_Key);
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
            Value* elementPatch = patch->append()->set_list(2);
            elementPatch->set_element_sym(0, s_Delete);
            elementPatch->set_element(1, key);
        }
    }
}

void compute_value_patch(Value* base, Value* latest, Value* patch, Value* error)
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

void apply_patch(Value* val, Value* patch)
{
    if (is_null(patch))
        return;

    touch(val);

    for (int i=0; i < list_length(patch); i++) {
        Value* patchItem = patch->index(i);

        switch (first_symbol(patchItem)) {
        case s_Replace:
            set_value(val, patchItem->index(1));
            break;
        case s_Append:
            copy(patchItem->index(1), list_append(val));
            break;
        case s_Truncate:
            list_resize(val, patchItem->index(1)->asInt());
            break;
        case s_Insert:
            set_value(hashtable_insert(val, patchItem->index(1)), patchItem->index(2));
            break;
        case s_Delete:
            hashtable_remove(val, patchItem->index(1));
            break;
        case s_Element:
            apply_patch(val->index(patchItem->index(1)->asInt()), patchItem->index(2));
            break;
        case s_Key:
            apply_patch(hashtable_get(val, patchItem->index(1)), patchItem->index(2));
            break;
        }
    }

}

} // namespace circa
