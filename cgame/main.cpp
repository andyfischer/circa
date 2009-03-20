#include "SDL/SDL.h"
#include "SDL_gfxPrimitives.h"

#include <string>

#include "circa.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;
const float HIGHLIGHT_MIN_DIST = 5.0;

bool KEY_DOWN[SDLK_LAST];
std::set<int> KEY_JUST_PRESSED;

int MOUSE_X = 0;
int MOUSE_Y = 0;
bool INFLUENCE_LIST_ENABLED = false;

SDL_Surface* SCREEN = NULL;
circa::Ref DRAGGABLE_FUNC = NULL;
circa::Ref HIGHLIGHT = NULL;
circa::Branch SCRIPT_MAIN;
bool CONTINUE_MAIN_LOOP = true;

circa::RefList INFLUENCE_LIST;

bool drag_in_progress = false;
circa::Ref drag_target;

int previous_mouse_x = 0;
int previous_mouse_y = 0;

int mouse_movement_x = 0;
int mouse_movement_y = 0;

void initialize_keydown()
{
    for (int i=0; i < SDLK_LAST; i++) {
        KEY_DOWN[i] = false;
    }
}

void key_down(circa::Term* caller)
{
    int i = caller->input(0)->asInt();
    caller->asBool() = KEY_DOWN[i];
}

void key_pressed(circa::Term* caller)
{
    int i = caller->input(0)->asInt();
    caller->asBool() = KEY_JUST_PRESSED.find(i) != KEY_JUST_PRESSED.end();
}

void fill_rectangle(circa::Term* caller)
{
    boxColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        (unsigned int) as_int(caller->input(4)));
}

void line_to(circa::Term* caller)
{
    aalineColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        as_int(caller->input(4)));
}

namespace sdl_hosted {

void _aalineColor(circa::Term* caller)
{
    aalineColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        as_int(caller->input(4)));
}

} // namespace sdl_hosted

void update_highlight()
{
    float closestDist = 0;
    HIGHLIGHT = NULL;

    for (circa::CodeIterator it(&SCRIPT_MAIN); !it.finished(); it.advance()) {
        if (it->function == DRAGGABLE_FUNC) {
            float x = as_float(it->input(0));
            float y = as_float(it->input(1));
            float dist_x = MOUSE_X - x;
            float dist_y = MOUSE_Y - y;
            float dist = sqrt(dist_x*dist_x + dist_y*dist_y);

            if (dist > HIGHLIGHT_MIN_DIST)
                continue;

            if ((dist < closestDist) || HIGHLIGHT == NULL) {
                HIGHLIGHT = *it;
                closestDist = dist;
            }
        }
    }
}

void update_influence_list()
{
    if (HIGHLIGHT == NULL) {
        INFLUENCE_LIST.clear();
        return;
    }

    INFLUENCE_LIST = circa::get_influencing_values(HIGHLIGHT);

    for (int i=0; i < INFLUENCE_LIST.count(); i++) {

        circa::Term* term = INFLUENCE_LIST[i];

        if (term->name == "")
            INFLUENCE_LIST[i] = NULL;

        if (term->boolPropertyOptional("dont_train", false))
            INFLUENCE_LIST[i] = NULL;
    }

    INFLUENCE_LIST.removeNulls();

    circa::sort_by_name(INFLUENCE_LIST);
}

void draw_influence_list()
{
    if (!INFLUENCE_LIST_ENABLED) return;
    if (HIGHLIGHT == NULL) return;

    // background rect
    int rect_width = 8 * 30;
    int item_height = 9;
    int rect_height = item_height * INFLUENCE_LIST.count();
    int rect_top = SCREEN_HEIGHT - rect_height;
    boxColor(SCREEN, 0, rect_top, rect_width, SCREEN_HEIGHT, 0xbbbbffff);

    for (int i=0; i < INFLUENCE_LIST.count(); i++) {
        circa::Term* term = INFLUENCE_LIST[i];
        int y = rect_top + i*item_height;

        // draw a box if this term is selected for training
        if (is_trainable(term))
            boxColor(SCREEN, 0, y, rect_width, y + 9, 0xffffaaff);

        std::string text = term->name + " = " + term->toString();

        stringColor(SCREEN, 0, y, text.c_str(), 0xff);
    }
}

void draw_highlight()
{
    if (HIGHLIGHT == NULL)
        return;

    float x = as_float(HIGHLIGHT->input(0));
    float y = as_float(HIGHLIGHT->input(1));

    circleColor(SCREEN, (int) x, (int) y, (int) HIGHLIGHT_MIN_DIST, 0);
}

void toggle_influencer(int index)
{
    if (index >= INFLUENCE_LIST.count()) return;
    circa::Term* term = INFLUENCE_LIST[index];

    term->boolProperty("trainable") = !is_trainable(term);
    update_derived_trainables(SCRIPT_MAIN);
}

void handle_key_press(int key)
{
    switch (key) {

    case SDLK_ESCAPE: CONTINUE_MAIN_LOOP = false; break;

    case SDLK_i:
        update_highlight();
        if (HIGHLIGHT != NULL)
            INFLUENCE_LIST_ENABLED = true;
        else
            INFLUENCE_LIST_ENABLED = false;
        update_influence_list();
        break;

    case SDLK_1: toggle_influencer(0); break;
    case SDLK_2: toggle_influencer(1); break;
    case SDLK_3: toggle_influencer(2); break;
    case SDLK_4: toggle_influencer(3); break;
    case SDLK_5: toggle_influencer(4); break;
    case SDLK_6: toggle_influencer(5); break;
    case SDLK_7: toggle_influencer(6); break;
    case SDLK_8: toggle_influencer(7); break;
    case SDLK_9: toggle_influencer(8); break;
    case SDLK_0: toggle_influencer(9); break;
    }
}

