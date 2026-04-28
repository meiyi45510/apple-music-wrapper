#include <errno.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "parser.h"
#include "symbol.h"
#include "logger.h"

enum {
    DEVICE_INFO_FIELD_COUNT = 9,
    TWO_FACTOR_CODE_LEN = 6,
    CURLOPT_SSL_VERIFYPEER = 64,
    CURLOPT_VERBOSE = 43,
    CURLOPT_SSL_VERIFYHOST = 81,
    CURLOPT_PINNEDPUBLICKEY = 10230,
};

static struct shared_ptr presentation_interface;
static uint8_t lease_manager[16];
static struct shared_ptr request_context;
static struct cli_options options;
static char *login_apple_id, *login_password;
static struct shared_ptr guid_handle;
static int is_offline_available;
static char *device_fields[DEVICE_INFO_FIELD_COUNT];

// account cache
static char *cached_storefront_id = NULL;
static char *cached_dev_token = NULL;
static char *cached_music_token = NULL;
static void *curl_lib_handle = NULL;

typedef int (*curl_easy_setopt_fn)(void *curl, int option, ...);

static curl_easy_setopt_fn real_curl_easy_setopt = NULL;

static char *json_extract_string(const char *json, const char *key) {
    if (json == NULL || key == NULL) {
        return NULL;
    }

    size_t key_len = strlen(key);
    size_t needle_len = key_len + 2;
    char *needle = malloc(needle_len + 1);
    if (needle == NULL) {
        return NULL;
    }

    needle[0] = '"';
    memcpy(needle + 1, key, key_len);
    needle[key_len + 1] = '"';
    needle[needle_len] = '\0';

    char *key_pos = strstr(json, needle);
    free(needle);
    if (key_pos == NULL) {
        return NULL;
    }

    char *colon = strchr(key_pos + needle_len, ':');
    if (colon == NULL) {
        return NULL;
    }

    char *value = colon + 1;
    while (*value == ' ' || *value == '\t' || *value == '\r' || *value == '\n') {
        value++;
    }
    if (*value != '"') {
        return NULL;
    }
    value++;

    char *end = value;
    while (*end != '\0') {
        if (*end == '"' && end > value && end[-1] != '\\') {
            break;
        }
        end++;
    }
    if (*end != '"') {
        return NULL;
    }

    size_t out_len = (size_t)(end - value);
    char *result = malloc(out_len + 1);
    if (result == NULL) {
        return NULL;
    }
    memcpy(result, value, out_len);
    result[out_len] = '\0';
    return result;
}

static int file_exists(const char *filename) {
    struct stat buffer;

    return stat(filename, &buffer) == 0;
}

static const char *nonnull_string(const char *value) {
    return value == NULL ? "" : value;
}

static int duplicate_substring(const char *src, size_t len, char **out) {
    char *copy = malloc(len + 1);
    if (copy == NULL) {
        return 0;
    }

    memcpy(copy, src, len);
    copy[len] = '\0';
    *out = copy;
    return 1;
}

static char *join_path(const char *dest, const char *src) {
    size_t len1 = strlen(dest);
    size_t len2 = strlen(src);

    char *result = malloc(len1 + len2 + 1);
    if (result == NULL) {
        return NULL;
    }

    strcpy(result, dest);
    strcat(result, src);

    return result;
}

