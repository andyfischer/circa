
// Outline
//  Run a ZMQ listener
//  Listen for incoming connections
//  Handle commands for each connection
//  Commands:
//   Retrieve file
//
//  Also
//   Watch files for changes (using circa)
//   On file change, push new file to all listeners
//
// globals
//   list of all active connections
//
// threads
//   main thread
//     run circa script
//     poll connections
//

#include "circa/circa.h"
#include "zmq.h"

void on_file_changed(caStack* stack)
{
    printf("TODO: handle file changed %s\n", circa_string_input(stack, 0));
}

int main(int argc, char** argv)
{
    circa_initialize();
    caBranch* module = circa_load_module_from_file(circa_name("script_server"), "script_server.ca");
    circa_install_function(module, "on_file_changed", on_file_changed);

    return 0;
}
