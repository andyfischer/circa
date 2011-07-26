// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <string>
#include <sstream>

#include "build_options.h"
#include "heap_debugging.h"
#include "importing_macros.h"
#include "tagged_value.h"
#include "testing.h"

#include "type.h"

#include "list_shared.h"
#include "list.h"

namespace circa {
namespace list_t {

    void clear(ListData** data)
    {
        if (*data == NULL) return;
        list_decref(*data);
        *data = NULL;
    }

    TaggedValue* append(ListData** data)
    {
        if (*data == NULL) {
            *data = allocate_empty_list(1);
        } else {
            *data = list_touch(*data);
            
            if ((*data)->count == (*data)->capacity)
                *data = list_double_capacity(*data);
        }

        ListData* d = *data;
        d->count++;
        return &d->items[d->count - 1];
    }

    int num_elements(ListData* list)
    {
        if (list == NULL) return 0;
        return list->count;
    }

    int refcount(ListData* value)
    {
        if (value == NULL) return 0;
        return value->refCount;
    }

    TaggedValue* prepend(ListData** data)
    {
        append(data);

        ListData* d = *data;

        for (int i=d->count - 1; i >= 1; i--)
            swap(&d->items[i], &d->items[i - 1]);

        return &d->items[0];
    }

    std::string to_string(ListData* value)
    {
        if (value == NULL)
            return "[]";

        std::stringstream out;
        out << "[";
        for (int i=0; i < value->count; i++) {
            if (i > 0) out << ", ";
            out << to_string(&value->items[i]);
        }
        out << "]";
        return out.str();
    }

    // TaggedValue wrappers
    void tv_touch(TaggedValue* value);

    void resize(TaggedValue* list, int newSize)
    {
        ca_assert(is_list(list));
        set_pointer(list, list_resize((ListData*) get_pointer(list), newSize));
    }

    void clear(TaggedValue* list)
    {
        ca_assert(is_list(list));
        clear((ListData**) &list->value_data);
    }

    TaggedValue* append(TaggedValue* list)
    {
        ca_assert(is_list(list));
        return append((ListData**) &list->value_data);
    }
    TaggedValue* prepend(TaggedValue* list)
    {
        ca_assert(is_list(list));
        return prepend((ListData**) &list->value_data);
    }

    void tv_initialize(Type* type, TaggedValue* value)
    {
        ca_assert(value->value_data.ptr == NULL);

        // If the parameter has a fixed size list, then initialize to that.
        if (list_type_has_specific_size(&type->parameter)) {
            TaggedValue* typeList = list_get_type_list_from_parameter(&type->parameter);
            List& types = *List::checkCast(typeList);
            List& result = *List::checkCast(value);
            result.resize(types.length());
            for (int i=0; i < types.length(); i++)
                change_type(result[i], as_type(types[i]));
        }
    }

    void tv_release(Type*, TaggedValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        if (data == NULL) return;
        list_decref(data);
    }

    void tv_copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        ca_assert(is_list(source));
        set_null(dest);

        ListData* s = (ListData*) get_pointer(source);
        set_pointer(dest, list_duplicate(s));
        change_type_no_initialize(dest, type);

        #if 0
    #if CIRCA_DISABLE_LIST_SHARING
        if (d != NULL) list_decref(d);
    #else
        if (s != NULL) incref(s);
        if (d != NULL) list_decref(d);
        set_pointer(dest, s);
    #endif
        #endif
    }

    bool tv_equals(Type*, TaggedValue* leftTaggedValue, TaggedValue* right)
    {
        ca_assert(is_list(leftTaggedValue));
        Type* rhsType = right->value_type;
        if (rhsType->numElements == NULL || rhsType->getIndex == NULL)
            return false;

        List* left = List::checkCast(leftTaggedValue);

        int leftCount = left->numElements();

        if (leftCount != right->numElements())
            return false;

        for (int i=0; i < leftCount; i++) {
            if (!circa::equals(left->get(i), right->getIndex(i)))
                return false;
        }
        return true;
    }

    void tv_cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly)
    {
        List* sourceList = List::checkCast(source);

        if (sourceList == NULL) {
            result->success = false;
            return;
        }

        int sourceLength = sourceList->length();

        // If the destination list type doesn't have a specific size restriction,
        // then just copy the source list and call it a day.
        if (!list_type_has_specific_size(&type->parameter)) {
            if (!checkOnly) {
                copy(source, dest);
                dest->value_type = type;
            }
            return;
        }

        List& destTypes = *List::checkCast(
            list_get_type_list_from_parameter(&type->parameter));

        // Check for correct number of elements.
        if (sourceLength != destTypes.length()) {
            result->success = false;
            return;
        }

        List* destList = NULL;

        if (!checkOnly) {
            destList = List::lazyCast(dest);
            destList->resize(sourceLength);
            dest->value_type = type;
        }

        for (int i=0; i < sourceLength; i++) {
            TaggedValue* sourceElement = sourceList->get(i);
            Type* expectedType = as_type(destTypes[i]);

            TaggedValue* destElement = NULL;
            if (!checkOnly)
                destElement = destList->get(i);
        
            cast(result, sourceElement, expectedType, destElement, checkOnly);

            if (!result->success)
                return;
        }

        if (!checkOnly)
            dest->value_type = type;
    }

    void tv_set_index(TaggedValue* value, int index, TaggedValue* element)
    {
        ca_assert(is_list(value));
        ListData* s = (ListData*) get_pointer(value);
        list_set_index(s, index, element);
    }

    TaggedValue* tv_get_field(TaggedValue* value, const char* fieldName)
    {
        int index = value->value_type->findFieldIndex(fieldName);
        if (index < 0)
            return NULL;
        return list_get_index(value, index);
    }

    std::string tv_to_string(TaggedValue* value)
    {
        ca_assert(is_list(value));
        return to_string((ListData*) get_pointer(value));
    }

    void tv_touch(TaggedValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        set_pointer(value, list_touch(data));
    }

    void tv_static_type_query(Type* type, StaticTypeQuery* query)
    {
        Term* subject = query->subject;
        Branch& prototype = type->prototype;
        
        // If prototype is empty then accept any list
        if (prototype.length() == 0) {
            if (is_list_based_type(unbox_type(subject->type)))
                return query->succeed();
            else
                return query->fail();
        }

        // Special case when looking at a call to list(); look at inputs instead of
        // looking at the result.
        if (subject->function == LIST_FUNC)
        {
            if (subject->numInputs() != prototype.length())
                return query->fail();

            for (int i=0; i < prototype.length(); i++)
                if (!circa::term_output_always_satisfies_type(
                            subject->input(i), unbox_type(prototype[i]->type)))
                    return query->fail();

            return query->succeed();
        }

        // Look at the subject's prototype.
        Branch& subjectPrototype = query->subjectType->prototype;

        bool anyUnableToDetermine = false;

        for (int i=0; i < prototype.length(); i++) {
            if (i >= subjectPrototype.length())
                return query->fail();

            StaticTypeQuery::Result result = run_static_type_query(
                    declared_type(prototype[i]), subjectPrototype[i]);

            // If any of these fail, then fail.
            if (result == StaticTypeQuery::FAIL)
                return query->fail();

            if (result == StaticTypeQuery::UNABLE_TO_DETERMINE)
                anyUnableToDetermine = true;
        }

        if (anyUnableToDetermine)
            return query->unableToDetermine();
        else
            return query->succeed();
    }

    void tv_visit_heap(Type*, TaggedValue* value, Type::VisitHeapCallback callback, TaggedValue* context)
    {
        ListData* data = (ListData*) value->value_data.ptr;
        if (data == NULL)
            return;
        TaggedValue relativeIdentifier;
        for (int i=0; i < data->count; i++) {
            set_int(&relativeIdentifier, i);
            callback(&data->items[i], &relativeIdentifier, context);
        }
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "List";
        type->storageType = STORAGE_TYPE_LIST;
        type->initialize = tv_initialize;
        type->release = tv_release;
        type->copy = tv_copy;
        type->toString = tv_to_string;
        type->equals = tv_equals;
        type->cast = tv_cast;
        type->getIndex = list_get_index;
        type->setIndex = tv_set_index;
        type->getField = tv_get_field;
        type->numElements = list_get_length;
        type->touch = tv_touch;
        type->staticTypeQuery = tv_static_type_query;
        type->visitHeap = tv_visit_heap;
    }

    CA_FUNCTION(append)
    {
        copy(INPUT(0), OUTPUT);
        List* result = List::checkCast(OUTPUT);
        TaggedValue* value = INPUT(1);
        copy(value, result->append());
    }

    CA_FUNCTION(count)
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }

} // namespace list_t

bool is_list_based_type(Type* type)
{
    return type->initialize == list_t::tv_initialize;
}

List::List()
  : TaggedValue()
{
    change_type(this, &LIST_T);
}

TaggedValue*
List::append()
{
    return list_t::append((TaggedValue*) this);
}
void
List::append(TaggedValue* val)
{
    copy(val, append());
}

TaggedValue*
List::prepend()
{
    return list_t::prepend((TaggedValue*) this);
}

void
List::clear()
{
    list_t::clear((TaggedValue*) this);
}

int
List::length()
{
    return list_get_length(this);
}

bool
List::empty()
{
    return length() == 0;
}

TaggedValue*
List::get(int index)
{
    return list_get_index((TaggedValue*) this, index);
}

void
List::set(int index, TaggedValue* value)
{
    list_t::tv_set_index((TaggedValue*) this, index, value);
}

void
List::resize(int newSize)
{
    list_t::resize(this, newSize); 
}

TaggedValue*
List::getLast()
{
    return get(length() - 1);
}

void
List::pop()
{
    resize(length() - 1);
}

void
List::remove(int index)
{
    list_remove_index(this, index);
}

void
List::removeNulls()
{
    list_remove_nulls(this);
}

List*
List::checkCast(TaggedValue* v)
{
    if (is_list(v))
        return (List*) v;
    else
        return NULL;
}

List*
List::lazyCast(TaggedValue* v)
{
    if (!is_list(v))
        set_list(v, 0);
    return (List*) v;
}
List*
List::cast(TaggedValue* v, int length)
{
    set_list(v, length);
    return (List*) v;
}

} // namespace circa
