#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define PATH "../arknoah.log"
#define MAX_LOG_LENGTH 100
//#define NO_FILE

int log_fd;
char log_buf[MAX_LOG_LENGTH];

void log_init()
{
    if(access(PATH,F_OK)<0)
        log_fd = open(PATH,O_WRONLY|O_CREAT,0666);
    else
        log_fd = open(PATH,O_WRONLY|O_APPEND);
    if(log_fd<0)
    {
        printf("log file open failed\n");
        exit(1);
    }
#ifdef NO_FILE
    dup2(log_fd,STDOUT_FILENO);
#endif
    printf("%s\n", "==================\nLog Start\n==================");
}

void log(const char* info)
{
    time_t timet = time(NULL);
    struct tm* ptm = localtime(&timet);
    memset(log_buf,0,MAX_LOG_LENGTH);
    snprintf(log_buf,MAX_LOG_LENGTH,"Log:%04d.%02d.%02d %02d:%02d:%02d %s\n",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,info);

    write(STDOUT_FILENO,log_buf,100);
    fsync(STDOUT_FILENO);
}

void log_error(const char* info)
{
    time_t timet = time(NULL);
    struct tm* ptm = localtime(&timet);
    memset(log_buf,0,MAX_LOG_LENGTH);
    snprintf(log_buf,MAX_LOG_LENGTH,"Error:%04d.%02d.%02d %02d:%02d:%02d %s\n",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,info);

    write(STDOUT_FILENO,log_buf,100);
    fsync(STDOUT_FILENO);
}

void log_error(const char* info,const char* file,int line)
{
    time_t timet = time(NULL);
    struct tm* ptm = localtime(&timet);
    memset(log_buf,0,MAX_LOG_LENGTH);
    snprintf(log_buf,MAX_LOG_LENGTH,"Error:%04d.%02d.%02d %02d:%02d:%02d file:%s line:%d %s\n",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,file,line,info);

    write(STDOUT_FILENO,log_buf,100);
    fsync(STDOUT_FILENO);
}

void log_error(const char* info,const char* file,int line,size_t in)
{
    time_t timet = time(NULL);
    struct tm* ptm = localtime(&timet);
    memset(log_buf,0,MAX_LOG_LENGTH);
    snprintf(log_buf,MAX_LOG_LENGTH,"Error:%04d.%02d.%02d %02d:%02d:%02d file:%s line:%d %s%ld\n",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,file,line,info,in);

    write(STDOUT_FILENO,log_buf,strlen(log_buf));
    fsync(STDOUT_FILENO);
}
#endif
