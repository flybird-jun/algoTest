#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "log.h"
#include "algo_typde_def.h"

#define LOG_FILE_PREFIX "algo_log_"
#define LOG_FILE_MAX_NAME_LEN 64
int log_fd = -1;
void LogInit()
{
    const char *logDir = "./log/";
    struct stat st;
    if (stat(logDir, &st) != 0) {
       int status = mkdir(logDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
       if (status < 0) {
            printf("create log dir error\n");
            return;
       }
    }
    char filePath[LOG_FILE_MAX_NAME_LEN] = {0};
    strcat(filePath, logDir);
    char *fileName = filePath + strlen(filePath);
    uint8_t i = 0;
    for (i = 0; i < 16; i++) {
        memset(fileName, 0, LOG_FILE_MAX_NAME_LEN);
        (void)sprintf(fileName, "%s%u.txt", LOG_FILE_PREFIX, i);
        if (stat(filePath, &st) == -1) {
            if (errno == ENOENT) {
                break;
            }
        }

    }
    if (i == 16) {
        // 文件都存在，使用文件1
        (void)sprintf(fileName, "%s%u.txt", LOG_FILE_PREFIX, 1);
    }
    log_fd = open(filePath, O_WRONLY|O_APPEND|O_CREAT|O_SYNC, 0666);
    if (log_fd < 0) {
        printf("log system init wrong\n");
    }
}

#define LOG_LINE_BUF 4096
void LogPut(char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[LOG_LINE_BUF] = {0};
    time_t curTime = time(NULL);
    struct tm *local = localtime(&curTime);
    strftime(buf, LOG_LINE_BUF, "%Y-%m-%d %H:%M:%S ", local);
    int err = vsnprintf(buf + strlen(buf), LOG_LINE_BUF, format, ap);
    if (err < 0) {
        LogPut("log line is too long");
        return;
    }
    va_end(ap);
    strcat(buf, "\n");
    err = write(log_fd, buf, strlen(buf));
    if (err < 0) {
        printf("printf log error\n");
        return;
    }
}

void LogClose()
{
    if (log_fd > 0) {
        close(log_fd);
    }
}