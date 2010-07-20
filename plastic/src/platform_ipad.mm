// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "ofEvents.h"
#include "ofUtils.h"

#include <circa.h>

#include "app.h"
#include "gl_util.h"
#include "of_bindings.h"

using namespace circa;

namespace platform_ipad {

void pl_storage_read_text_file(const char* filename,
    circa::storage::FileReceiveFunc func, void* context)
{
    NSString *nsFilename = [NSString stringWithCString:filename encoding:NSUTF8StringEncoding];
    NSString *path = [[NSBundle mainBundle] pathForResource:nsFilename ofType:nil];
    NSString *contents = [[NSString alloc] initWithContentsOfFile:path];
    func(context, [contents UTF8String]);
}

bool pl_storage_file_exists(const char* filename)
{
    NSFileManager *fileManager = [[NSFileManager alloc] init];
    NSString *nsFilename = [NSString stringWithCString:filename encoding:NSUTF8StringEncoding];
    NSString *path = [[NSBundle mainBundle] pathForResource:nsFilename ofType:nil];
    return [fileManager fileExistsAtPath:path isDirectory:NULL];
}

void initialize()
{
    // Install an iOS-compatible storage interface
    circa::storage::StorageInterface interface;
    interface.readTextFile = pl_storage_read_text_file;
    interface.writeTextFile = NULL;
    interface.getModifiedTime = NULL;
    interface.fileExists = pl_storage_file_exists;
    circa::storage::install_storage_interface(&interface);
    
    app::info("platform_ipad::initialize()");
    app::initialize();
    of_bindings::setup();
    app::load_user_script_filename("ipad_home.ca");
}

void update()
{
    app::update_time_from_elapsed_millis(ofGetElapsedTimeMillis());
}
    
void render()
{
    app::info("platform_ipad::render()");

    gl_clear_error();
    app::evaluate_main_script();

    const char* err = gl_check_error();
    if (err != NULL) {
        app::error(std::string("Uncaught GL error (in render_frame): ") + err);
    }

    List* incoming_events = List::checkCast(app::runtime_branch()["incoming_touch_events"]);
    incoming_events->clear();
}

void set_point(circa::TaggedValue* value, float x, float y)
{
    change_type(value, type_contents(circa::get_global("Point")));
    touch(value);
    set_float(value->getIndex(0), x);
    set_float(value->getIndex(1), y);
}

void touch_event_to_tagged_value(ofTouchEventArgs& event, circa::TaggedValue* eventType,
    circa::TaggedValue* value)
{
    change_type(value, type_contents(app::runtime_branch()["TouchEvent"]));
    List* list = List::checkCast(value);
    
    touch(list);
    set_int(list->get(0), event.id);
    set_int(list->get(1), event.time);
    copy(eventType, list->get(2));
    set_point(list->get(3), event.x, event.y);
    set_int(list->get(4), event.numTouches);
    set_point(list->get(5), event.x, event.y);
    set_float(list->get(6), event.angle);
    set_float(list->get(7), event.minoraxis);
    set_float(list->get(8), event.majoraxis);
    set_float(list->get(9), event.pressure);
    set_point(list->get(10), event.xspeed, event.yspeed);
    set_point(list->get(11), event.xaccel, event.yaccel);
}

void enqueue_touch_event(ofTouchEventArgs &touchEvent, TaggedValue* eventType)
{
    List* incoming_events = List::checkCast(app::runtime_branch()["incoming_touch_events"]);
    touch(incoming_events);
    
    touch_event_to_tagged_value(touchEvent, eventType, incoming_events->append());
}

void on_touch_down(ofTouchEventArgs &touch)
{
    enqueue_touch_event(touch, app::runtime_branch()["TouchEvent_down"]);
}

void on_touch_moved(ofTouchEventArgs &touch)
{
    enqueue_touch_event(touch, app::runtime_branch()["TouchEvent_moved"]);
}

void on_touch_up(ofTouchEventArgs &touch)
{
    enqueue_touch_event(touch, app::runtime_branch()["TouchEvent_up"]);
}
void on_touch_double_tap(ofTouchEventArgs &touch)
{
    enqueue_touch_event(touch, app::runtime_branch()["TouchEvent_doubleTap"]);
}

} // namespace platform_ipad
