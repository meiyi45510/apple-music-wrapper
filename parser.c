#include "parser.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

#ifndef WRAPPER_VERSION
#define WRAPPER_VERSION "unknown"
#endif

#define WRAPPER_CLI_NAME "wrapper"

enum {
    DEFAULT_DECRYPT_PORT = 10020,
    DEFAULT_M3U8_PORT = 20020,
    DEFAULT_ACCOUNT_PORT = 30020,
};

static const char *const DEFAULT_HOST = "0.0.0.0";
static const char *const DEFAULT_PROXY = "";
static const char *const DEFAULT_DATA_DIR =
    "/data/data/com.apple.android.music/files";
static const char *const DEFAULT_DEVICE_INFO =
    "Music/4.9/Android/10/Samsung S9/7663313/en-US/en-US/"
    "dc28071e981c439e";

static void print_help(void) {
    printf("usage: %s [opt]\n\n", WRAPPER_CLI_NAME);
    puts("opt:\n");
    puts("  -h, --help                help");
    puts("  -V, --version             version");
    printf("  -H, --host=ADDR           host [%s]\n", DEFAULT_HOST);
    printf("  -D, --decrypt-port=PORT   decrypt port [%d]\n",
           DEFAULT_DECRYPT_PORT);
    printf("  -M, --m3u8-port=PORT      m3u8 port [%d]\n",
           DEFAULT_M3U8_PORT);
    printf("  -A, --account-port=PORT   account port [%d]\n",
           DEFAULT_ACCOUNT_PORT);
    puts("  -P, --proxy=URL           proxy [none]");
    puts("  -L, --login=VALUE         login (apple-id or apple-id:password)");
    puts("  -W, --password-file=PATH  password file");
    puts("  -F, --read-2fa-file       read 2fa from <data-dir>/2fa.txt");
    puts("  -B, --data-dir=PATH       data dir");
    printf("                            [%s]\n", DEFAULT_DATA_DIR);
    puts("  -I, --device-info=VALUE   device info");
    printf("                            [%s]\n", DEFAULT_DEVICE_INFO);
}

static void print_version(void) {
    printf("%s %s\n", WRAPPER_CLI_NAME, WRAPPER_VERSION);
}

static int duplicate_string(const char *value, char **out) {
    char *copy = strdup(value);
    if (copy == NULL) {
        log_state(stderr, "allocation", "failed");
        return -1;
    }

    *out = copy;
    return 0;
}

static int set_string_option(char **field, const char *value) {
    char *copy = NULL;

    if (duplicate_string(value, &copy) != 0) {
        return -1;
    }

    free(*field);
    *field = copy;
    return 0;
}

static int parse_int_option(const char *value, const char *option_name,
                            int *out) {
    char *end = NULL;
    long parsed = 0;

    errno = 0;
    parsed = strtol(value, &end, 10);
    if (value[0] == '\0' || *end != '\0' || errno != 0 || parsed < 1 ||
        parsed > 65535) {
        log_value(stderr, "invalid option value", "%s: %s", option_name,
                  value);
        return -1;
    }

    *out = (int)parsed;
    return 0;
}

static const char *option_name_for_short(int opt) {
    switch (opt) {
    case 'h':
        return "--help";
    case 'V':
        return "--version";
    case 'H':
        return "--host";
    case 'D':
        return "--decrypt-port";
    case 'M':
        return "--m3u8-port";
    case 'A':
        return "--account-port";
    case 'P':
        return "--proxy";
    case 'L':
        return "--login";
    case 'W':
        return "--password-file";
    case 'F':
        return "--read-2fa-file";
    case 'B':
        return "--data-dir";
    case 'I':
        return "--device-info";
    default:
        return NULL;
    }
}

static void log_option_issue(const char *subject, int opt,
                             const char *raw_option) {
    const char *option_name = raw_option;

    if (option_name == NULL || option_name[0] == '\0' || option_name[0] != '-') {
        option_name = option_name_for_short(opt);
    }

    if (option_name == NULL || option_name[0] == '\0') {
        option_name = "unknown";
    }

    log_value(stderr, subject, "%s", option_name);
}

