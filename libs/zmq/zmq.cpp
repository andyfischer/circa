// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <pthread.h>

#include "zmq.h"

#include "stdio.h"
#include "unistd.h"
#include "string.h"

#include "circa/internal/for_hosted_funcs.h"

using namespace circa;

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

void ResponderRelease(TValue* value)
{
    Responder* responder = (Responder*) as_opaque_pointer(value);
    zmq_close(responder->socket);
    delete responder;
}

struct Requester
{
    void* socket;

    Requester()
    {
        socket = NULL;
    }
};

void RequesterRelease(TValue* value)
{
    Requester* requester = (Requester*) as_opaque_pointer(value);
    zmq_close(requester->socket);
    delete requester;
}

EXPORT CA_FUNCTION(zmq__create_responder)
{
    Responder* responder = new Responder();
    responder->socket = zmq_socket(g_context, ZMQ_REP);

    int port = INT_INPUT(0);
    char addr[50];
    sprintf(addr, "tcp://*:%d", port);

    if (zmq_bind(responder->socket, addr) == -1) {
        printf("zmq_bind failed with error: %s\n", strerror(errno));
        RAISE_ERROR("zmq_bind failed\n");
        delete responder;
        return;
    }

    set_handle_value_opaque_pointer(OUTPUT, CALLER->type, responder, ResponderRelease);
}

EXPORT CA_FUNCTION(zmq__Responder_read)
{
    Responder* responder = (Responder*) get_handle_value_opaque_pointer(INPUT(0));

    // Don't allow a read if we never replied to the last message
    if (responder->expectingReply) {
        RAISE_ERROR("read() called but we never sent a reply() to previous message");
        return;
    }

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_recv(responder->socket, &msg, ZMQ_NOBLOCK) == -1) {
        if (errno == EAGAIN) {
            // No message available right now; this is normal.
            set_null(OUTPUT);
        } else {
            printf("zmq_recv failed with error: %s\n", strerror(errno));
        }
    } else {

        // Successfully received a message
        set_string(OUTPUT, (char*) zmq_msg_data(&msg), zmq_msg_size(&msg));
        responder->expectingReply = true;
    }

    zmq_msg_close(&msg);
}

EXPORT CA_FUNCTION(zmq__Responder_reply)
{
    Responder* responder = (Responder*) get_handle_value_opaque_pointer(INPUT(0));

    if (!responder->expectingReply) {
        RAISE_ERROR("reply() called but there's no message to reply to");
        return;
    }

    responder->expectingReply = false;

    const char* msg_str = STRING_INPUT(1);

    zmq_msg_t msg;
    int len = strlen(msg_str);
    zmq_msg_init_size(&msg, len);
    memcpy(zmq_msg_data(&msg), msg_str, len);

    if (zmq_send(responder->socket, &msg, 0) == -1) {
        printf("zmq_reply failed with error: %s\n", strerror(errno));
    }
    zmq_msg_close(&msg);
}

EXPORT CA_FUNCTION(zmq__create_requester)
{
    Requester* requester = new Requester();
    requester->socket = zmq_socket(g_context, ZMQ_REQ);
    
    const char* addr = STRING_INPUT(0);

    if (zmq_bind(requester->socket, addr) == -1) {
        printf("zmq_bind failed with error: %s\n", strerror(errno));
        RAISE_ERROR("zmq_bind failed\n");
        delete requester;
        return;
    }

    set_handle_value_opaque_pointer(OUTPUT, CALLER->type, requester, RequesterRelease);
}

EXPORT CA_FUNCTION(zmq__Requester_read)
{
}

EXPORT void on_load(Branch* branch)
{
    setup_handle_type(get_declared_type(branch, "zmq:Requester"));
    setup_handle_type(get_declared_type(branch, "zmq:Responder"));
}
