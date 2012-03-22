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
    Responder* responder = (Responder*) circ_as_pointer(value);
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
    Requester* requester = (Requester*) circ_as_pointer(value);
    zmq_close(requester->socket);
    delete requester;
}

void SocketRelease(caValue* value)
{
    void* socket = (Requester*) circ_as_pointer(value);
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
    circ_set_string_size(&str, (char*) zmq_msg_data(msg), zmq_msg_size(msg));
    circ_parse_string(circ_as_string(&str), value);
}

void cavalue_to_zmsg(caValue* value, zmq_msg_t* msg)
{
    circa::Value str;
    circ_to_string_repr(value, &str);
    zmq_msg_init_from_str(msg, circ_as_string(&str));
}

void zmq__create_responder(caStack* stack)
{
    Responder* responder = new Responder();
    responder->socket = zmq_socket(g_context, ZMQ_REP);

    int port = circ_int_input(stack, 0);
    char addr[50];
    sprintf(addr, "tcp://*:%d", port);

    if (zmq_bind(responder->socket, addr) == -1) {
        printf("zmq_bind failed with error: %s\n", strerror(errno));
        circ_raise_error(stack, "zmq_bind failed\n");
        delete responder;
        return;
    }

    caValue* out = circ_create_default_output(stack, 0);
    circ_handle_set_object(out, responder, ResponderRelease);
}

void zmq__Responder_read(caStack* stack)
{
    Responder* responder = (Responder*) circ_as_pointer(
        circ_handle_get_value(circ_input(stack, 0)));

    // Don't allow a read if we never replied to the last message
    if (responder->expectingReply) {
        circ_raise_error(stack, "read() called but we never sent a reply() to previous message");
        return;
    }

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_recv(responder->socket, &msg, ZMQ_NOBLOCK) == -1) {
        if (errno == EAGAIN) {
            // No message available right now; this is normal.
            circ_set_null(circ_output(stack, 0));
        } else {
            printf("zmq_recv failed with error: %s\n", strerror(errno));
        }
    } else {

        // Successfully received a message
        zmsg_to_cavalue(&msg, circ_output(stack, 0));
        responder->expectingReply = true;
    }

    zmq_msg_close(&msg);
}

