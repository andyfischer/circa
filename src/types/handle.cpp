// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "list_shared.h"

#include "handle.h"

#define HANDLE_VERBOSE_LOG 0

namespace circa {
namespace handle_t {

    void set(Value* container, Type* type, Value* userdata)
    {
        change_type_no_initialize(container, type);
        container->value_data.ptr = allocate_list(1);
        copy(userdata, list_get_element(container, 0));

        #if HANDLE_VERBOSE_LOG
        ListData* data = (ListData*) container->value_data.ptr;
        std::cout << "initialized "
            << data
            << " " << container->toString()
            << " at " << container
            << ", refcount = " << data->refCount
            << std::endl;
        #endif
    }
    void set(Value* container, Type* type, void* opaquePointer)
    {
        Value value;
        set_opaque_pointer(&value, opaquePointer);
        set(container, type, &value);
    }
    Value* get(Value* container)
    {
        return list_get_element(container, 0);
    }

    void initialize(Type* type, Value* value)
    {
        value->value_data.ptr = NULL;
        //internal_error("don't call simple_handle::initialize");
    }
    void release(Type* type, Value* value)
    {
        ListData* data = (ListData*) value->value_data.ptr;

        if (data == NULL)
            return;

        ca_assert(data->refCount > 0);

        data->refCount--;

        #if HANDLE_VERBOSE_LOG
        std::cout << "decref " << data
            << " " << value->toString()
            << " from " << value << ", refCount = "
            << data->refCount << std::endl;
        #endif

        if (data->refCount == 0) {

            Value* userdata = list_get_element(value, 0);
            OnRelease onReleaseFunc = (OnRelease) as_opaque_pointer(&type->parameter);

            onReleaseFunc(userdata);

            #if HANDLE_VERBOSE_LOG
            std::cout << "released " << data
                << " " << value->toString()
                << std::endl;
            #endif

            
            free_list(data);
            value->value_data.ptr = NULL;
        }
    }
    void copy(Type* type, Value* source, Value* dest)
    {
        change_type_no_initialize(dest, type);
        dest->value_data = source->value_data;

        ListData* data = (ListData*) source->value_data.ptr;

        ca_assert(data->refCount > 0);

        data->refCount++;

        #if HANDLE_VERBOSE_LOG
        std::cout << "copied " << data
            << " " << source->toString()
            << " from " << source << " to " << dest << ", refCount = "
            << data->refCount << std::endl;
        #endif
    }

    void setup_type(Type* type)
    {
        type->name = "handle";
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
        type->storageType = STORAGE_TYPE_LIST;
        type->toString = list_t::tv_to_string;
    }

} // namespace handle_t
} // namespace circa