static char *read_trimmed_file(const char *path) {
    FILE *fp = fopen(path, "r");
    long size = 0;
    size_t bytes_read = 0;
    char *buffer = NULL;

    if (fp == NULL) {
        return NULL;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    buffer = calloc((size_t)size + 1, 1);
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);
    while (bytes_read > 0 &&
           (buffer[bytes_read - 1] == '\n' || buffer[bytes_read - 1] == '\r')) {
        bytes_read--;
    }
    buffer[bytes_read] = '\0';
    if (bytes_read == 0) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

static int read_two_factor_code(FILE *stream, char *buffer) {
    char line[32];
    size_t code_len = 0;

    if (stream == NULL || buffer == NULL) {
        return 0;
    }
    if (fgets(line, sizeof(line), stream) == NULL) {
        return 0;
    }

    code_len = strcspn(line, "\r\n");
    line[code_len] = '\0';
    if (code_len != TWO_FACTOR_CODE_LEN) {
        return 0;
    }

    memcpy(buffer, line, code_len + 1);
    return 1;
}

static int set_login_password(const char *password_value) {
    size_t password_len = strlen(password_value);
    char *password = calloc(password_len + TWO_FACTOR_CODE_LEN + 1, 1);

    if (password == NULL) {
        return 0;
    }

    memcpy(password, password_value, password_len);
    free(login_password);
    login_password = password;
    return 1;
}

static int ensure_login_password_loaded(void) {
    char *password_value = NULL;
    size_t password_len = 0;

    if (login_password != NULL) {
        return 1;
    }
    if (options.password_file == NULL) {
        return 0;
    }

    password_value = read_trimmed_file(options.password_file);
    remove(options.password_file);
    if (password_value == NULL) {
        return 0;
    }

    password_len = strlen(password_value);
    if (!set_login_password(password_value)) {
        memset(password_value, 0, password_len);
        free(password_value);
        return 0;
    }

    memset(password_value, 0, password_len);
    free(password_value);
    return 1;
}

static curl_easy_setopt_fn resolve_curl_easy_setopt(void) {
    if (real_curl_easy_setopt != NULL) {
        return real_curl_easy_setopt;
    }

    if (curl_lib_handle == NULL) {
        curl_lib_handle = dlopen("libcurl.so", RTLD_NOW | RTLD_LOCAL);
        if (curl_lib_handle == NULL) {
            curl_lib_handle = dlopen("/system/lib64/libcurl.so",
                                     RTLD_NOW | RTLD_LOCAL);
        }
    }
    if (curl_lib_handle == NULL) {
        return NULL;
    }

    real_curl_easy_setopt =
        (curl_easy_setopt_fn)dlsym(curl_lib_handle, "curl_easy_setopt");
    return real_curl_easy_setopt;
}

int curl_easy_setopt(void *curl, int option, ...) {
    curl_easy_setopt_fn fn = resolve_curl_easy_setopt();
    va_list args;
    void *param = NULL;

    if (fn == NULL) {
        return 0;
    }

    va_start(args, option);
    param = va_arg(args, void *);
    va_end(args);

    if (option == CURLOPT_SSL_VERIFYPEER ||
        option == CURLOPT_SSL_VERIFYHOST ||
        option == CURLOPT_PINNEDPUBLICKEY) {
        return fn(curl, option, 0L);
    }
    if (option == CURLOPT_VERBOSE) {
        return fn(curl, option, 1L);
    }
    return fn(curl, option, param);
}

static int parse_login_credentials(const char *login_arg) {
    const char *separator = NULL;
    size_t username_len = 0;
    char *username = NULL;

    if (login_arg == NULL) {
        return 0;
    }
    separator = strchr(login_arg, ':');
    if (separator == login_arg) {
        return 0;
    }
    if (separator == NULL) {
        username_len = strlen(login_arg);
    } else {
        username_len = (size_t)(separator - login_arg);
    }
    if (username_len == 0 ||
        !duplicate_substring(login_arg, username_len, &username)) {
        return 0;
    }

    login_apple_id = username;

    if (separator != NULL) {
        if (separator[1] == '\0' || !set_login_password(separator + 1)) {
            free(username);
            login_apple_id = NULL;
            return 0;
        }
    }

    return 1;
}

static void dialog_handler(long dialog_id, struct shared_ptr *dialog_ptr,
                           struct shared_ptr *response_handler) {
    (void)response_handler;
    const char *const title = std_string_data(
        _ZNK17storeservicescore14ProtocolDialog5titleEv(dialog_ptr->obj));

    unsigned char ptr[72];
    memset(ptr + 8, 0, 16);
    *(void **)(ptr) =
        &_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore22ProtocolDialogResponseENS_9allocatorIS2_EEEE +
        2;
    struct shared_ptr dialog_response = {.obj = ptr + 24, .ctrl_blk = ptr};
    _ZN17storeservicescore22ProtocolDialogResponseC1Ev(dialog_response.obj);

    struct std_vector *button_vec =
        _ZNK17storeservicescore14ProtocolDialog7buttonsEv(dialog_ptr->obj);
    if (strcmp("Sign In", title) == 0) {
        for (struct shared_ptr *button = button_vec->begin;
             button != button_vec->end; ++button) {
            if (strcmp("Use Existing Apple ID",
                       std_string_data(
                           _ZNK17storeservicescore14ProtocolButton5titleEv(
                               button->obj))) == 0) {
                _ZN17storeservicescore22ProtocolDialogResponse17setSelectedButtonERKNSt6__ndk110shared_ptrINS_14ProtocolButtonEEE(
                    dialog_response.obj, button);
                break;
            }
        }
    }
    _ZN20androidstoreservices28AndroidPresentationInterface28handleProtocolDialogResponseERKlRKNSt6__ndk110shared_ptrIN17storeservicescore22ProtocolDialogResponseEEE(
        presentation_interface.obj, &dialog_id, &dialog_response);
}

static void credential_handler(struct shared_ptr *credential_request,
                               struct shared_ptr *response_handler) {
    (void)response_handler;
    const uint8_t needs_2fa =
        _ZNK17storeservicescore18CredentialsRequest28requiresHSA2VerificationCodeEv(
            credential_request->obj);
    if (!ensure_login_password_loaded()) {
        log_state(stderr, "password", "missing");
        exit(EXIT_FAILURE);
    }

    size_t password_len = strlen(login_password);

    if (needs_2fa) {
        if (options.read_2fa_file) {
            log_value(stderr, "2fa file", "rootfs%s/2fa.txt",
                      options.data_dir);
            log_state(stderr, "2fa code", "waiting");
            char *path = join_path(options.data_dir, "/2fa.txt");
            int count = 0;
            if (path == NULL) {
                log_state(stderr, "2fa path allocation", "failed");
                exit(EXIT_FAILURE);
            }
            while (1) {
                if (count >= 20) {
                    log_state(stderr, "2fa wait", "timed out");
                    free(path);
                    exit(EXIT_FAILURE);
                }

                if (file_exists(path)) {
                    FILE *fp = fopen(path, "r");
                    if (fp != NULL) {
                        if (!read_two_factor_code(
                                fp, login_password + password_len)) {
                            fclose(fp);
                            log_state(stderr, "2fa code", "invalid");
                        } else {
                            fclose(fp);
                            remove(path);
                            log_state(stderr, "2fa code", "loaded");
                            break;
                        }
                    }
                }
                sleep(3);
                count++;
            }
            free(path);
        } else {
            log_prompt("2fa code");
            if (!read_two_factor_code(stdin, login_password + password_len)) {
                log_state(stderr, "2fa code", "invalid");
                exit(EXIT_FAILURE);
            }
        }
    }

    uint8_t *const ptr = malloc(80);
    memset(ptr + 8, 0, 16);
    *(void **)(ptr) =
        &_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore19CredentialsResponseENS_9allocatorIS2_EEEE +
        2;
    struct shared_ptr credentials_response = {.obj = ptr + 24, .ctrl_blk = ptr};
    _ZN17storeservicescore19CredentialsResponseC1Ev(credentials_response.obj);

    union std_string username = new_std_string(login_apple_id);
    _ZN17storeservicescore19CredentialsResponse11setUserNameERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        credentials_response.obj, &username);

    union std_string password = new_std_string(login_password);
    _ZN17storeservicescore19CredentialsResponse11setPasswordERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        credentials_response.obj, &password);

    _ZN17storeservicescore19CredentialsResponse15setResponseTypeENS0_12ResponseTypeE(
        credentials_response.obj, 2);

    _ZN20androidstoreservices28AndroidPresentationInterface25handleCredentialsResponseERKNSt6__ndk110shared_ptrIN17storeservicescore19CredentialsResponseEEE(
        presentation_interface.obj, &credentials_response);
}