void zmq__Responder_reply(caStack* stack)
{
    Responder* responder = (Responder*)  circ_as_pointer(
        circ_handle_get_value(circ_input(stack, 0)));

    if (!responder->expectingReply) {
        circ_raise_error(stack, "reply() called but there's no message to reply to");
        return;
    }

    responder->expectingReply = false;

    zmq_msg_t msg;
    cavalue_to_zmsg(circ_input(stack, 1), &msg);

    if (zmq_send(responder->socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
    }
    zmq_msg_close(&msg);
}

void zmq__create_requester(caStack* stack)
{
    Requester* requester = new Requester();
    requester->socket = zmq_socket(g_context, ZMQ_REQ);
    
    const char* addr = circ_string_input(stack, 0);

    if (zmq_connect(requester->socket, addr) == -1) {
        printf("in create_requester, zmq_bind failed with: %s\n", strerror(errno));
        circ_raise_error(stack, "zmq_bind failed");
        delete requester;
        return;
    }

    caValue* out = circ_create_default_output(stack, 0);
    circ_handle_set_object(out, requester, RequesterRelease);
}

void zmq__Requester_send(caStack* stack)
{
    Requester* requester = (Requester*) circ_handle_get_object(circ_input(stack, 0));

    zmq_msg_t msg;
    cavalue_to_zmsg(circ_input(stack, 1), &msg);

    if (zmq_send(requester->socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
        circ_raise_error(stack, "zmq_send failed");
    }
    zmq_msg_close(&msg);
}

void zmq__Requester_receive(caStack* stack)
{
    Requester* requester = (Requester*) circ_handle_get_object(circ_input(stack, 0));

    zmq_msg_t msg;
    zmq_msg_init(&msg);

    if (zmq_recv(requester->socket, &msg, 0) == -1) {
        printf("zmq_recv failed with error: %s\n", strerror(errno));
        circ_raise_error(stack, "zmq_recv failed");

    } else {
        zmsg_to_cavalue(&msg, circ_output(stack, 0));
    }
    zmq_msg_close(&msg);
}

void zmq__create_publisher(caStack* stack)
{
    int port = circ_int_input(stack, 0);

    void* socket = zmq_socket(g_context, ZMQ_PUB);

    char addr[50];
    sprintf(addr, "tcp://*:%d", port);
    if (zmq_bind(socket, addr) == -1) {
        printf("in create_publisher, zmq_bind failed with: %s\n", strerror(errno));
        circ_raise_error(stack, "zmq_bind failed");
        zmq_close(socket);
        return;
    }

    caValue* out = circ_create_default_output(stack, 0);
    circ_handle_set_object(out, socket, SocketRelease);
}

void zmq__Publisher_send(caStack* stack)
{
    void* socket = circ_handle_get_object(circ_input(stack, 0));

    zmq_msg_t msg;
    cavalue_to_zmsg(circ_input(stack, 1), &msg);

    if (zmq_send(socket, &msg, 0) == -1) {
        printf("zmq_send failed with error: %s\n", strerror(errno));
    }
    zmq_msg_close(&msg);
}

void zmq__create_subscriber(caStack* stack)
{
    const char* addr = circ_string_input(stack, 0);

    void* socket = zmq_socket(g_context, ZMQ_SUB);

    if (zmq_connect(socket, addr) == -1) {
        printf("in create_subscriber, zmq_connect failed with: %s\n", strerror(errno));
        circ_raise_error(stack, "zmq_connect failed");
        zmq_close(socket);
        return;
    }
    
    // Subscribe to all incoming messages (don't use ZMQ filtering)
    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);

    caValue* out = circ_create_default_output(stack, 0);
    circ_handle_set_object(out, socket, SocketRelease);
}

void zmq__Subscriber_poll(caStack* stack)
{
    void* socket = circ_handle_get_object(circ_input(stack, 0));

    zmq_msg_t msg;
    zmq_msg_init(&msg);

    if (zmq_recv(socket, &msg, ZMQ_NOBLOCK) == -1) {
        if (errno == EAGAIN) {
            // No message available right now; this is normal.
            circ_set_null(circ_output(stack, 0));
        } else {
            printf("zmq_recv failed with error: %s\n", strerror(errno));
        }
    } else {

        // Successfully received a message
        zmsg_to_cavalue(&msg, circ_output(stack, 0));
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

        fileSource.name = circ_name("zmq:FileClient");

        circ_install_file_source(&fileSource);

        // Connect to responder
        client->requesterSocket = zmq_socket(g_context, ZMQ_REQ);

        if (zmq_connect(client->requesterSocket, addr) == -1) {
            circ_set_string(error, "zmq_bind failed");
            return;
        }

        // Request manifest
        zmq_msg_t manifest_request;
        zmq_msg_init_from_str(&manifest_request, "get_manifest");
        
        if (zmq_send(client->requesterSocket, &manifest_request, 0) == -1) {
            circ_set_string(error, "zmq_send failed");
            return;
        }

        zmq_msg_close(&manifest_request);

        // Receive manifest
        zmq_msg_t manifest_response;
        zmq_msg_init(&manifest_response);

        if (zmq_recv(client->requesterSocket, &manifest_response, 0) == -1) {
            circ_set_string(error, "zmq_recv failed");
            return;
        }

        // Copy to a null-terminated string
        caValue response;
        zmsg_to_cavalue(&response);

        // Unpack result
        if (!circ_is_list(&response)) {
            circ_set_string(error, "malformed response");
            return;
        }

        // response[0] is address of publisher
        caValue* publisher_addr = circ_list_get(&response, 0);

        // response[1] is list of served files
        caValue* file_list = circ_list_get(&response, 1);

        // Create a circa FileRecord for each file
        for (int i=0; i < circ_list_length(file_list); i++) {
            caValue* item = circ_list_get(file_index, i);
            circ_fetch_file_record(circ_as_string(item), fileSource.name);
        }

        // Connect subscriber socket
        client->subscriberSocket = zmq_socket(g_context, ZMQ_SUB);
        
        if (zmq_bind(client->subscriberSocket, circ_as_string(publisher_addr)) == -1) {
            circ_set_string(error, "zmq_bind (to publisher) failed, addr:");
            circ_string_append(error, publisher_addr);
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
        caValue* eventType = circ_list_get(&msg, 0);

        if (circ_string_eq(eventType, "update")) {

            // msg[1] is the filename
            caValue* filename = circ_list_get(&msg, 1);

            // msg[2] is the file contents
            caValue* contents = circ_list_get(&msg, 2);

            caFileRecord* record = circ_get_file_record(circ_as_string(filename));

            if (record != NULL) {
                free(record->data);
                record->data = strdup(circ_as_string(contents));
                record->version++;
            }

        } else {
            printf("in read_next_feed_msg, unrecognized event type: %s",
                circ_to_string(eventType));
        }
        zmq_msg_close(&msg);
    }

    void file_source_update_file(caFileSource* source, caFileRecord* record)
    {
        ZmqFileClient* client = circ_get_pointer(&source->privateData);
        
        // Take this chance to read incoming feed data
        while (read_next_feed_msg(client));

        // Check if the data is available. If we don't have it yet, we'll need
        // to do a blocking read.
        if (record->data == NULL) {

            // Send a request for this file
            caValue requestStr;
            circ_set_string(&requestStr, "get_file ");
            circ_string_append(&requestStr, record->filename);

            zmq_msg_t zmsg;
            zmq_msg_init_from_str(&zmsg, circ_as_string(&requestStr));

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

            record->data = strdup(circ_as_string(&reply));
        }
    }
}
#endif

// For static linking:
void zmq_install_functions(caBranch* branch)
{
    circ_install_function(branch, "zmq:create_responder", zmq__create_responder);
    circ_install_function(branch, "zmq:Responder.read", zmq__Responder_read);
    circ_install_function(branch, "zmq:Responder.reply", zmq__Responder_reply);
    circ_install_function(branch, "zmq:create_requester", zmq__create_requester);
    circ_install_function(branch, "zmq:Requester.send", zmq__Requester_send);
    circ_install_function(branch, "zmq:Requester.receive", zmq__Requester_receive);
    circ_install_function(branch, "zmq:create_publisher", zmq__create_publisher);
    circ_install_function(branch, "zmq:Publisher.send", zmq__Publisher_send);
    circ_install_function(branch, "zmq:create_subscriber", zmq__create_subscriber);
    circ_install_function(branch, "zmq:Subscriber.poll", zmq__Subscriber_poll);
}

} // extern "C"
