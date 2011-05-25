// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// The Handle type allows you to have a shared object which is deallocated
// once all references to it are deleted.
//
// Using the 'set' function, you can create a handle value. The handle has
// type Handle, and it points to a refcounted list of length 1. When handles
// are copied, the list's refcount is updated.
//
// In the list is the 'userdata' value. This value is only initialized once,
// when created, and released only when all of the handles are released.
//

#include "circa.h"

#include "list_shared.h"

#include "handle.h"

#define HANDLE_VERBOSE_LOG 0

namespace circa {
namespace handle_t {

    void set(TaggedValue* handle, Type* type, TaggedValue* userdata)
    {
        change_type_no_initialize(handle, type);
        handle->value_data.ptr = allocate_list(1);
        swap(userdata, list_get_element(handle, 0));

        #if HANDLE_VERBOSE_LOG
        ListData* data = (ListData*) handle->value_data.ptr;
        std::cout << "initialized "
            << data
            << " " << handle->toString()
            << " at " << handle
            << ", refcount = " << data->refCount
            << std::endl;
        #endif
    }
    void set(TaggedValue* handle, Type* type, void* opaquePointer)
    {
        TaggedValue value;
        set_opaque_pointer(&value, opaquePointer);
        set(handle, type, &value);
    }
    TaggedValue* get(TaggedValue* handle)
    {
        return list_get_element(handle, 0);
    }
    void* get_ptr(TaggedValue* value)
    {
        return as_opaque_pointer(get(value));
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

        #if HANDLE_VERBOSE_LOG
        std::cout << "decref " << data
            << " " << value->toString()
            << " from " << value << ", refCount = "
            << data->refCount << std::endl;
        #endif

        if (data->refCount == 0) {

            TaggedValue* userdata = list_get_element(value, 0);

            if (!is_null(&type->parameter)) {
                OnRelease onReleaseFunc = (OnRelease) as_opaque_pointer(&type->parameter);

                if (onReleaseFunc != NULL)
                    onReleaseFunc(userdata);
            }

            #if HANDLE_VERBOSE_LOG
            std::cout << "released " << data
                << " " << value->toString()
                << std::endl;
            #endif

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

        #if HANDLE_VERBOSE_LOG
        std::cout << "copied " << data
            << " " << source->toString()
            << " from " << source << " to " << dest << ", refCount = "
            << data->refCount << std::endl;
        #endif
    }
    void visitHeap(Type* type, TaggedValue* handle, Type::VisitHeapCallback callback,
            TaggedValue* context)
    {
        TaggedValue relativeIdentifier;
        set_int(&relativeIdentifier, 0);
        callback(list_get_element(handle, 0), &relativeIdentifier, context);
    }
    void setup_type(Type* type)
    {
        type->name = "handle";
        type->storageType = STORAGE_TYPE_LIST;
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
        type->visitHeap = visitHeap;
        type->toString = list_t::tv_to_string;
    }

} // namespace handle_t
} // namespace circa
