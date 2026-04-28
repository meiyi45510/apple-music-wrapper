#pragma once

struct cli_options {
    char *host;
    int decrypt_port;
    int m3u8_port;
    int account_port;
    char *proxy;
    int proxy_given;
    char *login;
    int login_given;
    char *password_file;
    int read_2fa_file;
    char *data_dir;
    char *device_info;
};

int cli_options_parse(int argc, char **argv,
                      struct cli_options *options);
void cli_options_free(struct cli_options *options);
