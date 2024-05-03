#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

void LogInit();
void LogPut(char *format, ...);
void LogClose();

#define LOG_PUT(...) LogPut(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif