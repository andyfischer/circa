// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <string>
#include <sstream>

#include "circa/internal/for_hosted_funcs.h"

#include "./list.h"

namespace circa {
namespace list_t {

    void clear(ListData** data)
    {
        if (*data == NULL) return;
        list_decref(*data);
        *data = NULL;
    }

    caValue* append(ListData** data)
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

    caValue* prepend(ListData** data)
    {
        append(data);

        ListData* d = *data;

        for (int i=d->count - 1; i >= 1; i--)
            swap(&d->items[i], &d->items[i - 1]);

        return &d->items[0];
    }


    // caValue wrappers
    void tv_touch(caValue* value);

    void resize(caValue* list, int newSize)
    {
        ca_assert(is_list(list));
        set_pointer(list, list_resize((ListData*) get_pointer(list), newSize));
    }

    void clear(caValue* list)
    {
        ca_assert(is_list(list));
        clear((ListData**) &list->value_data);
    }

    caValue* append(caValue* list)
    {
        ca_assert(is_list(list));
        return append((ListData**) &list->value_data);
    }
    caValue* prepend(caValue* list)
    {
        ca_assert(is_list(list));
        return prepend((ListData**) &list->value_data);
    }

    void tv_initialize(Type* type, caValue* value)
    {
        ca_assert(value->value_data.ptr == NULL);

        // If the parameter has a fixed size list, then initialize to that.
        if (list_type_has_specific_size(&type->parameter)) {
            caValue* typeList = list_get_type_list_from_type(type);
            List& types = *List::checkCast(typeList);
            List& result = *List::checkCast(value);
            result.resize(types.length());
            for (int i=0; i < types.length(); i++)
                create(as_type(types[i]), result[i]);
        }
    }

    void tv_release(caValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        if (data == NULL) return;
        list_decref(data);
    }

    void tv_copy(Type* type, caValue* source, caValue* dest)
    {
        ca_assert(is_list(source));
        set_null(dest);
        change_type(dest, type);

        ListData* s = (ListData*) get_pointer(source);
        set_pointer(dest, list_duplicate(s));

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

    bool tv_equals(Type*, caValue* leftcaValue, caValue* right)
    {
        ca_assert(is_list(leftcaValue));
        Type* rhsType = right->value_type;
        if (rhsType->numElements == NULL || rhsType->getIndex == NULL)
            return false;

        List* left = List::checkCast(leftcaValue);

        int leftCount = left->numElements();

        if (leftCount != right->numElements())
            return false;

        for (int i=0; i < leftCount; i++) {
            if (!circa::equals(left->get(i), right->getIndex(i)))
                return false;
        }
        return true;
    }

    void tv_cast(CastResult* result, caValue* source, Type* type,
        caValue* dest, bool checkOnly)
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

        List& destTypes = *List::checkCast(list_get_type_list_from_type(type));

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
            caValue* sourceElement = sourceList->get(i);
            Type* expectedType = as_type(destTypes[i]);

            caValue* destElement = NULL;
            if (!checkOnly)
                destElement = destList->get(i);
        
            cast(result, sourceElement, expectedType, destElement, checkOnly);

            if (!result->success)
                return;
        }

        if (!checkOnly)
            dest->value_type = type;
    }

    void tv_set_index(caValue* value, int index, caValue* element)
    {
        ca_assert(is_list(value));
        ListData* s = (ListData*) get_pointer(value);
        list_set_index(s, index, element);
    }

    caValue* tv_get_field(caValue* value, const char* fieldName)
    {
        int index = list_find_field_index_by_name(value->value_type, fieldName);
        if (index < 0)
            return NULL;
        return list_get_index(value, index);
    }

    std::string tv_to_string(caValue* value)
    {
        ca_assert(is_list(value));
        return list_to_string((ListData*) get_pointer(value));
    }

    void tv_touch(caValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        set_pointer(value, list_touch(data));
    }

    void tv_static_type_query(Type* type, StaticTypeQuery* query)
    {
        Term* subject = query->subject;
        Type* subjectType = query->subjectType;

        // If the type doesn't have a specific size, then accept any list.
        if (!list_type_has_specific_size(&type->parameter)) {
            if (is_list_based_type(subjectType))
                return query->succeed();
            else
                return query->fail();
        }

        List& expectedElementTypes = *as_list(list_get_type_list_from_type(type));

        // Special case when looking at a call to list(); look at inputs instead of
        // looking at the result.
        if (subject && subject->function == LIST_FUNC)
        {
            if (subject->numInputs() != expectedElementTypes.length())
                return query->fail();

            for (int i=0; i < expectedElementTypes.length(); i++)
                if (!circa::term_output_always_satisfies_type(
                            subject->input(i), as_type(expectedElementTypes[i])))
                    return query->fail();

            return query->succeed();
        }

        // Look at the subject's type.
        if (!list_type_has_specific_size(&subjectType->parameter))
            return query->fail();

        List& subjectElementTypes = *as_list(list_get_type_list_from_type(subjectType));

        bool anyUnableToDetermine = false;

        for (int i=0; i < expectedElementTypes.length(); i++) {
            if (i >= subjectElementTypes.length())
                return query->fail();

            StaticTypeQuery::Result result = run_static_type_query(
                    as_type(expectedElementTypes[i]), as_type(subjectElementTypes[i]));

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

    void tv_visit_heap(Type*, caValue* value, Type::VisitHeapCallback callback, caValue* context)
    {
        ListData* data = (ListData*) value->value_data.ptr;
        if (data == NULL)
            return;
        caValue relativeIdentifier;
        for (int i=0; i < data->count; i++) {
            set_int(&relativeIdentifier, i);
            callback(&data->items[i], &relativeIdentifier, context);
        }
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = name_from_string("List");
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
        caValue* value = INPUT(1);
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
  : caValue()
{
    create(&LIST_T, this);
}

caValue*
List::append()
{
    return list_t::append((caValue*) this);
}
void
List::append(caValue* val)
{
    copy(val, append());
}

caValue* List::insert(int index)
{
    return list_insert((ListData**) &this->value_data.ptr, index);
}

caValue*
List::prepend()
{
    return list_t::prepend((caValue*) this);
}

void
List::clear()
{
    list_t::clear((caValue*) this);
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

caValue*
List::get(int index)
{
    return list_get_index((caValue*) this, index);
}

void
List::set(int index, caValue* value)
{
    list_t::tv_set_index((caValue*) this, index, value);
}

void
List::resize(int newSize)
{
    list_t::resize(this, newSize); 
}

caValue*
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
void
List::appendString(const char* str)
{
    caValue val;
    set_string(&val, str);
    swap(&val, append());
}
void
List::appendString(const std::string& str)
{
    caValue val;
    set_string(&val, str);
    swap(&val, append());
}

List*
List::checkCast(caValue* v)
{
    if (is_list(v))
        return (List*) v;
    else
        return NULL;
}

List*
List::lazyCast(caValue* v)
{
    if (!is_list(v))
        set_list(v, 0);
    return (List*) v;
}
List*
List::cast(caValue* v, int length)
{
    set_list(v, length);
    return (List*) v;
}

} // namespace circa
