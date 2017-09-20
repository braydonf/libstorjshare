#include <microhttpd.h>
#include <stdio.h>
#include <sqlite3.h>
#include <json-c/json.h>
#include <assert.h>
#include <uv.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>

#include "storjshare.h"

static inline void noop() {};

#define HELP_TEXT "usage: storjshare [<options>] <command> [<args>]\n\n"         \
    "These are common Storjshare commands for various situations:\n\n"           \
    "  start <config-path>       start a farming node\n"                         \
    "  stop                      stop a farming node\n"                          \
    "  restart                   restart a farming node\n"                       \
    "  status                    check status of node(s)\n"                      \
    "  logs                      tail the logs for a node\n"                     \
    "  create                    create a new configuration\n"                   \
    "  save                      snapshot the currently managed shares\n"        \
    "  load                      load a snapshot of previously managed shares\n" \
    "  destroy                   kills the farming node\n"                       \
    "  killall                   kills all shares and stops the daemon\n"        \
    "  daemon                    starts the daemon\n"                            \
    "  help [cmd]                display help for [cmd]\n\n"                     \
    "options:\n"                                                                 \
    "  -h, --help                output usage information\n"                     \
    "  -v, --version             output the version number\n"                    \
    "environment variables:\n"                                                   \
    "  STORJ_BRIDGE              the bridge host "                               \
    "(e.g. https://api.storj.io)\n"                                              \


#define CLI_VERSION "libstorjshare-0.0.0-alpha.1"


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

static int read_config(char *config_path, char **config_raw)
{
    FILE *config_file = fopen(config_path, "r");

    if (!config_file) {
        printf("Could not open config: %s\n", config_path);
        return 1;
    }

    // TODO read config
    struct stat st;
    stat(config_path, &st);
    int config_size = st.st_size;

    // Read config into config_raw
    *config_raw = calloc(config_size, sizeof(char));;
    fread(*config_raw, config_size, 1, config_file);

    return 0;
}

int main(int argc, char **argv)
{
    int status = 0;
    int c;
    int log_level = 0;
    int index = 0;

    static struct option cmd_options[] = {
        {"version", no_argument,  0, 'v'},
        {"log", required_argument,  0, 'l'},
        {"debug", no_argument,  0, 'd'},
        {"help", no_argument,  0, 'h'},
        {0, 0, 0, 0}
    };

    opterr = 0;

    while ((c = getopt_long_only(argc, argv, "hdl:vV:",
                                 cmd_options, &index)) != -1) {
        switch (c) {
            case 'l':
                log_level = atoi(optarg);
                break;
            case 'd':
                log_level = 4;
                break;
            case 'V':
            case 'v':
                printf(CLI_VERSION "\n\n");
                exit(0);
                break;
            case 'h':
                printf(HELP_TEXT);
                exit(0);
                break;
        }
    }

    if (log_level > 4 || log_level < 0) {
        printf("Invalid log level\n");
        return 1;
    }

    int command_index = optind;

    char *command = argv[command_index];
    if (!command) {
        printf(HELP_TEXT);
        return 0;
    }


    if (strcmp(command, "start") == 0) {
        printf("starting the server\n\n");

        struct MHD_Daemon *daemon = NULL;
        uv_loop_t *loop = uv_default_loop();
        sqlite3 *db = NULL;

        char *config_path = argv[command_index + 1];
        char *config_raw = NULL;

        if (!config_path) {
            printf("Missing first argument: <config-path>\n");
            status = 1;
            goto end_program;
        }

        int ret = read_config(config_path, &config_raw);

        if (ret == 1) {
            printf("Could not read config\n");
            status = 1;
            goto end_program;
        }

        printf("Config:\n%s\n",config_raw);

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

        if (config_raw) {
            free(config_raw);
        }

        return 1;
    }

    return status;
}
