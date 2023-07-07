#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static char error_message[256] = {0};

static const char *test_error_message(const char *message, va_list args) {
    vsnprintf(error_message, sizeof(error_message) / sizeof(error_message[0]), message, args);
    return error_message;
}

static void check_condition(bool condition, const char *function, int32_t line, const char *message, ...) {
    if (condition == 0) {
        va_list args;
        va_start(args, message);
        fprintf(stderr, "%s:%s():at_line %d: %s\n", __FILE__, function, line, test_error_message(message, args));
        va_end(args);
        exit(1);
    }
}