
// Outline:
//  Run a ZMQ listener
//  Listen for incoming connections
//  Handle commands for each connection
//  Commands:
//   Retrieve file
//
//  Also:
//   Watch files for changes (using circa?)
//   On file change, push new file to all listeners
//
// globals
//   list of all active connections
//
// threads
//   main thread
//     run circa script
//     poll connections


#include "circa/circa.h"
#include "zmq.h"

int main(int argc, char** argv)
{
    circa_initialize();

    return 0;
}
