// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <pthread.h>

#include "zmq.h"

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"

#include "circa/circa.h"
#include "circa/file.h"
#include "circa/cpp.h"

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

// ZMQ utility functions
void zmq_msg_init_from_str(zmq_msg_t* msg, const char* str)
{
    int len = strlen(str);
    zmq_msg_init_size(msg, len);
    memcpy(zmq_msg_data(msg), str, len);
}

void zmsg_to_cavalue(zmq_msg_t* msg, caValue* value)
{
    circa::Value str;
    circa_set_string_size(&str, (char*) zmq_msg_data(msg), zmq_msg_size(msg));
    circa_parse_string(circa_as_string(&str), value);
}

void cavalue_to_zmsg(caValue* value, zmq_msg_t* msg)
{
    circa::Value str;
    circa_to_string_repr(value, &str);
    zmq_msg_init_from_str(msg, circa_as_string(&str));
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
        zmsg_to_cavalue(&msg, circa_output(stack, 0));
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

    zmq_msg_t msg;
    cavalue_to_zmsg(circa_input(stack, 1), &msg);

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

    if (zmq_connect(requester->socket, addr) == -1) {
        printf("in create_requester, zmq_bind failed with: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_bind failed");
        delete requester;
        return;
    }

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, requester, RequesterRelease);
}

void zmq__Requester_send(caStack* stack)
{
    Requester* requester = (Requester*) circa_handle_get_object(circa_input(stack, 0));

    zmq_msg_t msg;
    cavalue_to_zmsg(circa_input(stack, 1), &msg);

    if (zmq_send(requester->socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_send failed");
    }
    zmq_msg_close(&msg);
}

void zmq__Requester_receive(caStack* stack)
{
    Requester* requester = (Requester*) circa_handle_get_object(circa_input(stack, 0));

    zmq_msg_t msg;
    zmq_msg_init(&msg);

    if (zmq_recv(requester->socket, &msg, 0) == -1) {
        printf("zmq_recv failed with error: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_recv failed");

    } else {
        zmsg_to_cavalue(&msg, circa_output(stack, 0));
    }
    zmq_msg_close(&msg);
}

void zmq__create_publisher(caStack* stack)
{
    int port = circa_int_input(stack, 0);

    void* socket = zmq_socket(g_context, ZMQ_PUB);

    char addr[50];
    sprintf(addr, "tcp://*:%d", port);
    if (zmq_bind(socket, addr) == -1) {
        printf("in create_publisher, zmq_bind failed with: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_bind failed");
        zmq_close(socket);
        return;
    }

    caValue* out = circa_create_default_output(stack, 0);
    circa_handle_set_object(out, socket, SocketRelease);
}

void zmq__Publisher_send(caStack* stack)
{
    void* socket = circa_handle_get_object(circa_input(stack, 0));

    zmq_msg_t msg;
    cavalue_to_zmsg(circa_input(stack, 1), &msg);

    if (zmq_send(socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
    }
    zmq_msg_close(&msg);
}

void zmq__create_subscriber(caStack* stack)
{
    const char* addr = circa_string_input(stack, 0);

    void* socket = zmq_socket(g_context, ZMQ_SUB);

    if (zmq_connect(socket, addr) == -1) {
        printf("in create_subscriber, zmq_connect failed with: %s\n", strerror(errno));
        circa_raise_error(stack, "zmq_connect failed");
        zmq_close(socket);
        return;
    }
    
    // Subscribe to all incoming messages (don't use ZMQ filtering)
    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);

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
        zmsg_to_cavalue(&msg, circa_output(stack, 0));
    }

    zmq_msg_close(&msg);
}

#if 0
namespace zmq_file_source {
    struct ZmqFileClient
    {
        // ZMQ socket for requests
        void* requesterSocket;

        // ZMQ socket for receiving pushed updates.
        void* subscriberSocket;
    };

    void file_source_update_file(caFileSource* source, caFileRecord* record);

