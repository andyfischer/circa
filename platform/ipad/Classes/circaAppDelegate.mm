//
//  circaAppDelegate.m
//  circa
//
//  Created by Paul Hodge on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#include <circa.h>
#include <importing_macros.h>

#import "circaAppDelegate.h"
#import "EAGLView.h"

#import "app.h"
#import "platform_ipad.h"

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

CA_FUNCTION(draw_text)
{
    const char* str = STRING_INPUT(0);
    circa::TaggedValue* loc = INPUT(1);
    float locX = loc->getIndex(0)->toFloat();
    float locY = loc->getIndex(1)->toFloat();
    
    NSString *nsStr = [NSString stringWithCString:str encoding:NSUTF8StringEncoding];
    
    [nsStr drawAtPoint:CGPointMake(locX,locY)
        withFont:[UIFont systemFontOfSize:[UIFont systemFontSize]]];
}

void install_storage_interface()
{
    circa::storage::StorageInterface interface;
    interface.readTextFile = pl_storage_read_text_file;
    interface.writeTextFile = NULL;
    interface.getModifiedTime = NULL;
    interface.fileExists = pl_storage_file_exists;
    circa::storage::install_storage_interface(&interface);
}
    

@implementation circaAppDelegate

@synthesize window;
@synthesize glView;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions   
{
    install_storage_interface();
    platform_ipad::initialize();
    
    circa::install_function(app::runtime_branch()["draw_text"], draw_text);
    
    [glView startAnimation];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    [glView stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    [glView stopAnimation];
}

- (void)dealloc
{
    [window release];
    [glView release];

    [super dealloc];
}

@end