static inline void initialize_wrapper(void) {
    log_state(stderr, "wrapper", "start");
    setenv("ANDROID_DNS_MODE", "local", 1);
    if (options.proxy_given) {
        log_value(stderr, "proxy", "%s", options.proxy);
        setenv("all_proxy", options.proxy, 1);
    }

    static const char *resolvers[2] = {"223.5.5.5", "223.6.6.6"};
    _resolv_set_nameservers_for_net(0, resolvers, 2, ".");
    union std_string conf1 = new_std_string(device_fields[8]);
    union std_string conf2 = new_std_string("");
    _ZN14FootHillConfig6configERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(
        &conf1);

    _ZN17storeservicescore10DeviceGUID8instanceEvASM(&guid_handle);

    static uint8_t ret[88];
    static unsigned int conf3 = 29;
    static uint8_t conf4 = 1;
    _ZN17storeservicescore10DeviceGUID9configureERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKjRKbASM(
        &ret, guid_handle.obj, &conf1, &conf2, &conf3, &conf4);
}

static void output_thread_error(const char *label, int error_code) {
    log_value(stderr, label, "%s", strerror(error_code));
}

static inline struct shared_ptr create_request_context(void) {
    log_state(stderr, "request context", "init");

    char *database_directory = join_path(options.data_dir, "/mpl_db");
    if (database_directory == NULL) {
        log_state(stderr, "request context dir", "allocation failed");
        exit(EXIT_FAILURE);
    }

    struct shared_ptr *request_context_ptr =
        new_request_context(database_directory);
    struct shared_ptr *request_context_config =
        new_request_context_config();

    union std_string data_directory_path = new_std_string(database_directory);
    _ZN17storeservicescore20RequestContextConfig20setBaseDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &data_directory_path);
    union std_string fair_play_directory_path =
        new_std_string(options.data_dir);
    _ZN17storeservicescore20RequestContextConfig24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &fair_play_directory_path);

    union std_string client_identifier = new_std_string(device_fields[0]);
    _ZN17storeservicescore20RequestContextConfig19setClientIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &client_identifier);

    union std_string version_identifier = new_std_string(device_fields[1]);
    _ZN17storeservicescore20RequestContextConfig20setVersionIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &version_identifier);

    union std_string platform_identifier = new_std_string(device_fields[2]);
    _ZN17storeservicescore20RequestContextConfig21setPlatformIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &platform_identifier);

    union std_string product_version = new_std_string(device_fields[3]);
    _ZN17storeservicescore20RequestContextConfig17setProductVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &product_version);

    union std_string device_model = new_std_string(device_fields[4]);
    _ZN17storeservicescore20RequestContextConfig14setDeviceModelERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &device_model);

    union std_string build_version = new_std_string(device_fields[5]);
    _ZN17storeservicescore20RequestContextConfig15setBuildVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &build_version);

    union std_string locale = new_std_string(device_fields[6]);
    _ZN17storeservicescore20RequestContextConfig19setLocaleIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &locale);

    union std_string language = new_std_string(device_fields[7]);
    _ZN17storeservicescore20RequestContextConfig21setLanguageIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        request_context_config->obj, &language);
    configure_request_context(request_context_ptr);
    free(database_directory);

    static uint8_t buf[88];
    init_request_context(buf, request_context_ptr, request_context_config);
    init_presentation_interface(
        &presentation_interface, &dialog_handler, &credential_handler);
    set_presentation_interface(request_context_ptr, &presentation_interface);

    return *request_context_ptr;
}

extern void *lease_end_callback_fn;
extern void *playback_error_callback_fn;

static inline uint8_t login(struct shared_ptr request_context_ref) {
    log_state(stderr, "login", "start");
    struct shared_ptr flow;
    _ZNSt6__ndk110shared_ptrIN17storeservicescore16AuthenticateFlowEE11make_sharedIJRNS0_INS1_14RequestContextEEEEEES3_DpOT_ASM(
        &flow, &request_context_ref);
    _ZN17storeservicescore16AuthenticateFlow3runEv(flow.obj);
    struct shared_ptr *resp =
        _ZNK17storeservicescore16AuthenticateFlow8responseEv(flow.obj);
    if (resp == NULL || resp->obj == NULL) {
        return 0;
    }
    const int response_type =
        _ZNK17storeservicescore20AuthenticateResponse12responseTypeEv(
            resp->obj);
    return response_type == 6;
}

