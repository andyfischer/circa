// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

template <class T>
struct RefcountedPtr
{
    T* p;

    RefcountedPtr() : p(NULL) {}
    RefcountedPtr(Term *initial) : p(NULL) { set(initial); }
    RefcountedPtr(RefcountedPtr const& copy) : p(NULL) { set(copy.p); }
    ~RefcountedPtr() { set(NULL); }

    RefcountedPtr& operator=(RefcountedPtr const& rhs) { set(rhs.p); return *this; }
    RefcountedPtr& operator=(T* other) { set(other); return *this; }
    bool operator==(T* other) const { return other == p; }
    operator T*() const { return p; }
    T* operator->() const { return p; }

    void set(T* target) {
        if (p == target) return;

        T* prev = p;
        p = target;
        if (p != NULL)
            p->refCount++;

        if (prev != NULL) {
            prev->refCount--;
            if (prev->refCount <= 0)
                delete prev;
        }

    }
};

struct WeakRefHub
{
    // Points to the real object. The real object also has a pointer to this
    // object, and the real object will set 'obj' to null when it is deleted.
    void* obj;
    int refCount;

    WeakRefHub() : obj(NULL), refCount(0) {}
};

template <class T>
struct WeakRef
{
    RefcountedPtr<WeakRefHub> ptr;

    WeakRef& operator=(T const* other) { ptr = other->_weakRefOwner.hub; return *this; }

    operator T*() const { return get(); }
    T* operator*() const { return get(); }
    T* operator->() { return get(); }
    T* get() const { 
        if (ptr == NULL)
            return NULL;
        return (T*) ptr->obj;
    }
};

template <class T>
struct WeakRefOwner
{
    RefcountedPtr<WeakRefHub> hub;

    WeakRefOwner() { hub = new WeakRefHub(); }
    ~WeakRefOwner() { hub->obj = NULL; }

    void initialize(T* obj)
    {
        hub->obj = obj;
    }
};

} // namespace circa
