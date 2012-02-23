// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <pthread.h>

#include "zmq.h"

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"

#include "circa/circa.h"

extern "C" {

void *g_context = zmq_init(1);

struct Responder
{
    void* socket;
    bool expectingReply;

    Responder()
    {
        socket = NULL;
        expectingReply = false;
    }
};

void ResponderRelease(caValue* value)
{
    Responder* responder = (Responder*) circa_as_pointer(value);
    zmq_close(responder->socket);
    delete responder;
}

struct Requester
{
    void* socket;

    Requester() : socket(NULL) {}
};

void RequesterRelease(caValue* value)
{
    Requester* requester = (Requester*) circa_as_pointer(value);
    zmq_close(requester->socket);
    delete requester;
}

void SocketRelease(caValue* value)
{
    void* socket = (Requester*) circa_as_pointer(value);
    zmq_close(socket);
}

void zmq__create_responder(caStack* stack)
{
    Responder* responder = new Responder();
    responder->socket = zmq_socket(g_context, ZMQ_REP);

    int port = circa_int_input(stack, 0);
    char addr[50];
    sprintf(addr, "tcp://*:%d", port);

    if (zmq_bind(responder->socket, addr) == -1) {
        printf("zmq_bind failed with error: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_bind failed\n");
        delete responder;
        return;
    }

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, responder, ResponderRelease);
}

void zmq__Responder_read(caStack* stack)
{
    Responder* responder = (Responder*) circa_as_pointer(
        circa_handle_get_value(circa_input(stack, 0)));

    // Don't allow a read if we never replied to the last message
    if (responder->expectingReply) {
        circa_raise_error(stack, "read() called but we never sent a reply() to previous message");
        return;
    }

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_recv(responder->socket, &msg, ZMQ_NOBLOCK) == -1) {
        if (errno == EAGAIN) {
            // No message available right now; this is normal.
            circa_set_null(circa_output(stack, 0));
        } else {
            printf("zmq_recv failed with error: %s\n", strerror(errno));
        }
    } else {

        // Successfully received a message
        circa_set_string_size(circa_output(stack, 0), (char*) zmq_msg_data(&msg), zmq_msg_size(&msg));
        responder->expectingReply = true;
    }

    zmq_msg_close(&msg);
}

void zmq__Responder_reply(caStack* stack)
{
    Responder* responder = (Responder*)  circa_as_pointer(
        circa_handle_get_value(circa_input(stack, 0)));

    if (!responder->expectingReply) {
        circa_raise_error(stack, "reply() called but there's no message to reply to");
        return;
    }

    responder->expectingReply = false;

    const char* msg_str = circa_string_input(stack, 1);

    zmq_msg_t msg;
    int len = strlen(msg_str);
    zmq_msg_init_size(&msg, len);
    memcpy(zmq_msg_data(&msg), msg_str, len);

    if (zmq_send(responder->socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
    }
    zmq_msg_close(&msg);
}

void zmq__create_requester(caStack* stack)
{
    Requester* requester = new Requester();
    requester->socket = zmq_socket(g_context, ZMQ_REQ);
    
    const char* addr = circa_string_input(stack, 0);

    if (zmq_bind(requester->socket, addr) == -1) {
        printf("in create_requester, zmq_bind failed with: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_bind failed");
        delete requester;
        return;
    }

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, requester, RequesterRelease);
}

void zmq__Requester_read(caStack* stack)
{
    Requester* requester = (Requester*) circa_handle_get_object(circa_input(stack, 0));
    // TODO
}

void zmq__create_publisher(caStack* stack)
{
    int port = circa_int_input(stack, 0);

    void* socket = zmq_socket(g_context, ZMQ_PUB);

    char addr[50];
    sprintf(addr, "tcp://localhost:%d", port);
    if (zmq_bind(socket, addr) == -1) {
        printf("in create_publisher, zmq_bind failed with: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_bind failed");
        zmq_close(socket);
    }

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, socket, SocketRelease);
}

void zmq__Publisher_send(caStack* stack)
{
    void* socket = circa_handle_get_object(circa_input(stack, 0));
    char* msg_str = circa_to_string(circa_input(stack, 1));

    zmq_msg_t msg;
    int len = strlen(msg_str);
    zmq_msg_init_size(&msg, len);
    memcpy(zmq_msg_data(&msg), msg_str, len);

    if (zmq_send(socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
    }
    zmq_msg_close(&msg);

    free(msg_str);
}

void zmq__create_subscriber(caStack* stack)
{
    const char* addr = circa_string_input(stack, 0);

    void* socket = zmq_socket(g_context, ZMQ_PUB);

    if (zmq_bind(socket, addr) == -1) {
        printf("in create_subscriber, zmq_bind failed with: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_bind failed");
        zmq_close(socket);
    }

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, socket, SocketRelease);
}

void zmq__Subscriber_poll(caStack* stack)
{
    void* socket = circa_handle_get_object(circa_input(stack, 0));

    zmq_msg_t msg;
    zmq_msg_init(&msg);

    if (zmq_recv(socket, &msg, ZMQ_NOBLOCK) == -1) {
        if (errno == EAGAIN) {
            // No message available right now; this is normal.
            circa_set_null(circa_output(stack, 0));
        } else {
            printf("zmq_recv failed with error: %s\n", strerror(errno));
        }
    } else {

        // Successfully received a message
        circa_set_string_size(circa_output(stack, 0), (char*) zmq_msg_data(&msg), zmq_msg_size(&msg));
    }

    zmq_msg_close(&msg);
}

void on_load(caBranch* branch)
{
    // setup_handle_type(get_declared_type(branch, "zmq:Requester"));
    // setup_handle_type(get_declared_type(branch, "zmq:Responder"));
}

} // extern "C"
