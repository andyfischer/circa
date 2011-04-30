
#include "circa.h"

using namespace circa;

b2World *g_world;

float c_timeStep = 1/60.0;
int c_velocityIterations = 6;
int c_positionIterations = 2;

int c_maxBodies = 100;



float screen_to_world(float screen) { return screen / 100.0; }
float world_to_screen(float world) { return world * 100.0; }

CA_FUNCTION(step)
{
    if (g_world != NULL)
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

CA_FUNCTION(box)
{
    // Inputs:
    //   State id
    //   Initial position
    //   Box shape
    //   bool shapeChanged
    //   Density
    //   Friction
    // Outputs:
    //   current position & angle
    //   state info
    
    // If we need to initialize:
    //   Create the b2bodyDef (with position)
    //   Create the b2body using the bodyDef
    //   Create the b2PolygonShape using input
    //   Set the density & friction
    //   Create a fixture attaching the b2PolygonShape to the b2Body
}

void setup(Branch& kernel)
{
    Branch& ns = kernel["box2d"]->nestedContents;

    ns["step"] = step;
    ns["gravity"] = gravity;
}