static inline uint8_t read_full(const int connfd, void *const buf,
                                const size_t size) {
    size_t total_read = 0;

    while (size > total_read) {
        const ssize_t bytes_read =
            read(connfd, ((uint8_t *)buf) + total_read, size - total_read);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        if (bytes_read == 0) {
            return 0;
        }
        total_read += (size_t)bytes_read;
    }
    return 1;
}

static inline void write_full(const int connfd, const void *buf,
                              const size_t size) {
    size_t total_written = 0;

    while (size > total_written) {
        const ssize_t bytes_written =
            write(connfd, ((uint8_t *)buf) + total_written,
                  size - total_written);
        if (bytes_written < 0) {
            if (errno == EINTR) {
                continue;
            }
            log_errno("write");
            break;
        }
        if (bytes_written == 0) {
            break;
        }
        total_written += (size_t)bytes_written;
    }
}

static ssize_t read_http_headers(const int connfd, char *buffer,
                                 const size_t size) {
    size_t total = 0;

    while (total + 1 < size) {
        const ssize_t n = read(connfd, buffer + total, size - total - 1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            break;
        }
        total += (size_t)n;
        buffer[total] = '\0';
        if (strstr(buffer, "\r\n\r\n") != NULL ||
            strstr(buffer, "\n\n") != NULL) {
            return (ssize_t)total;
        }
    }
    buffer[total] = '\0';
    return (ssize_t)total;
}

static void *foot_hill_instance = NULL;
static void *preshared_context = NULL;

static int should_retry_accept_error(int error_code) {
    return error_code == ENETDOWN || error_code == EPROTO ||
           error_code == ENOPROTOOPT || error_code == EHOSTDOWN ||
#ifdef ENONET
           error_code == ENONET ||
#endif
           error_code == EHOSTUNREACH || error_code == EOPNOTSUPP ||
           error_code == ENETUNREACH;
}

static int create_listener_socket(const char *host, int port,
                                  const char *label) {
    const int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd == -1) {
        log_errno("socket");
        return -1;
    }

    const int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    struct sockaddr_in serv_addr = {.sin_family = AF_INET};
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) != 1) {
        log_value(stderr, "invalid host", "%s", host);
        close(fd);
        return -1;
    }
    serv_addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        log_errno("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 5) == -1) {
        log_errno("listen");
        close(fd);
        return -1;
    }

    if (label == NULL) {
        log_value(stderr, "listening", "%s:%d", host, port);
    } else {
        log_value(stderr, "listening", "%s %s:%d", label, host, port);
    }

    return fd;
}

static int accept_client(const int fd, struct sockaddr_in *peer_addr,
                         socklen_t *peer_addr_size) {
    while (1) {
        const int connfd = accept4(fd, (struct sockaddr *)peer_addr,
                                   peer_addr_size, SOCK_CLOEXEC);
        if (connfd != -1 || !should_retry_accept_error(errno)) {
            return connfd;
        }
    }
}

static inline void *get_kd_context(const char *const adam,
                                   const char *const uri) {
    uint8_t is_preshare = (strcmp("0", adam) == 0);
    if (is_preshare && preshared_context != NULL) {
        return preshared_context;
    }

    union std_string default_id = new_std_string(adam);
    union std_string key_uri = new_std_string(uri);
    union std_string key_format =
        new_std_string("com.apple.streamingkeydelivery");
    union std_string key_format_version = new_std_string("1");
    union std_string server_uri = new_std_string(
        "https://play.itunes.apple.com/WebObjects/MZPlay.woa/music/fps");
    union std_string protocol_type = new_std_string("simplified");
    union std_string fps_cert = new_std_string(fairplay_cert);

    struct shared_ptr persistent_key = {.obj = NULL};
    _ZN21SVFootHillSessionCtrl16getPersistentKeyERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_S8_S8_S8_S8_S8_ASM(
        &persistent_key, foot_hill_instance, &default_id, &default_id,
        &key_uri,
        &key_format, &key_format_version, &server_uri, &protocol_type,
        &fps_cert);

    if (persistent_key.obj == NULL) {
        return NULL;
    }

    struct shared_ptr foot_hill_context;
    _ZN21SVFootHillSessionCtrl14decryptContextERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN11SVDecryptor15SVDecryptorTypeERKbASM(
        &foot_hill_context, foot_hill_instance, persistent_key.obj);

    if (foot_hill_context.obj == NULL) {
        return NULL;
    }

    void *kd_context =
        *_ZNK18SVFootHillPContext9kdContextEv(foot_hill_context.obj);
    if (kd_context != NULL && is_preshare) {
        preshared_context = kd_context;
    }
    return kd_context;
}

