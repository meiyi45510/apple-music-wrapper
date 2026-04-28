#include <cstdint>
#include <cstdio>
#include <exception>
#include <functional>

#include "logger.h"

extern "C" void handle_decrypt_session(int fd);

extern "C" uint8_t handle_decrypt_guarded(int fd) {
    try {
        handle_decrypt_session(fd);
        return 1;
    } catch (const std::exception &e) {
        log_value(stderr, "exception", "%s", e.what());
        return 0;
    }
}

static void lease_end_callback(const int &code) {
    log_value(stderr, "lease ended", "%d", code);
}

static void playback_error_callback(void *) {
    log_state(stderr, "playback", "error");
}

extern "C" std::function<void(const int &)> lease_end_callback_fn(
    lease_end_callback);
extern "C" std::function<void(void *)> playback_error_callback_fn(
    playback_error_callback);
