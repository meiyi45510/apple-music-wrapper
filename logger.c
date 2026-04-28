#include "logger.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>

static const char *nonnull_string(const char *value) {
    return value == NULL ? "" : value;
}

static void print_subject_prefix(FILE *stream, const char *subject) {
    const char *value = nonnull_string(subject);

    if (value[0] != '\0') {
        fprintf(stream, "%s: ", value);
    }
}

void log_state(FILE *stream, const char *subject, const char *state) {
    print_subject_prefix(stream, subject);
    fprintf(stream, "%s\n", nonnull_string(state));
}

void log_value(FILE *stream, const char *subject,
               const char *value_format, ...) {
    va_list args;

    print_subject_prefix(stream, subject);

    va_start(args, value_format);
    vfprintf(stream, nonnull_string(value_format), args);
    va_end(args);

    fputc('\n', stream);
}

void log_errno(const char *operation) {
    const int saved_errno = errno;

    log_value(stderr, operation, "%s", strerror(saved_errno));
}

void log_prompt(const char *label) {
    print_subject_prefix(stdout, label);
    fflush(stdout);
}
