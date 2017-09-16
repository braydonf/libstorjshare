#include <microhttpd.h>
#include <stdio.h>
#include <sqlite3.h>
#include <json-c/json.h>
#include <assert.h>
#include <uv.h>
#include <errno.h>

#include "storjshare.h"

static int storjshare_connection(void *cls,
                                 struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method,
                                 const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size,
                                 void **con_cls)
{
    const char *encoding = MHD_lookup_connection_value(connection,
                                                       MHD_HEADER_KIND,
                                                       MHD_HTTP_HEADER_CONTENT_TYPE);

    if (NULL == *con_cls) {

        *con_cls = (void *)connection;

        return MHD_YES;
    }

    if (0 == strcmp(url, "/") && 0 == strcmp(method, "POST")) {
        // JSON-RPC API

        // TODO parse json

        // TODO match command

        // TODO dispatch to matching command

    } else if (0 == strncmp(url, "/shards/", 8)) {

        // SHARDS HTTP API

        // TODO get shard hash and token

        if (0 == strcmp(method, "POST")) {

            // TODO check token

            // TODO save shard data

        } else if (0 == strcmp(method, "GET")) {

            // TODO check token
            // TODO check shard exists
            // TODO send shard data

            int status_code = MHD_HTTP_OK;
            char *page = "Shard data!";
            int page_size = 11;
            char *sent_page = calloc(page_size + 1, sizeof(char));
            memcpy(sent_page, page, page_size);
            struct MHD_Response *response;

            response = MHD_create_response_from_buffer(page_size,
                                                       (void *) sent_page,
                                                       MHD_RESPMEM_MUST_FREE);

            int ret = MHD_queue_response(connection, status_code, response);
            if (ret == MHD_NO) {
                fprintf(stderr, "Response error\n");
            }

            MHD_destroy_response(response);

            return ret;

        }

    }

    return MHD_NO;

}

static void storjshare_connection_completed(void *cls,
                                            struct MHD_Connection *connection,
                                            void **con_cls,
                                            enum MHD_RequestTerminationCode toe)
{
    *con_cls = NULL;
}

static void signal_handler(uv_signal_t *req, int signum)
{
    printf("Shutting down!\n");
    uv_signal_stop(req);
}

int main(int argc, char **argv)
{

    int status = 0;

    struct MHD_Daemon *daemon = NULL;
    uv_loop_t *loop = uv_default_loop();
    sqlite3 *db = NULL;

    // TODO read config
    // TODO handle arguments

    char *db_path = "./shards.sqlite";

    if (sqlite3_open(db_path, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        status = 1;
        goto end_program;
    }

    int port = 4001;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DUAL_STACK,
                              port,
                              NULL,
                              NULL,
                              &storjshare_connection,
                              NULL,
                              MHD_OPTION_NOTIFY_COMPLETED,
                              &storjshare_connection_completed,
                              NULL,
                              MHD_OPTION_END);

    if (NULL == daemon) {
        status = 1;
        goto end_program;
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    bool more;
    do {
        more = uv_run(loop, UV_RUN_ONCE);
        if (more == false) {
            more = uv_loop_alive(loop);
            if (uv_run(loop, UV_RUN_NOWAIT) != 0) {
                more = true;
            }
        }

    } while (more == true);


end_program:

    if (loop) {
        uv_loop_close(loop);
    }

    if (daemon) {
        MHD_stop_daemon(daemon);
    }

    if (db) {
        sqlite3_close(db);
    }

    return status;
}
