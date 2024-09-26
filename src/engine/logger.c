#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define GET_TIME() ({\
        time_t timer;\
        time(&timer);\
        localtime(&timer);})


void log_msg(log_level level, char *fmt, ...) {
    const char *level_text[] = {"[FETAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};  
    const char *color_code[] = {"\x1B[91m","\x1B[31m","\x1B[33m","\x1B[32m","\x1B[36m","\x1B[94m"};
    struct tm *tm = GET_TIME();

    __builtin_va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL,0,fmt,args);
    char *msg = malloc(len+1);
    va_end(args);

    va_start(args, fmt);
    vsnprintf(msg,len+1,fmt,args);
    va_end(args);

    fprintf(stdout,"%s[%02d-%02d-%02d] %s%s\n", color_code[level],tm->tm_hour,tm->tm_min,tm->tm_sec,level_text[level],msg);
    free(msg);
}

void assetion_failure(const char* expression, const char *msg, const char *file, int line) {
    log_msg(LOG_LEVEL_FETAL, "Assertion expression: %s, Message: %s, File: %s, Line: %d", expression, msg, file, line);
}