void handle_decrypt_session(const int connfd) {
    while (1) {
        uint8_t adam_size;
        if (!read_full(connfd, &adam_size, sizeof(uint8_t))) {
            return;
        }
        if (adam_size <= 0) {
            return;
        }

        char adam[adam_size + 1];
        if (!read_full(connfd, adam, adam_size)) {
            return;
        }
        adam[adam_size] = '\0';

        uint8_t uri_size;
        if (!read_full(connfd, &uri_size, sizeof(uint8_t))) {
            return;
        }

        char uri[uri_size + 1];
        if (!read_full(connfd, uri, uri_size)) {
            return;
        }
        uri[uri_size] = '\0';

        void **const kd_context = get_kd_context(adam, uri);
        if (kd_context == NULL) {
            return;
        }

        while (1) {
            uint32_t size;
            if (!read_full(connfd, &size, sizeof(uint32_t))) {
                log_errno("read");
                return;
            }

            if (size <= 0) {
                break;
            }

            void *sample = malloc(size);
            if (sample == NULL) {
                log_errno("malloc");
                return;
            }
            if (!read_full(connfd, sample, size)) {
                free(sample);
                log_errno("read");
                return;
            }

            NfcRKVnxuKZy04KWbdFu71Ou(*kd_context, 5, sample, sample, size);
            write_full(connfd, sample, size);
            free(sample);
        }
    }
}

extern uint8_t handle_decrypt_guarded(int);

static inline int serve_decrypt(void) {
    const int fd =
        create_listener_socket(options.host, options.decrypt_port, NULL);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    struct sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(peer_addr);
    while (1) {
        const int connfd = accept_client(fd, &peer_addr, &peer_addr_size);
        if (connfd == -1) {
            log_errno("accept4");
            return EXIT_FAILURE;
        }

        if (!handle_decrypt_guarded(connfd)) {
            uint8_t lease_request_enabled = 1;
            _ZN22SVPlaybackLeaseManager12requestLeaseERKb(
                lease_manager, &lease_request_enabled);
        }
        if (close(connfd) == -1) {
            log_errno("close");
            return EXIT_FAILURE;
        }
    }
}

static const char *get_m3u8_method_download(
    struct shared_ptr request_context_ref, unsigned long adam) {
    void *purchase_request = malloc(1024);
    _ZN17storeservicescore15PurchaseRequestC2ERKNSt6__ndk110shared_ptrINS_14RequestContextEEE(
        purchase_request, &request_context_ref);
    _ZN17storeservicescore15PurchaseRequest23setProcessDialogActionsEb(
        purchase_request, 1);
    union std_string url_bag_key = new_std_string("subDownload");
    _ZN17storeservicescore15PurchaseRequest12setURLBagKeyERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        purchase_request, &url_bag_key);
    char buy_parameters_str[128];
    snprintf(buy_parameters_str, sizeof(buy_parameters_str),
             "salableAdamId=%lu&price=0&pricingParameters=SUBS&productType=S",
             adam);
    union std_string buy_parameters = new_std_string(buy_parameters_str);
    _ZN17storeservicescore15PurchaseRequest16setBuyParametersERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        purchase_request, &buy_parameters);
    _ZN17storeservicescore15PurchaseRequest3runEv(purchase_request);
    struct shared_ptr *response =
        _ZNK17storeservicescore15PurchaseRequest8responseEv(
            purchase_request);
    struct shared_ptr *error =
        _ZN17storeservicescore16PurchaseResponse5errorEv(response->obj);
    if (error->obj == NULL) {
        struct std_vector items =
            _ZNK17storeservicescore16PurchaseResponse5itemsEv(response->obj);
        struct shared_ptr *first_item = items.begin;
        struct std_vector assets =
            _ZNK17storeservicescore12PurchaseItem6assetsEv(first_item->obj);
        struct shared_ptr *last_asset = (struct shared_ptr *)assets.end - 1;
        union std_string url_str;
        _ZNK17storeservicescore13PurchaseAsset3URLEvASM(&url_str,
                                                         last_asset->obj);
        const char *url = std_string_data(&url_str);
        if (url) {
            return strdup(url);
        }
    }
    return NULL;
}

static const char *get_m3u8_method_play(uint8_t lease_manager_buf[16],
                                        unsigned long adam) {
    union std_string hls = new_std_string_short_mode("HLS");
    struct std_vector hls_params = new_std_vector(&hls);
    static uint8_t z0 = 0;
    struct shared_ptr ptr_result;
    _ZN22SVPlaybackLeaseManager12requestAssetERKmRKNSt6__ndk16vectorINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS7_IS9_EEEERKbASM(
        &ptr_result, lease_manager_buf, &adam, &hls_params, &z0);

    if (ptr_result.obj == NULL) {
        return NULL;
    }

    if (_ZNK23SVPlaybackAssetResponse13hasValidAssetEv(ptr_result.obj)) {
        struct shared_ptr *playback_asset =
            _ZNK23SVPlaybackAssetResponse13playbackAssetEv(ptr_result.obj);
        if (playback_asset == NULL || playback_asset->obj == NULL) {
            return NULL;
        }

        void *playback_obj = playback_asset->obj;
        union std_string m3u8;
        _ZNK17storeservicescore13PlaybackAsset9URLStringEvASM(&m3u8,
                                                               playback_obj);
        const char *m3u8_str = std_string_data(&m3u8);
        if (m3u8_str == NULL) {
            return NULL;
        }

        return strdup(m3u8_str);
    }

    return NULL;
}

