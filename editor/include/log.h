#ifndef LOG_H
#define LOG_H
#define LOG_DEBUG(...) fprintf(stdout, "[DEBUG]: "); fprintf(stdout, __VA_ARGS__); putchar('\n');
#define LOG_EVENT(...) fprintf(stdout, "[EVENT]: "); fprintf(stdout, __VA_ARGS__); putchar('\n');
#define LOG_ERROR(...) fprintf(stderr, "[ERROR]: "); fprintf(stderr,  __VA_ARGS__); putchar('\n');
#define LOG_FETAL(...) fprintf(stderr, "[FETAL_ERROR]: "); fprintf(stderr,  __VA_ARGS__); putchar('\n');
#endif
