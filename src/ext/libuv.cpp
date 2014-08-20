// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

/*
 
 https://github.com/joyent/libuv/blob/master/test/echo-server.c

 */
#include "common_headers.h"

#include <uv.h>
#include <http-parser/http_parser.h>

#include "debug.h"
#include "libuv.h"
#include "stack.h"
#include "string_type.h"
#include "world.h"

namespace circa {

static void on_close(uv_handle_t* peer);
static uv_buf_t alloc_buffer(uv_handle_t* handle, size_t suggested_size);
static void after_write(uv_write_t* req, int status);
static void after_shutdown(uv_shutdown_t* req, int status);
static void on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
static void buf_resize(uv_buf_t* buf, size_t newSize);
static void server_on_connect(uv_stream_t *uv_server, int status);
static void http_on_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t buf);
static void http_client_on_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t buf);
static int http_on_message_begin(http_parser* parser);
static int http_on_url(http_parser* parser, const char* field, size_t len);
static int http_on_status(http_parser* parser, const char* field, size_t len);
static int http_on_header_field(http_parser* parser, const char* field, size_t len);
static int http_on_header_value(http_parser* parser, const char* value, size_t len);
static int http_on_body(http_parser* parser, const char* value, size_t len);
static int http_on_headers_complete(http_parser* parser);
static int http_on_message_complete(http_parser* parser);

struct LibuvWorld {
    uv_loop_t* uv_loop;
};

enum ServerType { TCP, WEBSOCK };

enum ConnectionState {
    TCP_STATE,
    WEBSOCK_NEGOTIATE_STATE,
    WEBSOCK_DUPLEX_STATE
};

struct Server {
    uv_tcp_t uv_tcp;
    Value connections;

    ServerType serverType;

    http_parser_settings parser_settings;
};

struct Connection {
    uv_tcp_t uv_tcp;

    Value incomingStr;
    Value incomingMsgs;

    http_parser parser;

    Value httpPendingHeaderField;
    Value httpPendingHeaderValue;
    Value httpHeaders;

    ConnectionState state;
    Server* server;

    Connection() {
        server = NULL;
        circa_set_string(&incomingStr, "");
        circa_set_list(&incomingMsgs, 0);
    }
};

void ServerRelease(void* ptr)
{
}

void ConnectionRelease(void* ptr)
{
    // Connection* connection = (Connection*) ptr;
    //uv_close((uv_handle_t*) &connection->uv_connect.handle, NULL);
    // delete connection;
}

static uv_loop_t* get_uv_loop(caWorld* world)
{
    return world->libuvWorld->uv_loop;
}

