// Copyright 2008 Andrew Fischer

#include "SDL.h"
#include "SDL_gfxPrimitives.h"

extern SDL_Surface* SCREEN;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

namespace sdl_wrapper {

void box(circa::Term* caller)
{
    boxColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        (unsigned int) as_int(caller->input(4)));
}

void line(circa::Term* caller)
{
    aalineColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        as_int(caller->input(4)));
}

void circle(circa::Term* caller)
{
    aacircleColor(SCREEN,
                  as_float(caller->input(0)),
                  as_float(caller->input(1)),
                  as_float(caller->input(2)),
                  as_int(caller->input(3)));
}

void filledcircle(circa::Term* caller)
{
    filledCircleColor(SCREEN,
                  as_float(caller->input(0)),
                  as_float(caller->input(1)),
                  as_float(caller->input(2)),
                  as_int(caller->input(3)));
}

void background(circa::Term* caller)
{
    boxColor(SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
            as_int(caller->input(0)));
}

void shape(circa::Term* caller)
{
    static Sint16 vx[500];
    static Sint16 vy[500];

    circa::Branch& list = circa::as_branch(caller->input(0));
    for (int i=0; i < list.numTerms(); i++) {
        vx[i] = circa::to_float(list[i]->field(0));
        vy[i] = circa::to_float(list[i]->field(1));
    }
    aapolygonColor(SCREEN, vx, vy, list.numTerms(), caller->input(1)->asInt());
    filledPolygonColor(SCREEN, vx, vy, list.numTerms(), caller->input(1)->asInt());
}

void drawText(circa::Term* caller)
{
    stringColor(SCREEN, caller->input(0)->asFloat(), caller->input(1)->asFloat(),
            caller->input(2)->asString().c_str(), caller->input(3)->asInt());
}

void register_functions(circa::Branch& branch)
{
    circa::import_function(branch, box, "box(float,float,float,float,int)");
    circa::import_function(branch, line, "line(float,float,float,float,int)");
    circa::import_function(branch, circle, "circle(float,float,float,int)");
    circa::import_function(branch, filledcircle, "fill_circle(float,float,float,int)");
    circa::import_function(branch, background, "background(int)");
    circa::import_function(branch, shape, "shape(List,int)");
    circa::import_function(branch, drawText, "drawText(float,float,string,int)");
}

} // namespace sdl_wrapper
