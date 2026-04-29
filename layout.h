#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct shared_ptr {
    void *obj;
    void *ctrl_blk;
};

union std_string {
    struct {
        uint8_t mark;
        char str[0];
    };
    struct {
        size_t cap;
        size_t size;
        const char *data;
    };
};

struct std_vector {
    void *begin;
    void *end;
    void *end_capacity;
};

static inline size_t std_vector_size(const struct std_vector *vector,
                                     size_t element_size) {
    if (vector == NULL || vector->begin == NULL || vector->end == NULL ||
        element_size == 0) {
        return 0;
    }

    uintptr_t begin = (uintptr_t)vector->begin;
    uintptr_t end = (uintptr_t)vector->end;
    if (end < begin) {
        return 0;
    }

    return (size_t)((end - begin) / element_size);
}

static inline union std_string new_std_string(const char *s) {
    union std_string str = {
        .cap = 1,
        .size = strlen(s),
        .data = s,
    };
    return str;
}

static inline struct std_vector new_std_vector(void *begin, size_t element_size) {
    void *end =
        begin == NULL ? NULL : (void *)((char *)begin + element_size);
    struct std_vector vector = {
        .begin = begin,
        .end = end,
    };
    vector.end_capacity = vector.end;
    return vector;
}

static inline union std_string new_std_string_short_mode(const char *str) {
    short str_size = (short)strlen(str);
    union std_string std_str = {
        .mark = str_size << 1,
    };
    strcpy(std_str.str, str);
    return std_str;
}

static inline const char *std_string_data(union std_string *str) {
    if ((str->mark & 1) == 0) {
        return str->str;
    }
    return str->data;
}