static void handle_m3u8_request(const int connfd) {
    while (1) {
        uint8_t adam_size;
        if (!read_full(connfd, &adam_size, sizeof(uint8_t))) {
            return;
        }
        if (adam_size <= 0) {
            return;
        }

        char adam[adam_size + 1];
        if (!read_full(connfd, adam, adam_size)) {
            return;
        }
        adam[adam_size] = '\0';

        unsigned long adam_id = strtoul(adam, NULL, 10);
        const char *m3u8;
        if (is_offline_available) {
            m3u8 = get_m3u8_method_download(request_context, adam_id);
        } else {
            m3u8 = get_m3u8_method_play(lease_manager, adam_id);
        }
        if (m3u8 == NULL) {
            log_value(stderr, "m3u8 missing", "%ld", adam_id);
            write_full(connfd, "\n", 1);
        } else {
            char *with_newline = malloc(strlen(m3u8) + 2);
            if (with_newline) {
                strcpy(with_newline, m3u8);
                strcat(with_newline, "\n");
                write_full(connfd, with_newline, strlen(with_newline));
                free(with_newline);
            }
            free((void *)m3u8);
        }
    }
}

static inline void *serve_m3u8(void *args) {
    (void)args;
    const int fd = create_listener_socket(options.host,
                                          options.m3u8_port,
                                          "m3u8");
    if (fd == -1) {
        return NULL;
    }

    struct sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(peer_addr);
    while (1) {
        const int connfd = accept_client(fd, &peer_addr, &peer_addr_size);
        if (connfd == -1) {
            log_errno("accept4");
            return NULL;
        }

        handle_m3u8_request(connfd);

        if (close(connfd) == -1) {
            log_errno("close");
        }
    }
}

static void handle_account_request(const int connfd) {
    char buffer[4096];
    ssize_t n = read_http_headers(connfd, buffer, sizeof(buffer));
    if (n <= 0) {
        log_value(stderr, "account request read failed", "%zd errno=%d",
                  n, errno);
        return;
    }

    if (strncmp(buffer, "GET", 3) != 0 && strncmp(buffer, "POST", 4) != 0) {
        log_value(stderr, "invalid account request", "%.16s", buffer);
        const char *error_response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 0\r\n\r\n";
        write_full(connfd, error_response, strlen(error_response));
        shutdown(connfd, SHUT_WR);
        return;
    }

    const char *storefront_id = nonnull_string(cached_storefront_id);
    const char *dev_token = nonnull_string(cached_dev_token);
    const char *music_token = nonnull_string(cached_music_token);
    int json_len = snprintf(
        NULL, 0,
        "{\"storefront_id\":\"%s\",\"dev_token\":\"%s\",\"music_token\":\"%s\"}",
        storefront_id, dev_token, music_token);
    if (json_len < 0) {
        log_state(stderr, "account info format", "failed");
        const char *error_response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 0\r\n\r\n";
        write_full(connfd, error_response, strlen(error_response));
        return;
    }

    char *json_body = malloc((size_t)json_len + 1);
    if (json_body == NULL) {
        log_state(stderr, "account info allocation", "failed");
        const char *error_response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 0\r\n\r\n";
        write_full(connfd, error_response, strlen(error_response));
        return;
    }

    snprintf(
        json_body, (size_t)json_len + 1,
        "{\"storefront_id\":\"%s\",\"dev_token\":\"%s\",\"music_token\":\"%s\"}",
        storefront_id, dev_token, music_token);

    int response_len = snprintf(
        NULL, 0,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n",
        json_len);
    if (response_len < 0) {
        log_state(stderr, "account response format", "failed");
        free(json_body);
        const char *error_response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 0\r\n\r\n";
        write_full(connfd, error_response, strlen(error_response));
        return;
    }

    char *http_response = malloc((size_t)response_len + 1);
    if (http_response == NULL) {
        log_state(stderr, "account response allocation", "failed");
        free(json_body);
        const char *error_response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 0\r\n\r\n";
        write_full(connfd, error_response, strlen(error_response));
        return;
    }

    snprintf(
        http_response, (size_t)response_len + 1,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n",
        json_len);

    write_full(connfd, http_response, strlen(http_response));
    write_full(connfd, json_body, json_len);
    shutdown(connfd, SHUT_WR);

    free(http_response);
    free(json_body);
}

static inline void *serve_account(void *args) {
    (void)args;
    const int fd = create_listener_socket(options.host,
                                          options.account_port,
                                          "account");
    if (fd == -1) {
        return NULL;
    }

    struct sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(peer_addr);
    while (1) {
        const int connfd = accept_client(fd, &peer_addr, &peer_addr_size);
        if (connfd == -1) {
            log_errno("accept4");
            return NULL;
        }

        handle_account_request(connfd);

        if (close(connfd) == -1) {
            log_errno("close");
        }
    }
}

static char *get_account_storefront_id(struct shared_ptr request_context_ref) {
    union std_string region;
    struct shared_ptr url_bag = {.obj = 0x0, .ctrl_blk = 0x0};
    _ZNK17storeservicescore14RequestContext20storeFrontIdentifierERKNSt6__ndk110shared_ptrINS_6URLBagEEEASM(
        &region, request_context_ref.obj, &url_bag);
    const char *region_str = std_string_data(&region);
    if (region_str) {
        return strdup(region_str);
    }
    return NULL;
}

static void write_storefront_id(void) {
    char *path = join_path(options.data_dir, "/STOREFRONT_ID");
    if (path == NULL) {
        return;
    }

    FILE *fp = fopen(path, "w");
    free(path);
    if (fp == NULL) {
        return;
    }

    log_state(stdout, "storefront", "saved");
    fprintf(fp, "%s", nonnull_string(cached_storefront_id));
    fclose(fp);
}

static char *get_guid(void) {
    char *ret[2];
    _ZN17storeservicescore10DeviceGUID4guidEvASM(ret, guid_handle.obj);
    char *guid = _ZNK13mediaplatform4Data5bytesEv(ret[0]);
    return guid;
}

