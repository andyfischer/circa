// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "Box2D/Box2D.h"

#include "circa.h"
#include "types/simple_handle.h"

using namespace circa;

namespace box2d_support {

b2World *g_world;

float c_timeStep = 1/60.0;
int c_velocityIterations = 6;
int c_positionIterations = 2;

// Store a list of pointers to active b2Body objects.
const int c_maxBodies = 100;

int g_nextFreeBody = 0;
b2Body* g_bodies[c_maxBodies] = {NULL,};

// Scale between screen & world coordinates. Box2d says that it likes objects
// to have a size between 0.1 and 10 units.
float screen_to_world(float screen) { return screen / 100.0; }
float world_to_screen(float world) { return world * 100.0; }

// The Circa type g_bodyHandleType is used to hold on to b2Body objects. When
// the Circa object is destroyed then releaseBodyHandle will be called.

Type g_bodyHandleType;

void release_body_handle(int handle)
{
    // TODO: call destructor
    g_bodies[handle] = NULL;
}

b2Body* get_body_from_handle(TaggedValue* value)
{
    return g_bodies[simple_handle_t::get(value)];
}

bool is_valid_body_handle(TaggedValue* value)
{
    return value->value_type == &g_bodyHandleType
        && get_body_from_handle(value) != NULL;
}

int find_free_body_index()
{
    while(1) {

        if (g_nextFreeBody >= c_maxBodies)
            g_nextFreeBody = 0;

        if (g_bodies[g_nextFreeBody] == NULL)
            break;

        g_nextFreeBody++;
    }

    return g_nextFreeBody++;
}

void assign_body_handle(TaggedValue* value, b2Body* body)
{
    int index = find_free_body_index();
    g_bodies[index] = body;
    set_null(value);
    simple_handle_t::set(&g_bodyHandleType, value, index);
}

CA_FUNCTION(step)
{
    if (g_world == NULL)
        g_world = new b2World(b2Vec2(), true);

    g_world->Step(c_timeStep, c_velocityIterations, c_positionIterations);
    g_world->ClearForces();
}

CA_FUNCTION(gravity)
{
    if (g_world == NULL)
        return;

    List& list = *List::checkCast(INPUT(0));
    b2Vec2 gravity(as_float(list[0]), as_float(list[1]));

    g_world->SetGravity(gravity);
}

CA_FUNCTION(body)
{
    // Inputs:
    //   State id
    //   Point initialPosition
    //   bool dynamic
    //   Point shape
    // Outputs:
    //   current position & angle
    //   state info
    
    // If we need to initialize:
    //   Create the b2bodyDef (with position)
    //   Create the b2Body using the bodyDef
    //   Create the b2PolygonShape using input
    //   Set the density & friction
    //   Create a fixture attaching the b2PolygonShape to the b2Body

    TaggedValue* handle = INPUT(0);
    Point& initialPosition = *Point::checkCast(INPUT(1));
    bool isDynamic = BOOL_INPUT(2);
    TaggedValue* shape = INPUT(3);

    if (!is_valid_body_handle(handle)) {
        // Create a b2Body

        b2BodyDef bodyDef;

        if (isDynamic)
            bodyDef.type = b2_dynamicBody;

        bodyDef.position.Set(initialPosition.getX(),
            initialPosition.getY());

        b2Body* body = g_world->CreateBody(&bodyDef);

        // Assign the shape
        b2PolygonShape polygonShape;
        // TODO

        assign_body_handle(handle, body);
    }

    b2Body* body = get_body_from_handle(handle);
}

void setup(Branch& kernel)
{
    simple_handle_t::setup_type(&g_bodyHandleType);
    g_bodyHandleType.name = "Box2d::BodyHandle";
    set_opaque_pointer(&g_bodyHandleType.parameter, (void*) release_body_handle);

    Branch& ns = kernel["box2d"]->nestedContents;

    install_function(ns["step"], step);
    install_function(ns["gravity"], gravity);
}

} // box2d_support
