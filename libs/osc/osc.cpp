// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <pthread.h>

#include "lo/lo.h"

#include "circa.h"
#include "types/handle.h"

using namespace circa;

struct ServerContext
{
    lo_server_thread server_thread;

    // incomingMessages is guarded by a mutex.
    pthread_mutex_t mutex;
    circa::List incomingMessages;

    ServerContext() : server_thread(NULL)
    {
        pthread_mutex_init(&mutex, NULL);
    }
    ~ServerContext() {
        pthread_mutex_destroy(&mutex);
        if (server_thread != NULL)
            lo_server_thread_stop(server_thread);
    }
};

struct Address
{
    lo_address address;
};

Type* g_serverContext_t;
Type* g_address_t;

extern "C" {

void error_callback(int num, const char *m, const char *path)
{
    printf("lo reported error: %d, %s, %s\n", num, m, path);
}

int incoming_message_callback(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data)
{
    ServerContext* context = (ServerContext*) user_data;

    List message;
    message.resize(argc + 1);
    set_string(message[0], path);

    for (int i=0; i < argc; i++) {
        char type = types[i];
        TValue* val = message[i + 1];
        if (type == 'i')
            set_int(val, argv[i]->i);
        else if (type == 'f')
            set_float(val, argv[i]->f);
        else if (type == 's')
            set_string(val, &argv[i]->s);
        else {
            printf("osc incoming: couldn't handle type code: %c\n", type);
            return 1;
        }
    }

    int ret = 0;

    ret = pthread_mutex_lock(&context->mutex);
    swap(&message, context->incomingMessages.append());
    ret = pthread_mutex_unlock(&context->mutex);

    return 1;
}

CA_FUNCTION(osc__create_server_thread)
{
    int port = INT_INPUT(0);
    char portStr[15];
    sprintf(portStr, "%d", port);

    ServerContext* context = handle_t::create<ServerContext>(OUTPUT, g_serverContext_t);
    // printf("opened server at %s, context = %p\n", portStr, context);

    context->server_thread = lo_server_thread_new(portStr, error_callback);

    if (context->server_thread == NULL) {
        RAISE_ERROR("lo_server_thread_new failed");
        set_null(OUTPUT);
        return;
    }

    lo_server_thread_add_method(context->server_thread, NULL, NULL, incoming_message_callback,
            (void*) context);
    lo_server_thread_start(context->server_thread);
}

CA_FUNCTION(osc__read_from_server)
{
    ServerContext* context = (ServerContext*) handle_t::get_ptr(INPUT(0));

    List incoming;

    pthread_mutex_lock(&context->mutex);
    swap(&incoming, &context->incomingMessages);
    pthread_mutex_unlock(&context->mutex);

    swap(&incoming, OUTPUT);
}

CA_FUNCTION(osc__address)
{
    Address* address = handle_t::create<Address>(OUTPUT, g_address_t);

    int port = INT_INPUT(1);
    char portStr[20];
    sprintf(portStr, "%d", port);

    address->address = lo_address_new(STRING_INPUT(0), portStr);
}

CA_FUNCTION(osc__send)
{
    Address* address = (Address*) handle_t::get_ptr(INPUT(0));

    const char* destination = STRING_INPUT(1);

    const int c_maxArguments = 15;

    if ((NUM_INPUTS + 2) > c_maxArguments)
        return RAISE_ERROR("too many arguments");

    lo_message message = lo_message_new();

    bool failed = false;

    for (int i=2; i < NUM_INPUTS; i++) {
        TValue* val = INPUT(i);

        if (is_int(val))
            lo_message_add(message, "i", as_int(val));
        else if (is_float(val))
            lo_message_add(message, "f", as_float(val));
        else if (is_string(val))
            lo_message_add(message, "s", as_cstring(val));
        else {
            std::stringstream error;
            error << "Don't know how to send value " << val->toString()
                << " (with type " << val->value_type->name << ")";
            RAISE_ERROR(error.str());
            failed = true;
            break;
        }
    }

    if (!failed) {
        int result = lo_send_message(address->address, destination, message);

        if (result == -1)
            RAISE_ERROR("lo_send_message returned -1");
    }

    lo_message_free(message);
}

void on_load(Branch* branch)
{
    g_serverContext_t = get_declared_type(branch, "osc:ServerContext");
    g_address_t = get_declared_type(branch, "osc:Address");

    handle_t::setup_type<ServerContext>(g_serverContext_t);
    handle_t::setup_type<Address>(g_address_t);
}

} // extern "C"

