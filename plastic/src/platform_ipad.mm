// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "app.h"
#include "gl_util.h"
#include "of_bindings.h"

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
    
void render()
{
    app::info("platform_ipad::render()");

    gl_clear_error();
    app::evaluate_main_script();

    const char* err = gl_check_error();
    if (err != NULL) {
        std::cerr << "Uncaught GL error (in render_frame): " << err << std::endl;
    }
}

} // namespace platform_ipad
