// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "simple_handle.h"

#define SIMPLE_HANDLE_VERBOSE_LOG 0

namespace circa {
namespace simple_handle_t {

    void set(Type* type, TaggedValue* value, int handle)
    {
        change_type_no_initialize(value, type);
        value->value_data.ptr = allocate_list(1);
        set_int(list_get_element(value, 0), handle);

        #if SIMPLE_HANDLE_VERBOSE_LOG
        std::cout << "allocated " << value->value_data.ptr
            << " at " << value << std::endl;
        #endif
    }
    int get(TaggedValue* value)
    {
        return as_int(list_get_element(value, 0));
    }

    void initialize(Type* type, TaggedValue* value)
    {
        value->value_data.ptr = NULL;
        //internal_error("don't call simple_handle::initialize");
    }
    void release(Type* type, TaggedValue* value)
    {
        ListData* data = (ListData*) value->value_data.ptr;

        if (data == NULL)
            return;

        ca_assert(data->refCount > 0);

        data->refCount--;

        #if SIMPLE_HANDLE_VERBOSE_LOG
        std::cout << "released " << data
            << " from " << value << ", refCount = "
            << data->refCount << std::endl;
        #endif

        if (data->refCount == 0) {

            int handle = as_int(list_get_element(value, 0));
            OnRelease onReleaseFunc = (OnRelease) as_opaque_pointer(&type->parameter);

            onReleaseFunc(handle);
            
            free_list(data);
            value->value_data.ptr = NULL;
        }
    }
    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type_no_initialize(dest, type);
        dest->value_data = source->value_data;

        ListData* data = (ListData*) source->value_data.ptr;

        ca_assert(data->refCount > 0);

        data->refCount++;

        #if SIMPLE_HANDLE_VERBOSE_LOG
        std::cout << "copied " << data
            << " from " << source << " to " << dest << ", refCount = "
            << data->refCount << std::endl;
        #endif
    }

    void setup_type(Type* type)
    {
        type->name = "simple_handle";
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
        type->storageType = STORAGE_TYPE_LIST;
        type->toString = list_t::tv_to_string;
    }
}
}