static long long get_current_time_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

static char *get_music_user_token(char *guid, char *auth_token,
                                  struct shared_ptr request_context_ref) {
    const char *guid_value = nonnull_string(guid);
    const char *auth_token_value = nonnull_string(auth_token);
    uint8_t ptr[480];
    *(void **)(ptr) =
        &_ZTVNSt6__ndk120__shared_ptr_emplaceIN13mediaplatform11HTTPMessageENS_9allocatorIS2_EEEE +
        2;
    struct shared_ptr http_message = {.obj = ptr + 32, .ctrl_blk = ptr};
    union std_string url = new_std_string(
        "https://play.itunes.apple.com/WebObjects/MZPlay.woa/"
        "wa/createMusicToken");
    union std_string method = new_std_string("POST");
    _ZN13mediaplatform11HTTPMessageC2ENSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES7_(
        http_message.obj, &url, &method);
    union std_string content_type_header = new_std_string("Content-Type");
    union std_string content_type_value =
        new_std_string("application/json; charset=UTF-8");
    _ZN13mediaplatform11HTTPMessage9setHeaderERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(
        http_message.obj, &content_type_header, &content_type_value);
    union std_string expect_header = new_std_string("Expect");
    union std_string expect_value = new_std_string("");
    _ZN13mediaplatform11HTTPMessage9setHeaderERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(
        http_message.obj, &expect_header, &expect_value);
    union std_string bundle_id_header =
        new_std_string("X-Apple-Requesting-Bundle-Id");
    union std_string bundle_id_value =
        new_std_string("com.apple.android.music");
    _ZN13mediaplatform11HTTPMessage9setHeaderERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(
        http_message.obj, &bundle_id_header, &bundle_id_value);
    union std_string bundle_version_header =
        new_std_string("X-Apple-Requesting-Bundle-Version");
    union std_string bundle_version_value = new_std_string(
        "Music/4.9 Android/10 model/Samsung S9 build/7663313 (dt:66)");
    _ZN13mediaplatform11HTTPMessage9setHeaderERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(
        http_message.obj, &bundle_version_header, &bundle_version_value);
    int body_size = snprintf(
        NULL, 0,
        "{\"guid\":\"%s\",\"assertion\":\"%s\","
        "\"tcc-acceptance-date\":\"%lld\"}",
        guid_value, auth_token_value, get_current_time_millis());
    if (body_size < 0) {
        return NULL;
    }

    char *body = malloc((size_t)body_size + 1);
    if (body == NULL) {
        return NULL;
    }

    snprintf(
        body, (size_t)body_size + 1,
        "{\"guid\":\"%s\",\"assertion\":\"%s\","
        "\"tcc-acceptance-date\":\"%lld\"}",
        guid_value, auth_token_value, get_current_time_millis());

    _ZN13mediaplatform11HTTPMessage11setBodyDataEPcm(
        http_message.obj, body, strlen(body));
    free(body);
    uint8_t url_request[512];
    _ZN17storeservicescore10URLRequestC2ERKNSt6__ndk110shared_ptrIN13mediaplatform11HTTPMessageEEERKNS2_INS_14RequestContextEEE(
        url_request, &http_message, &request_context_ref);
    _ZN17storeservicescore10URLRequest3runEv(url_request);
    struct shared_ptr *err =
        _ZNK17storeservicescore10URLRequest5errorEv(url_request);
    if (err->obj != NULL) {
        return NULL;
    }
    struct shared_ptr *url_response =
        _ZNK17storeservicescore10URLRequest8responseEv(url_request);
    struct shared_ptr *resp =
        _ZNK17storeservicescore11URLResponse18underlyingResponseEv(
            url_response->obj);
    void *http_message_obj = resp->obj;
    void **data_ptr_location = (void **)((char *)http_message_obj + 48);
    void *data_ptr = *data_ptr_location;
    char *response_body = _ZNK13mediaplatform4Data5bytesEv(data_ptr);
    return json_extract_string(response_body, "music_token");
}

static char *get_dev_token(struct shared_ptr request_context_ref) {
    uint8_t ptr[480];
    *(void **)(ptr) =
        &_ZTVNSt6__ndk120__shared_ptr_emplaceIN13mediaplatform11HTTPMessageENS_9allocatorIS2_EEEE +
        2;
    struct shared_ptr http_message = {.obj = ptr + 32, .ctrl_blk = ptr};
    union std_string url = new_std_string(
        "https://sf-api-token-service.itunes.apple.com/apiToken");
    union std_string method = new_std_string("GET");
    _ZN13mediaplatform11HTTPMessageC2ENSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES7_(
        http_message.obj, &url, &method);
    uint8_t url_request[512];
    _ZN17storeservicescore10URLRequestC2ERKNSt6__ndk110shared_ptrIN13mediaplatform11HTTPMessageEEERKNS2_INS_14RequestContextEEE(
        url_request, &http_message, &request_context_ref);
    union std_string client_id_name = new_std_string("clientId");
    union std_string client_id_value = new_std_string("musicAndroid");
    _ZN17storeservicescore10URLRequest19setRequestParameterERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(
        url_request, &client_id_name, &client_id_value);
    union std_string version_name = new_std_string("version");
    union std_string version_value = new_std_string("1");
    _ZN17storeservicescore10URLRequest19setRequestParameterERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(
        url_request, &version_name, &version_value);
    _ZN17storeservicescore10URLRequest3runEv(url_request);
    struct shared_ptr *err =
        _ZNK17storeservicescore10URLRequest5errorEv(url_request);
    if (err->obj != NULL) {
        return NULL;
    }
    struct shared_ptr *url_response =
        _ZNK17storeservicescore10URLRequest8responseEv(url_request);
    struct shared_ptr *resp =
        _ZNK17storeservicescore11URLResponse18underlyingResponseEv(
            url_response->obj);
    void *http_message_obj = resp->obj;
    void **data_ptr_location = (void **)((char *)http_message_obj + 48);
    void *data_ptr = *data_ptr_location;
    char *response_body = _ZNK13mediaplatform4Data5bytesEv(data_ptr);
    return json_extract_string(response_body, "token");
}