    void zmq_init_file_source(const char* addr, caValue* error)
    {
        ZmqFileClient* client = new ZmqFileClient();

        // Install the Circa service
        caFileSource fileSource;

        // No openFile. If the file isn't listed in our manifest then we don't have it.
        fileSource.openFile = NULL;

        fileSource.updateFile = file_source_update_file;

        fileSource.name = circa_name("zmq:FileClient");

        circa_install_file_source(&fileSource);

        // Connect to responder
        client->requesterSocket = zmq_socket(g_context, ZMQ_REQ);

        if (zmq_connect(client->requesterSocket, addr) == -1) {
            circa_set_string(error, "zmq_bind failed");
            return;
        }

        // Request manifest
        zmq_msg_t manifest_request;
        zmq_msg_init_from_str(&manifest_request, "get_manifest");
        
        if (zmq_send(client->requesterSocket, &manifest_request, 0) == -1) {
            circa_set_string(error, "zmq_send failed");
            return;
        }

        zmq_msg_close(&manifest_request);

        // Receive manifest
        zmq_msg_t manifest_response;
        zmq_msg_init(&manifest_response);

        if (zmq_recv(client->requesterSocket, &manifest_response, 0) == -1) {
            circa_set_string(error, "zmq_recv failed");
            return;
        }

        // Copy to a null-terminated string
        caValue response;
        zmsg_to_cavalue(&response);

        // Unpack result
        if (!circa_is_list(&response)) {
            circa_set_string(error, "malformed response");
            return;
        }

        // response[0] is address of publisher
        caValue* publisher_addr = circa_list_get_index(&response, 0);

        // response[1] is list of served files
        caValue* file_list = circa_list_get_index(&response, 1);

        // Create a circa FileRecord for each file
        for (int i=0; i < circa_list_length(file_list); i++) {
            caValue* item = circa_list_get_index(file_index, i);
            circa_fetch_file_record(circa_as_string(item), fileSource.name);
        }

        // Connect subscriber socket
        client->subscriberSocket = zmq_socket(g_context, ZMQ_SUB);
        
        if (zmq_bind(client->subscriberSocket, circa_as_string(publisher_addr)) == -1) {
            circa_set_string(error, "zmq_bind (to publisher) failed, addr:");
            circa_string_append(error, publisher_addr);
            return;
        }

        // All done
    }

    // Read the next incoming message from client->subscriberSocket. Returns
    // true if there may be additional messages.
    bool read_next_feed_msg(ZmqFileClient* client)
    {
        zmq_msg_t zmsg;
        zmq_msg_init(&zmsg);

        if (zmq_recv(client->subscriberSocket, &zmsg, ZMQ_NOBLOCK) == -1) {

            if (errno == EAGAIN)
                return false;

            printf("in read_next_feed_msg, zmq_recv failed\n");
            return false;
        }

        caValue msg;
        zmsg_to_cavalue(&zmsg, &msg);

        // msg[0] is the event type
        caValue* eventType = circa_list_get_index(&msg, 0);

        if (circa_string_eq(eventType, "update")) {

            // msg[1] is the filename
            caValue* filename = circa_list_get_index(&msg, 1);

            // msg[2] is the file contents
            caValue* contents = circa_list_get_index(&msg, 2);

            caFileRecord* record = circa_get_file_record(circa_as_string(filename));

            if (record != NULL) {
                free(record->data);
                record->data = strdup(circa_as_string(contents));
                record->version++;
            }

        } else {
            printf("in read_next_feed_msg, unrecognized event type: %s",
                circa_to_string(eventType));
        }
        zmq_msg_close(&msg);
    }

    void file_source_update_file(caFileSource* source, caFileRecord* record)
    {
        ZmqFileClient* client = circa_get_pointer(&source->privateData);
        
        // Take this chance to read incoming feed data
        while (read_next_feed_msg(client));

        // Check if the data is available. If we don't have it yet, we'll need
        // to do a blocking read.
        if (record->data == NULL) {

            // Send a request for this file
            caValue requestStr;
            circa_set_string(&requestStr, "get_file ");
            circa_string_append(&requestStr, record->filename);

            zmq_msg_t zmsg;
            zmq_msg_init_from_str(&zmsg, circa_as_string(&requestStr));

            if (zmq_send(client->requesterSocket, &zmsg, 0) == -1) {
                printf("in file_source_update_file, zmq_send failed\n");
                return;
            }

            // Receive the file
            zmq_msg_close(&zmsg);

            zmq_msg_init(&zmsg);
            if (zmq_recv(client->requesterSocket, &zmsg, 0) == -1) {
                printf("in file_source_update_file, zmq_recv failed\n");
                return;
            }

            caValue reply;
            zmsg_to_cavalue(&zmsg, &reply);
            zmq_msg_close(&zmsg);

            record->data = strdup(circa_as_string(&reply));
        }
    }
}
#endif

// For static linking:
void zmq_install_functions(caBranch* branch)
{
    circa_install_function(branch, "zmq:create_responder", zmq__create_responder);
    circa_install_function(branch, "zmq:Responder.read", zmq__Responder_read);
    circa_install_function(branch, "zmq:Responder.reply", zmq__Responder_reply);
    circa_install_function(branch, "zmq:create_requester", zmq__create_requester);
    circa_install_function(branch, "zmq:Requester.send", zmq__Requester_send);
    circa_install_function(branch, "zmq:Requester.receive", zmq__Requester_receive);
    circa_install_function(branch, "zmq:create_publisher", zmq__create_publisher);
    circa_install_function(branch, "zmq:Publisher.send", zmq__Publisher_send);
    circa_install_function(branch, "zmq:create_subscriber", zmq__create_subscriber);
    circa_install_function(branch, "zmq:Subscriber.poll", zmq__Subscriber_poll);
}

} // extern "C"
