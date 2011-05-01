// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "Box2D/Box2D.h"

#include "circa.h"
#include "types/simple_handle.h"

using namespace circa;

namespace box2d_support {

b2World *g_world = NULL;

void initialize_world()
{
    if (g_world == NULL)
        g_world = new b2World(b2Vec2(), true);
}

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
    if (g_world == NULL)
        return;
    if (g_bodies[handle] == NULL)
        return;
    g_world->DestroyBody(g_bodies[handle]);
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
    initialize_world();

    g_world->Step(c_timeStep, c_velocityIterations, c_positionIterations);
    g_world->ClearForces();
}

CA_FUNCTION(gravity)
{
    initialize_world();

    Point& gravity = *Point::checkCast(INPUT(0));
    g_world->SetGravity(b2Vec2(gravity.getX(), gravity.getY()));
}

CA_FUNCTION(body)
{
    // Inputs:
    //   State id
    //   Point initialPosition
    //   List properties
    // Outputs:
    //   current position & angle
    //   state info
    
    // If we need to initialize:
    //   Create the b2bodyDef (with position)
    //   Create the b2Body using the bodyDef
    //   Create the b2PolygonShape using input
    //   Set the density & friction
    //   Create a fixture attaching the b2PolygonShape to the b2Body

    initialize_world();

    TaggedValue* handle = INPUT(0);
    Point& initialPosition = *Point::checkCast(INPUT(1));
    List& properties = *List::checkCast(INPUT(2));
    bool isDynamic = as_bool(properties[0]);
    Point& size = *Point::checkCast(properties[1]);
    float density = to_float(properties[2]);
    float friction = to_float(properties[3]);

    if (!is_valid_body_handle(handle)) {

        if (size.getX() <= 0.0 || size.getY() <= 0.0)
            return error_occurred(CONTEXT, CALLER, "Size must be non-zero");

        // Create a b2Body
        b2BodyDef bodyDef;

        if (isDynamic)
            bodyDef.type = b2_dynamicBody;

        bodyDef.position.Set(
                initialPosition.getX(),
            initialPosition.getY());

        b2Body* body = g_world->CreateBody(&bodyDef);

        // Assign the shape
        b2PolygonShape polygonShape;
        polygonShape.SetAsBox(size.getX(), size.getY());

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &polygonShape;
        fixtureDef.density = density;
        fixtureDef.friction = friction;

        body->CreateFixture(&fixtureDef);

        assign_body_handle(handle, body);

        ca_assert(is_valid_body_handle(handle));
    }

    b2Body* body = get_body_from_handle(handle);

    //b2Vec2 position = body->GetPosition();
    //float32 angle = body->GetAngle();

    // Write output as a list, with [handle, boxPoints]
    List& output = *List::cast(OUTPUT, 2);
    List& points = *List::cast(output[1], 0);

    // Copy vertex locations
    for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
        b2Shape* shape = fixture->GetShape();

        if (shape->GetType() == b2Shape::e_polygon) {
            b2PolygonShape *poly = (b2PolygonShape*) shape;

            int count = poly->GetVertexCount();

            for (int i = 0; i < count; i++) {
                b2Vec2 vec = body->GetWorldPoint(poly->GetVertex(i));
                Point* p = Point::cast(points.append());
                p->set(vec.x, vec.y);
            }
        }
    }

    copy(handle, output[0]);
}

void setup(Branch& kernel)
{
    simple_handle_t::setup_type(&g_bodyHandleType);
    g_bodyHandleType.name = "Box2d::BodyHandle";
    set_opaque_pointer(&g_bodyHandleType.parameter, (void*) release_body_handle);

    Branch& ns = kernel["box2d"]->nestedContents;

    install_function(ns["step"], step);
    install_function(ns["gravity"], gravity);
    install_function(ns["body_int"], body);
}

} // box2d_support