static int init_defaults(struct cli_options *options) {
    memset(options, 0, sizeof(*options));

    if (duplicate_string(DEFAULT_HOST, &options->host) != 0 ||
        duplicate_string(DEFAULT_PROXY, &options->proxy) != 0 ||
        duplicate_string(DEFAULT_DATA_DIR, &options->data_dir) != 0 ||
        duplicate_string(DEFAULT_DEVICE_INFO, &options->device_info) != 0) {
        cli_options_free(options);
        return -1;
    }

    options->decrypt_port = DEFAULT_DECRYPT_PORT;
    options->m3u8_port = DEFAULT_M3U8_PORT;
    options->account_port = DEFAULT_ACCOUNT_PORT;
    return 0;
}

int cli_options_parse(int argc, char **argv,
                      struct cli_options *options) {
    int opt = 0;
    int option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'V'},
        {"host", required_argument, NULL, 'H'},
        {"decrypt-port", required_argument, NULL, 'D'},
        {"m3u8-port", required_argument, NULL, 'M'},
        {"account-port", required_argument, NULL, 'A'},
        {"proxy", required_argument, NULL, 'P'},
        {"login", required_argument, NULL, 'L'},
        {"password-file", required_argument, NULL, 'W'},
        {"read-2fa-file", no_argument, NULL, 'F'},
        {"data-dir", required_argument, NULL, 'B'},
        {"device-info", required_argument, NULL, 'I'},
        {0, 0, 0, 0},
    };

    if (init_defaults(options) != 0) {
        return -1;
    }

    optind = 1;
    opterr = 0;

    while ((opt = getopt_long(argc, argv, ":hVH:D:M:A:P:L:W:FB:I:", long_options,
                              &option_index)) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            exit(EXIT_SUCCESS);
        case 'V':
            print_version();
            exit(EXIT_SUCCESS);
        case 'H':
            if (set_string_option(&options->host, optarg) != 0) {
                goto fail;
            }
            break;
        case 'D':
            if (parse_int_option(optarg, "--decrypt-port",
                                 &options->decrypt_port) != 0) {
                goto fail;
            }
            break;
        case 'M':
            if (parse_int_option(optarg, "--m3u8-port",
                                 &options->m3u8_port) != 0) {
                goto fail;
            }
            break;
        case 'A':
            if (parse_int_option(optarg, "--account-port",
                                 &options->account_port) != 0) {
                goto fail;
            }
            break;
        case 'P':
            if (set_string_option(&options->proxy, optarg) != 0) {
                goto fail;
            }
            options->proxy_given = 1;
            break;
        case 'L':
            if (set_string_option(&options->login, optarg) != 0) {
                goto fail;
            }
            options->login_given = 1;
            break;
        case 'W':
            if (set_string_option(&options->password_file, optarg) != 0) {
                goto fail;
            }
            break;
        case 'F':
            options->read_2fa_file = 1;
            break;
        case 'B':
            if (set_string_option(&options->data_dir, optarg) != 0) {
                goto fail;
            }
            break;
        case 'I':
            if (set_string_option(&options->device_info, optarg) != 0) {
                goto fail;
            }
            break;
        case ':':
            log_option_issue("missing option value", optopt,
                             optind > 0 ? argv[optind - 1] : NULL);
            goto fail;
        case '?':
            log_option_issue("unknown option", optopt,
                             optind > 0 ? argv[optind - 1] : NULL);
            goto fail;
        default:
            goto fail;
        }
    }

    if (optind < argc) {
        log_value(stderr, "unexpected argument", "%s", argv[optind]);
        goto fail;
    }

    return 0;

fail:
    cli_options_free(options);
    return -1;
}

void cli_options_free(struct cli_options *options) {
    if (options == NULL) {
        return;
    }

    free(options->host);
    free(options->proxy);
    free(options->login);
    free(options->password_file);
    free(options->data_dir);
    free(options->device_info);
    memset(options, 0, sizeof(*options));
}