static void write_music_token(void) {
    char *path = join_path(options.data_dir, "/MUSIC_TOKEN");
    if (path == NULL) {
        return;
    }

    if (file_exists(path)) {
        FILE *fp = fopen(path, "r");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            if (ftell(fp) != 0) {
                fclose(fp);
                free(path);
                log_state(stdout, "music token", "kept");
                return;
            }
            fclose(fp);
        }
    }

    FILE *fp = fopen(path, "w");
    free(path);
    if (fp == NULL) {
        return;
    }

    log_state(stdout, "music token", "saved");
    fprintf(fp, "%s", nonnull_string(cached_music_token));
    fclose(fp);
}

static int offline_available(void) {
    struct shared_ptr *fairplay = malloc(16);
    if (fairplay == NULL) {
        return 0;
    }

    _ZN17storeservicescore14RequestContext8fairPlayEvASM(
        fairplay, request_context.obj);
    struct std_vector fairplay_status =
        _ZN17storeservicescore8FairPlay21getSubscriptionStatusEv(
            fairplay->obj);
    char *begin_ptr = (char *)fairplay_status.begin;
    char *second_item_ptr = begin_ptr + 16;
    int state = *(int *)((char *)second_item_ptr + 8);
    free(fairplay);
    if (state == 2 || state == 3) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int thread_error = 0;

    if (cli_options_parse(argc, argv, &options) != 0) {
        return EXIT_FAILURE;
    }
    int device_info_index = 0;
    char *device_info_saveptr = NULL;
    char *device_info =
        strtok_r(options.device_info, "/", &device_info_saveptr);
    while (device_info != NULL && device_info_index < DEVICE_INFO_FIELD_COUNT) {
        device_fields[device_info_index++] = device_info;
        device_info = strtok_r(NULL, "/", &device_info_saveptr);
    }
    if (device_info_index != DEVICE_INFO_FIELD_COUNT || device_info != NULL) {
        log_value(stderr, "invalid device info", "%s",
                  "expected 9 slash-separated fields");
        return EXIT_FAILURE;
    }

    if (options.login_given) {
        if (!parse_login_credentials(options.login)) {
            log_value(stderr, "invalid login", "%s",
                      "expected apple-id or apple-id:password");
            return EXIT_FAILURE;
        }
        if (login_password == NULL && options.password_file == NULL) {
            log_value(
                stderr, "invalid login", "%s",
                "password missing: pass apple-id:password or --password-file");
            return EXIT_FAILURE;
        }
    }

    initialize_wrapper();
    request_context = create_request_context();
    if (options.login_given && !login(request_context)) {
        log_state(stderr, "login", "failed");
        return EXIT_FAILURE;
    }
    _ZN22SVPlaybackLeaseManagerC2ERKNSt6__ndk18functionIFvRKiEEERKNS1_IFvRKNS0_10shared_ptrIN17storeservicescore19StoreErrorConditionEEEEEE(
        lease_manager, &lease_end_callback_fn,
        &playback_error_callback_fn);
    uint8_t lease_updates_enabled = 1;
    _ZN22SVPlaybackLeaseManager25refreshLeaseAutomaticallyERKb(
        lease_manager, &lease_updates_enabled);
    _ZN22SVPlaybackLeaseManager12requestLeaseERKb(
        lease_manager, &lease_updates_enabled);
    foot_hill_instance = get_foot_hill_instance();

    is_offline_available = offline_available();
    if (is_offline_available) {
        log_state(stdout, "offline", "enabled");
    }

    cached_storefront_id = get_account_storefront_id(request_context);
    cached_dev_token = get_dev_token(request_context);
    cached_music_token = get_music_user_token(get_guid(), cached_dev_token,
                                              request_context);
    log_state(stderr, "account", "ready");

    write_storefront_id();
    write_music_token();

    pthread_t m3u8_thread;
    thread_error = pthread_create(&m3u8_thread, NULL, &serve_m3u8, NULL);
    if (thread_error != 0) {
        output_thread_error("pthread_create m3u8", thread_error);
        return EXIT_FAILURE;
    }
    thread_error = pthread_detach(m3u8_thread);
    if (thread_error != 0) {
        output_thread_error("pthread_detach m3u8", thread_error);
        return EXIT_FAILURE;
    }

    pthread_t account_thread;
    thread_error =
        pthread_create(&account_thread, NULL, &serve_account, NULL);
    if (thread_error != 0) {
        output_thread_error("pthread_create account", thread_error);
        return EXIT_FAILURE;
    }
    thread_error = pthread_detach(account_thread);
    if (thread_error != 0) {
        output_thread_error("pthread_detach account", thread_error);
        return EXIT_FAILURE;
    }

    return serve_decrypt();
}
