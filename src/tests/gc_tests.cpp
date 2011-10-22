// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "object.h"
#include "types/handle.h"

namespace circa {
namespace gc_tests {

List g_recentlyDeleted;

struct Thing : CircaObject
{
    std::string name;
    Thing* ref;

    Thing() : ref(NULL) {}
};

void thingListReferences(void* obj, GCReferenceList* refs)
{
    Thing* thing = (Thing*) obj;
    if (thing->ref != NULL)
        gc_ref_append(refs, thing->ref);
}

void releaseThing(void* obj)
{
    Thing* thing = (Thing*) obj;
    set_string(g_recentlyDeleted.append(), thing->name.c_str());
    delete (Thing*) obj;
}

void start_test(Type* type)
{
    type->name = "Thing";
    type->gcListReferences = thingListReferences;
    type->gcRelease = releaseThing;

    set_list(&g_recentlyDeleted, 0);
}

void test_simple()
{
    Type type;
    start_test(&type);

    Thing* a = new Thing();
    a->name = "test_simple";

    // Register and collect, 'a' will get released.
    register_new_object(a, &type);
    gc_collect();

    test_equals(&g_recentlyDeleted, "['test_simple']");
}

void test_simple_root()
{
    Type type;
    start_test(&type);

    Thing* a = new Thing();
    a->name = "test_simple_root";

    // Register and collect, 'a' will remain because it's a root.
    register_new_object(a, &type);
    set_object_permanent(a, true);
    gc_collect();

    test_equals(&g_recentlyDeleted, "[]");

    // Clean up the mess..
    set_object_permanent(a, false);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['test_simple_root']");
}

void test_with_refs()
{
    Type type;
    start_test(&type);

    Thing* thing1 = new Thing();
    thing1->name = "thing_1";
    register_new_object(thing1, &type);
    set_object_permanent(thing1, true);

    Thing* thing2 = new Thing();
    thing2->name = "thing_2";
    register_new_object(thing2, &type);

    Thing* thing3 = new Thing();
    thing3->name = "thing_3";
    register_new_object(thing3, &type);

    Thing* thing4 = new Thing();
    thing4->name = "thing_4";
    register_new_object(thing4, &type);
    
    thing1->ref = thing2;
    thing2->ref = thing3;

    set_list(&g_recentlyDeleted, 0);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['thing_4']");

    thing2->ref = NULL;
    set_list(&g_recentlyDeleted, 0);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['thing_3']");

    set_object_permanent(thing1, false);
    set_list(&g_recentlyDeleted, 0);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['thing_1', 'thing_2']");
}

void test_with_refs2()
{
    Type type;
    start_test(&type);

    Thing* thing1 = new Thing();
    thing1->name = "thing_1";
    register_new_object(thing1, &type);
    set_object_permanent(thing1, true);

    Thing* thing2 = new Thing();
    thing2->name = "thing_2";
    register_new_object(thing2, &type);

    Thing* thing3 = new Thing();
    thing3->name = "thing_3";
    register_new_object(thing3, &type);

    Thing* thing4 = new Thing();
    thing4->name = "thing_4";
    register_new_object(thing4, &type);
    
    thing1->ref = thing2;
    thing2->ref = thing3;
    thing3->ref = thing4;

    gc_collect();
    test_equals(&g_recentlyDeleted, "[]");

    set_object_permanent(thing1, false);
    set_list(&g_recentlyDeleted, 0);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['thing_1', 'thing_2', 'thing_3', 'thing_4']");
}

void register_tests()
{
    REGISTER_TEST_CASE(gc_tests::test_simple);
    REGISTER_TEST_CASE(gc_tests::test_simple_root);
    REGISTER_TEST_CASE(gc_tests::test_with_refs);
    REGISTER_TEST_CASE(gc_tests::test_with_refs2);
}

} // namesapce gc_tests
} // namespace circa