static uv_buf_t alloc_buffer(uv_handle_t* handle,
                       size_t suggested_size) {

    return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

// Send 'msg' to the stream. 'msg' must be a string value. This function takes ownership
// of 'msg'.
static void circa_uv_write(uv_stream_t* stream, Value* msg, bool addNull)
{
    uv_write_t *wr = (uv_write_t*) malloc(sizeof *wr);
    wr->data = msg;

    uv_buf_t outgoing_buf;
    outgoing_buf.base = (char*) circa_string(msg);
    outgoing_buf.len = circa_string_length(msg);
    if (addNull)
        outgoing_buf.len++;

    if (uv_write(wr, stream, &outgoing_buf, 1, after_write)) {
        printf("uv_write failed\n");
    }
}

static void after_write(uv_write_t* req, int status) {

    Value* msg = (Value*) req->data;
    circa_dealloc_value(msg);
    req->data = NULL;

    if (status == 0)
        return;

    printf("uv_write error: %s\n", uv_err_name(uv_last_error(req->handle->loop)));

    if (status == UV_ECANCELED)
        return;
#if 0
    uv_close((uv_handle_t*)req->handle, on_close);
#endif
}

void make_server(caStack* stack)
{
    Value* ip = circa_input(stack, 0);
    Value* port = circa_input(stack, 1);
    Value* type = circa_input(stack, 2);

    Server* server = new Server();

    if (circa_string_equals(type, ":tcp")) {
        server->serverType = TCP;
    } else if (circa_string_equals(type, ":websock")) {
        server->serverType = WEBSOCK;
        memset(&server->parser_settings, 0, sizeof(server->parser_settings));
        server->parser_settings.on_message_begin = http_on_message_begin;
        server->parser_settings.on_url = http_on_url;
        server->parser_settings.on_status = http_on_status;
        server->parser_settings.on_header_field = http_on_header_field;
        server->parser_settings.on_header_value = http_on_header_value;
        server->parser_settings.on_headers_complete = http_on_headers_complete;
        server->parser_settings.on_body = http_on_body;
        server->parser_settings.on_message_complete = http_on_message_complete;

    } else {
        Value msg;
        circa_set_string(&msg, "Unrecognized server type: ");
        circa_string_append_val(&msg, type);
        circa_output_error_val(stack, &msg);
        delete server;
        return;
    }

    uv_loop_t* loop = get_uv_loop(stack->world);
    circa_set_list(&server->connections, 0);

    uv_tcp_init(stack->world->libuvWorld->uv_loop, &server->uv_tcp);

    struct sockaddr_in bind_addr = uv_ip4_addr(circa_string(ip), circa_int(port));

    uv_tcp_bind(&server->uv_tcp, bind_addr);
    int err = uv_listen((uv_stream_t*) &server->uv_tcp, 128, server_on_connect);
    server->uv_tcp.data = server;

    if (err) {
        Value msg;
        circa_set_string(&msg, "Listen error: ");
        circa_string_append(&msg, uv_err_name(uv_last_error(loop)));
        circa_output_error_val(stack, &msg);
        return;
    }

    Value* out = circa_set_default_output(stack, 0);
    circa_set_native_ptr(circa_index(out, 0), server, ServerRelease);
}

static void server_on_connect(uv_stream_t *uv_server, int status)
{
    Server* server = (Server*) uv_server->data;

    if (status == -1) {
        printf("server_on_connect failed\n");
        return;
    }

    Connection* connection = new Connection();
    connection->server = server;
    uv_tcp_init(uv_server->loop, &connection->uv_tcp);
    connection->uv_tcp.data = connection;

    switch (connection->server->serverType) {
    case WEBSOCK:
        http_parser_init(&connection->parser, HTTP_REQUEST);
        circa_set_map(&connection->httpHeaders);
        connection->parser.data = connection;
        connection->state = WEBSOCK_NEGOTIATE_STATE;
        break;
    case TCP:
        connection->state = TCP_STATE;
        break;
    }

    if (uv_accept(uv_server, (uv_stream_t*) &connection->uv_tcp) != 0)
        internal_error("uv_accept error\n");

    if (uv_read_start((uv_stream_t*) &connection->uv_tcp, alloc_buffer, on_read) != 0)
        internal_error("uv_read_start error\n");

    Value* wrapped = circa_append(&server->connections);
    circa_set_list(wrapped, 1);
    circa_set_native_ptr(circa_index(wrapped, 0), connection, ConnectionRelease);
    
    printf("on new connection\n");
}

static void buf_resize(uv_buf_t* buf, size_t newSize)
{
    buf->base = (char*) realloc(buf->base, newSize);
    buf->len = newSize;
}

static void buf_append(uv_buf_t* buf, uv_buf_t suffix)
{
    size_t oldSize = buf->len;
    size_t newSize = buf->len + suffix.len;
    buf_resize(buf, newSize);
    memcpy(buf->base + oldSize, suffix.base, suffix.len);
}

static void try_parse(Value* str, Value* msgList) 
{
    int msgStart = 0;

    for (int i=0; i < circa_string_length(str); i++) {
        if (circa_string(str)[i] == 0) {
            if (i > msgStart) {
                Value* msg = circa_append(msgList);
                circa_parse_string_len(circa_string(str), i - msgStart, msg);
            }
            msgStart = i + 1;
        }
    }

    if (msgStart > 0)
        string_slice(str, msgStart, -1);
}

static void http_write_upgrade_response(uv_stream_t* stream)
{
    Value* msg = circa_alloc_value();

    circa_set_string(msg, "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: ");
    
    circa_string_append(msg, "(responseKey)");
    circa_string_append(msg, "\r\n\r\n");

    circa_uv_write(stream, msg, false);
}

static void on_read(uv_stream_t* stream,
                       ssize_t nread,
                       uv_buf_t buf)
{
    printf("on_read: <<%.*s\n>>", (int) nread, buf.base);

    if (nread < 0) {
        if (buf.base)
            free(buf.base);

        printf("uv_shutdown\n");
        uv_shutdown_t* req = (uv_shutdown_t*) malloc(sizeof *req);
        uv_shutdown(req, stream, after_shutdown);

        return;
    }

    if (nread == 0) {
        /* Everything OK, but nothing read. */
        free(buf.base);
        return;
    }

    if (stream->data == NULL)
        internal_error("stream->data is null in on_read");

    Connection* connection = (Connection*) stream->data;
    if (connection->server != NULL && connection->server->serverType == WEBSOCK) {

        http_parser* parser = &connection->parser;

        printf("parsing as http\n");

        int parsed = http_parser_execute(parser,
            &connection->server->parser_settings,
            buf.base,
            nread);

        if (parser->upgrade) {
            connection->state = WEBSOCK_DUPLEX_STATE;
            return;
        }

        if (HTTP_PARSER_ERRNO(parser) != HPE_OK) { 

            printf("http parse error: [%s] %s\n",
                http_errno_name(HTTP_PARSER_ERRNO(parser)),
                http_errno_description(HTTP_PARSER_ERRNO(parser))
            );
            // handle parse error 
            return; 
        } 

        if (parsed < nread) {
            printf("TODO: Handle second message?\n");
        }

    } else {

        circa_string_append_len(&connection->incomingStr, buf.base, nread);
        try_parse(&connection->incomingStr, &connection->incomingMsgs);
    }

    free(buf.base);
}

static void on_close(uv_handle_t* peer) {
    free(peer);
}

static void after_shutdown(uv_shutdown_t* req, int status) {
    uv_close((uv_handle_t*)req->handle, on_close);
    free(req);
}

static int http_on_message_begin(http_parser* parser) {
    printf("http_on_message_begin\n");
    return 0;
}

static int http_on_url(http_parser* parser, const char* field, size_t len) {
    printf("http_on_url\n");
    return 0;
}

static int http_on_status(http_parser* parser, const char* field, size_t len) {
    printf("http_on_status\n");
    return 0;
}

static void http_flush_finished_header(Connection* connection)
{
    if (!circa_is_null(&connection->httpPendingHeaderField)) {
        circa_move(&connection->httpPendingHeaderValue,
            circa_map_insert_move(&connection->httpHeaders, &connection->httpPendingHeaderField));
    }
}

static int http_on_header_field(http_parser* parser, const char* field, size_t len) {
    Connection* connection = (Connection*) parser->data;
    if (!circa_is_null(&connection->httpPendingHeaderValue))
        http_flush_finished_header(connection);
    circa_string_append_len(&connection->httpPendingHeaderField, field, len);
    return 0;
}

static int http_on_header_value(http_parser* parser, const char* value, size_t len) {
    Connection* connection = (Connection*) parser->data;
    circa_string_append_len(&connection->httpPendingHeaderValue, value, len);
    return 0;
}

static int http_on_headers_complete(http_parser* parser) {
    Connection* connection = (Connection*) parser->data;
    http_flush_finished_header(connection);
    return 0;
}

static int http_on_body(http_parser* parser, const char* value, size_t len) {
    printf("http_on_body\n");
    return 0;
}

static int http_on_message_complete(http_parser* parser) {
    printf("http_on_message_complete\n");
    return 0;
}

void Server__connections(caStack* stack)
{
    Server* server = (Server*) circa_native_ptr(circa_index(circa_input(stack, 0), 0));
    circa_copy(&server->connections, circa_output(stack, 0));
}

void client_on_connect(uv_connect_t *req, int status)
{
    if (status == -1) {
        printf("on connect failed\n");
        return;
    }

    Connection* connection = (Connection*) req->data;

    if (uv_read_start((uv_stream_t*) &connection->uv_tcp, alloc_buffer, on_read) != 0)
        internal_error("uv_read_start error\n");
}

void make_client(caStack* stack)
{
    printf("make_client\n");
    Connection* connection = new Connection();

    ca_assert(stack->world->libuvWorld->uv_loop != NULL);
    uv_tcp_init(stack->world->libuvWorld->uv_loop, &connection->uv_tcp);
    connection->uv_tcp.data = connection;

    Value* ip = circa_input(stack, 0);
    Value* port = circa_input(stack, 1);

    sockaddr_in bind_addr = uv_ip4_addr(circa_string(ip), circa_int(port));
#if 0
    if (uv_ip4_addr(circa_string(ip), circa_int(port), &bind_addr)) {
        printf("error from uv_ip4_addr\n");
        return;
    }
#endif

    uv_connect_t* uv_connect = (uv_connect_t*) malloc(sizeof(*uv_connect));
    uv_connect->data = connection;
    if (uv_tcp_connect(uv_connect, &connection->uv_tcp, bind_addr, client_on_connect)) {
        printf("uv_tcp_connect error\n");
    }

    Value* out = circa_set_default_output(stack, 0);
    circa_set_native_ptr(circa_index(out, 0), connection, ConnectionRelease);
    printf("make_client fin\n");
}

void Connection__send(caStack* stack)
{
    Connection* connection = (Connection*) circa_native_ptr(circa_index(circa_input(stack, 0), 0));

    Value* asStr = circa_alloc_value();
    circa_to_string(circa_input(stack, 1), asStr);
    circa_uv_write((uv_stream_t*) &connection->uv_tcp, asStr, true);
}

void Connection__receive(caStack* stack)
{
    Connection* connection = (Connection*) circa_native_ptr(circa_index(circa_input(stack, 0), 0));
    circa_move(&connection->incomingMsgs, circa_output(stack, 0));
    circa_set_list(&connection->incomingMsgs, 0);
}



LibuvWorld* alloc_libuv_world()
{
    LibuvWorld* state = new LibuvWorld();
    state->uv_loop = uv_loop_new();
    return state;
}

void libuv_native_patch(caWorld* world)
{
    caNativePatch* socket = circa_create_native_patch(world, "socket");
    circa_patch_function(socket, "make_server", make_server);
    circa_patch_function(socket, "Server.connections", Server__connections);
    circa_patch_function(socket, "make_tcp_client", make_client);
    circa_patch_function(socket, "make_tcp_client", make_client);
    circa_patch_function(socket, "Connection.send", Connection__send);
    circa_patch_function(socket, "Connection.receive", Connection__receive);
    circa_finish_native_patch(socket);
}

void libuv_process_events(LibuvWorld* libuvWorld)
{
    uv_run(libuvWorld->uv_loop, UV_RUN_NOWAIT);
}

} // namespace circa