void handle_dragging()
{
    if (!drag_in_progress)
        return;

    circa::Term* subjectX = drag_target->input(0);
    circa::Term* subjectY = drag_target->input(1);

    circa::Branch tempBranch;

    circa::Term* desiredX = circa::float_value(tempBranch, MOUSE_X);
    circa::Term* desiredY = circa::float_value(tempBranch, MOUSE_Y);

    circa::generate_training(tempBranch, subjectX, desiredX);
    circa::generate_training(tempBranch, subjectY, desiredY);

    evaluate_branch(tempBranch);

    print_raw_branch(tempBranch, std::cout);
}

void post_script_load(circa::Branch& script)
{
    update_derived_trainables(script);
}

int main( int argc, char* args[] )
{
    initialize_keydown();

    circa::initialize();

    circa::KERNEL->eval("type Point { float x, float y }");

    circa::import_function(*circa::KERNEL, key_down, "key_down(int) -> bool");
    circa::import_function(*circa::KERNEL, key_pressed, "key_pressed(int) -> bool");
    circa::int_value(*circa::KERNEL, SDLK_UP, "KEY_UP");
    circa::int_value(*circa::KERNEL, SDLK_DOWN, "KEY_DOWN");
    circa::int_value(*circa::KERNEL, SDLK_LEFT, "KEY_LEFT");
    circa::int_value(*circa::KERNEL, SDLK_RIGHT, "KEY_RIGHT");
    circa::int_value(*circa::KERNEL, SDLK_SPACE, "KEY_SPACE");

    (*circa::KERNEL)["KEY_UP"]->boolProperty("dont_train") = true;
    (*circa::KERNEL)["KEY_DOWN"]->boolProperty("dont_train") = true;
    (*circa::KERNEL)["KEY_LEFT"]->boolProperty("dont_train") = true;
    (*circa::KERNEL)["KEY_RIGHT"]->boolProperty("dont_train") = true;
    (*circa::KERNEL)["KEY_SPACE"]->boolProperty("dont_train") = true;
    (*circa::KERNEL)["PI"]->boolProperty("dont_train") = true;

    circa::import_function(*circa::KERNEL, line_to, "line_to(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, fill_rectangle,
            "fill_rectangle(float,float,float,float,int)");

    circa::import_function(*circa::KERNEL, sdl_hosted::_aalineColor,
            "sdl::lineColor(float,float,float,float,int)");

    DRAGGABLE_FUNC = circa::create_empty_function(*circa::KERNEL, "draggable(Point)");

    // Set up the screen
    SCREEN = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    // If there was an error in setting up the screen
    if( SCREEN == NULL )
        return 1;

    std::string filename;
    if (argc > 1)
        filename = args[1];
    else
        filename = "cgame/main.ca";

    std::cout << "loading file: " << filename << std::endl;

    circa::evaluate_file(SCRIPT_MAIN, filename);
    post_script_load(SCRIPT_MAIN);

    // Initialize all SDL subsystems
    if( SDL_Init(SDL_INIT_EVERYTHING) == -1)
        return 1;

    // Set the window caption
    SDL_WM_SetCaption("Circa Game Editor", NULL );

    // Main loop
    while (CONTINUE_MAIN_LOOP) {
        KEY_JUST_PRESSED.clear();

        SDL_Event event;
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
            CONTINUE_MAIN_LOOP = false;

        if (event.type == SDL_MOUSEMOTION) {
            SCRIPT_MAIN["mouse_x"]->asFloat() = event.motion.x;
            SCRIPT_MAIN["mouse_y"]->asFloat() = event.motion.y;
            MOUSE_X = event.motion.x;
            MOUSE_Y = event.motion.y;
            mouse_movement_x = event.motion.x;
            mouse_movement_y = event.motion.y;

            if (!INFLUENCE_LIST_ENABLED)
                update_highlight();


        } else if (event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_ALT)) {
            // Alt key
            switch(event.key.keysym.sym) {
            case SDLK_4:
                std::cout << "Script contents:" << std::endl;
                print_raw_branch(SCRIPT_MAIN, std::cout);
                break;
            case SDLK_5:
                circa::reload_branch_from_file(SCRIPT_MAIN);
                post_script_load(SCRIPT_MAIN);
                std::cout << "Script reloaded" << std::endl;
                break;
            }
        } else if (event.type == SDL_KEYDOWN) {

            if (!KEY_DOWN[event.key.keysym.sym]) {
                KEY_JUST_PRESSED.insert(event.key.keysym.sym);
                handle_key_press(event.key.keysym.sym);
            }

            KEY_DOWN[event.key.keysym.sym] = true;

        } else if (event.type == SDL_KEYUP) {
            KEY_DOWN[event.key.keysym.sym] = false;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (HIGHLIGHT != NULL) {
                drag_in_progress = true;
                drag_target = HIGHLIGHT;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            drag_in_progress = false;
            drag_target = NULL;
        }

        try {
            handle_dragging();
            SCRIPT_MAIN.eval();
            draw_highlight();
            draw_influence_list();

        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            return 0;
        }

        // Update the screen
        if( SDL_Flip(SCREEN) == -1 )
            return 1;

        SDL_Delay( 10 );
    }

    // Quit SDL
    SDL_Quit();

    return 0;
}
