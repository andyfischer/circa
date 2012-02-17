
#include "zmq.h"

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include <string>

int main(int nargs, const char** args)
{
    if (nargs == 1) {
        printf("Missing arg\n");
        return 0;
    }

    void *context = zmq_init(1);

    if (strcmp(args[1], "listen") == 0) {
        printf("Starting listener\n");
        void *responder = zmq_socket(context, ZMQ_REP);
        if (zmq_bind(responder, "tcp://*:5555") == -1)
            printf("zmq_bind failed\n");

        while (1) {
            zmq_msg_t request;
            zmq_msg_init(&request);
            zmq_recv(responder, &request, 0);
            std::string msg;
            msg.assign((char*) zmq_msg_data(&request), zmq_msg_size(&request));
            printf("Received: %s\n", msg.c_str());
            zmq_msg_close(&request);

            sleep(1);

            zmq_msg_t reply;
            zmq_msg_init_size(&reply, 5);
            memcpy(zmq_msg_data(&reply), "World", 5);
            zmq_send(responder, &reply, 0);
            zmq_msg_close(&reply);
        }

        zmq_close(responder);



    } else {
        printf("Starting requester\n");

        void *socket = zmq_socket(context, ZMQ_REQ);
        printf("Connecting to %s\n", args[1]);
        
        if (zmq_connect(socket, args[1]) == -1)
            printf("zmq_connect failed\n");

        zmq_msg_t request;
        zmq_msg_init_size(&request, 5);
        memcpy(zmq_msg_data(&request), "Hello", 5);
        printf("Sending hello\n");
        zmq_send(socket, &request, 0);
        zmq_msg_close(&request);

        zmq_msg_t reply;
        zmq_msg_init(&reply);
        zmq_recv(socket, &reply, 0);

        std::string msg;
        msg.assign((char*) zmq_msg_data(&reply), zmq_msg_size(&reply));
        printf("Received: %s\n", msg.c_str());
        zmq_msg_close(&reply);

        zmq_close(socket);
    }
    zmq_term(context);
    return 0;
}
