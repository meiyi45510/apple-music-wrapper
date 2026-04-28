#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_state(FILE *stream, const char *subject, const char *state);
void log_value(FILE *stream, const char *subject,
               const char *value_format, ...);
void log_errno(const char *operation);
void log_prompt(const char *label);

#ifdef __cplusplus
}
#endif
