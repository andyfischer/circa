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

int c_velocityIterations = 6;
int c_positionIterations = 2;

// Store a list of pointers to active b2Body objects.
const int c_maxBodies = 10000;

int g_nextFreeBody = 0;
b2Body* g_bodies[c_maxBodies] = {NULL,};

// Scale between screen & world coordinates. Box2d says that it likes objects
// to have a size between 0.1 and 10 units.
float screen_to_world(float screen) { return screen / 10.0; }
float world_to_screen(float world) { return world * 10.0; }

// Circa angles are in the range of 0..1
float radians_to_unit_angles(float radians) { return radians / M_PI; }
float unit_angles_to_radians(float unit) { return unit * M_PI; }

// The Circa type g_bodyHandleType is used to hold on to b2Body objects. When
// the Circa object is destroyed then releaseBodyHandle will be called.
Type g_bodyHandleType;

void release_body_handle(int handle)
{
    std::cout << "releasing body: " << handle << std::endl;
    if (g_world == NULL)
        return;
    if (g_bodies[handle] == NULL)
        return;
    g_world->DestroyBody(g_bodies[handle]);
    g_bodies[handle] = NULL;
}

b2Body* get_body_from_handle(TaggedValue* value)
{
    if (value->value_type != &g_bodyHandleType)
        return NULL;
    return g_bodies[simple_handle_t::get(value)];
}

bool is_valid_body_handle(TaggedValue* value)
{
    return value->value_type == &g_bodyHandleType
        && get_body_from_handle(value) != NULL;
}

int find_free_body_index()
{
    int first = g_nextFreeBody;

    while(1) {

        if (g_nextFreeBody >= c_maxBodies)
            g_nextFreeBody = 0;

        if (g_bodies[g_nextFreeBody] == NULL)
            break;

        g_nextFreeBody++;

        if (g_nextFreeBody == first)
            internal_error("reached maximum body count");
    }

    return g_nextFreeBody++;
}

int assign_body_handle(TaggedValue* value, b2Body* body)
{
    int index = find_free_body_index();
    g_bodies[index] = body;
    set_null(value);
    simple_handle_t::set(&g_bodyHandleType, value, index);
    return index;
}

CA_FUNCTION(step)
{
    initialize_world();

    float duration = FLOAT_INPUT(0);

    g_world->Step(duration, c_velocityIterations, c_positionIterations);
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
    //   number initialRotation
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
    float initialRotation = as_float(INPUT(2));
    List& properties = *List::checkCast(INPUT(3));
    bool isDynamic = as_bool(properties[0]);
    Point& size = *Point::checkCast(properties[1]);
    float density = to_float(properties[2]);
    float friction = to_float(properties[3]);
    float restitution = to_float(properties[4]);
    bool propertiesChanged = as_bool(INPUT(4));

    if (!is_valid_body_handle(handle)) {


        if (size.getX() <= 0.0 || size.getY() <= 0.0)
            return error_occurred(CONTEXT, CALLER, "Size must be non-zero");

        // Create a b2Body
        b2BodyDef bodyDef;

        if (isDynamic)
            bodyDef.type = b2_dynamicBody;

        bodyDef.position.Set(
            screen_to_world(initialPosition.getX()),
            screen_to_world(initialPosition.getY()));

        bodyDef.angle = unit_angles_to_radians(initialRotation);

        b2Body* body = g_world->CreateBody(&bodyDef);

        int handle_index = assign_body_handle(handle, body);

        std::cout << " allocating body: " << handle_index << std::endl;

        ca_assert(is_valid_body_handle(handle));

        propertiesChanged = true;
    }

    b2Body* body = get_body_from_handle(handle);

#if 0
    // If it's not dynamic then update the position & rotation.
    if (!isDynamic) {
        b2Vec2 pos(
            screen_to_world(initialPosition.getX()),
            screen_to_world(initialPosition.getY()));

        body->SetTransform(pos, unit_angles_to_radians(initialRotation));
    }
#endif

    if (propertiesChanged) {

        // remove any old fixtures
        while (body->GetFixtureList())
            body->DestroyFixture(body->GetFixtureList());

        // Assign the shape
        b2PolygonShape polygonShape;
        polygonShape.SetAsBox(
                screen_to_world(size.getX()),
                screen_to_world(size.getY()));

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &polygonShape;
        fixtureDef.density = density;
        fixtureDef.friction = friction;
        fixtureDef.restitution = restitution;

        body->CreateFixture(&fixtureDef);
    }

    copy(handle, OUTPUT);
}

CA_FUNCTION(get_body_points)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;
    
    List& points = *List::cast(OUTPUT, 0);

    // Copy vertex locations
    for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
        b2Shape* shape = fixture->GetShape();

        if (shape->GetType() == b2Shape::e_polygon) {
            b2PolygonShape *poly = (b2PolygonShape*) shape;

            int count = poly->GetVertexCount();

            for (int i = 0; i < count; i++) {
                b2Vec2 vec = body->GetWorldPoint(poly->GetVertex(i));
                Point* p = Point::cast(points.append());
                p->set(
                    world_to_screen(vec.x),
                    world_to_screen(vec.y));
            }
        }
    }
}

CA_FUNCTION(get_body_position)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    b2Vec2 vec = body->GetPosition();

    Point& output = *Point::cast(OUTPUT);
    output.set(
        world_to_screen(vec.x),
        world_to_screen(vec.y));
}

CA_FUNCTION(set_body_position)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    Point& loc = *Point::checkCast(INPUT(1));

    b2Vec2 vec(
        screen_to_world(loc.getX()),
        screen_to_world(loc.getY()));

    body->SetTransform(vec, body->GetAngle());
}

CA_FUNCTION(body_contains_point)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    Point& input = *Point::checkCast(INPUT(1));

    b2Vec2 p(
        screen_to_world(input.getX()),
        screen_to_world(input.getY()));

    b2Fixture* fixture = body->GetFixtureList();

    while (fixture) {
        if (fixture->TestPoint(p)) {
            set_bool(OUTPUT, true);
            return;
        }
        fixture = fixture->GetNext();
    }

    set_bool(OUTPUT, false);
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
    install_function(ns["get_body_points"], get_body_points);
    install_function(ns["get_body_position"], get_body_position);
    install_function(ns["set_body_position"], set_body_position);
    install_function(ns["body_contains_point"], body_contains_point);
}

} // box2d_support
