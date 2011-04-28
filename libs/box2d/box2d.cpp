
#include "circa.h"

b2World *g_world;

float c_timeStep = 1/60.0;
int c_velocityIterations = 6;
int c_positionIterations = 2;

int c_maxBodies = 100;

CA_FUNCTION(setup_world)
{
    if (g_world != NULL)
        g_world = new b2World(FLOAT_INPUT(0), FLOAT_INPUT(1));

    g_world->Step(c_timeStep, c_velocityIterations, c_positionIterations);
    g_world->ClearForces();
}

CA_FUNCTION(body)
{
}


void setup(circa::Branch& kernel)
{
}
