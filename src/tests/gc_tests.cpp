// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "types/handle.h"

namespace circa {
namespace gc_tests {

List g_recentlyDeleted;

struct Thing
{
    GCHeader gcHeader;

    std::string name;
    Thing* ref;

    Thing() : ref(NULL) {}
};

void thingListReferences(void* obj, GCReferenceList* refs)
{
    Thing* thing = (Thing*) obj;
    if (thing->ref != NULL)
        gc_ref_append(refs, &thing->ref->gcHeader);
}

void releaseThing(void* obj)
{
    Thing* thing = (Thing*) obj;
    set_string(g_recentlyDeleted.append(), thing->name.c_str());
    delete (Thing*) obj;
}

void start_test(Type* type)
{
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
    a->gcHeader.type = &type;

    // Register and collect, 'a' will get released.
    gc_register_new_object(&a->gcHeader);
    gc_collect();

    test_equals(&g_recentlyDeleted, "['test_simple']");
}

void test_simple_root()
{
    Type type;
    start_test(&type);

    Thing* a = new Thing();
    a->name = "test_simple_root";
    a->gcHeader.type = &type;
    a->gcHeader.root = true;

    // Register and collect, 'a' will remain because it's a root.
    gc_register_new_object(&a->gcHeader);
    gc_collect();

    test_equals(&g_recentlyDeleted, "[]");

    // Clean up the mess..
    a->gcHeader.root = false;
    gc_collect();
    test_equals(&g_recentlyDeleted, "['test_simple_root']");
}

void test_with_refs()
{
    Type type;
    start_test(&type);

    Thing* thing1 = new Thing();
    thing1->name = "thing_1";
    thing1->gcHeader.type = &type;
    thing1->gcHeader.root = true;
    gc_register_new_object(&thing1->gcHeader);

    Thing* thing2 = new Thing();
    thing2->name = "thing_2";
    thing2->gcHeader.type = &type;
    gc_register_new_object(&thing2->gcHeader);

    Thing* thing3 = new Thing();
    thing3->name = "thing_3";
    thing3->gcHeader.type = &type;
    gc_register_new_object(&thing3->gcHeader);

    Thing* thing4 = new Thing();
    thing4->name = "thing_4";
    thing4->gcHeader.type = &type;
    gc_register_new_object(&thing4->gcHeader);
    
    thing1->ref = thing2;
    thing2->ref = thing3;

    set_list(&g_recentlyDeleted, 0);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['thing_4']");

    thing2->ref = NULL;
    set_list(&g_recentlyDeleted, 0);
    gc_collect();
    test_equals(&g_recentlyDeleted, "['thing_3']");

    thing1->gcHeader.root = false;
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
    thing1->gcHeader.type = &type;
    thing1->gcHeader.root = true;
    gc_register_new_object(&thing1->gcHeader);

    Thing* thing2 = new Thing();
    thing2->name = "thing_2";
    thing2->gcHeader.type = &type;
    gc_register_new_object(&thing2->gcHeader);

    Thing* thing3 = new Thing();
    thing3->name = "thing_3";
    thing3->gcHeader.type = &type;
    gc_register_new_object(&thing3->gcHeader);

    Thing* thing4 = new Thing();
    thing4->name = "thing_4";
    thing4->gcHeader.type = &type;
    gc_register_new_object(&thing4->gcHeader);
    
    thing1->ref = thing2;
    thing2->ref = thing3;
    thing3->ref = thing4;

    gc_collect();
    test_equals(&g_recentlyDeleted, "[]");

    thing1->gcHeader.root = false;
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
