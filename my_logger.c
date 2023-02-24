//
// Created by root on 12/20/22.
//

#include "my_logger.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define DUMP_SIGNAL SIGUSR1
#define OFF_LOGGING_SIGNAL SIGUSR2
#define LOGGING_LEVEL_SIGNAL SIGURG

static pthread_t dumpFileThread;

static pthread_mutex_t mutex_logging;

static char dumpFname[30];
static char logFile[30];

static int dumpsCounter = 0;

// typedef enum {
//    MIN,
//    STANDARD,
//    MAX
// } details_level_t;

_Atomic static volatile details_level_t currentDetailsLevel;
_Atomic static volatile int isRunning = LOGGING_OFF;

_Noreturn void *wait_for_dump_signal(__attribute__((unused)) void *ptr);
static sem_t dumpSemaphore;
static pthread_mutex_t dumpFileMutex;

// Install handlers //
// Start dump file thread //
// Take dump file infos //

static void setLevel(int sig, siginfo_t *si, void *ucontext) {
    currentDetailsLevel = si->si_value.sival_int;
}


int logger_init(const char *logFilename, const char *dumpFilename, details_level_t detailsLevel) {
    if (!logFilename || !dumpFilename)
        return 1;

    if (isRunning == LOGGING_ON)
        return 1;

    isRunning = LOGGING_ON;

    currentDetailsLevel = detailsLevel;

    strcpy(dumpFname, dumpFilename);
    strcpy(logFile, logFilename);
    fclose(fopen(strcat(logFile, ".log"), "wt"));

    sem_init(&dumpSemaphore, 1, 0);
    pthread_mutex_init(&dumpFileMutex, NULL);
    pthread_mutex_init(&mutex_logging, NULL);

    struct sigaction saDump;
    saDump.sa_flags = SA_RESTART;
    saDump.sa_handler = &dump;
    sigaction(DUMP_SIGNAL, &saDump, NULL);

    struct sigaction saLoggingOff;
    saLoggingOff.sa_flags = SA_RESTART;
    saLoggingOff.sa_handler = &switchLogging;
    sigaction(OFF_LOGGING_SIGNAL, &saLoggingOff, NULL);

    struct sigaction saSetLevel;
    sigemptyset(&saSetLevel.sa_mask);
    saSetLevel.sa_sigaction = &setLevel;
    saSetLevel.sa_flags = SA_SIGINFO;
    sigaction(LOGGING_LEVEL_SIGNAL, &saSetLevel, NULL);

    if (pthread_create(&dumpFileThread, NULL, wait_for_dump_signal, NULL)) {
        pthread_mutex_destroy(&dumpFileMutex);
        pthread_mutex_destroy(&mutex_logging);
        sem_destroy(&dumpSemaphore);

        signal(DUMP_SIGNAL, SIG_DFL);
        signal(OFF_LOGGING_SIGNAL, SIG_DFL);
        signal(LOGGING_LEVEL_SIGNAL, SIG_DFL);

        return 1;
    }
    // saDump.sa_sigaction = save_dump_file;

    return 0;
}

static void dump(int sig) {
    sem_post(&dumpSemaphore);
}

_Noreturn void *wait_for_dump_signal(__attribute__((unused)) void *ptr) {

    do {
        sem_wait(&dumpSemaphore);
        // kill(getpid(), DUMP_SIGNAL);
        pthread_mutex_lock(&dumpFileMutex);
        save_dump_file();
        pthread_mutex_unlock(&dumpFileMutex);
    } while (1);

}

void save_dump_file() {

    char *num = calloc(2, sizeof(char));
    snprintf(num, 2, "%d", dumpsCounter++);
    char fname[30] = "";
    strcpy(fname, dumpFname);
    strcat(fname, num);

    FILE *dFile = fopen(strcat(fname, ".dump"), "wt");

    fputs("Dump has occured", dFile);

    fclose(dFile);
    free(num);

    // printf("Dump has occured\n");
}

static void switchLogging(int sig) {
    // TODO -> getPrev
    currentDetailsLevel = !currentDetailsLevel;
}

void setLoggingLevel(details_level_t detailsLevel) {
    union sigval sv;
    sv.sival_int = detailsLevel;

    sigqueue(getpid(), LOGGING_LEVEL_SIGNAL, sv);
}


int save_log_message(details_level_t logDetailsLevel, const char *log_message_format, ...) {
    if (logDetailsLevel < currentDetailsLevel)
        return 1;

    va_list ap;
    va_start(ap, log_message_format);

    pthread_mutex_lock(&mutex_logging);

    FILE *fp = fopen(logFile, "at");
    vfprintf(fp, log_message_format, ap);
    fclose(fp);

    pthread_mutex_unlock(&mutex_logging);

    va_end(ap);

    return 0;
}

void checkLevel() {
    printf("%d\n", currentDetailsLevel);
}

void logger_destroy() {
    signal(DUMP_SIGNAL, SIG_DFL);
    signal(OFF_LOGGING_SIGNAL, SIG_DFL);
    signal(LOGGING_LEVEL_SIGNAL, SIG_DFL);

    pthread_mutex_lock(&dumpFileMutex);

    pthread_cancel(dumpFileThread);

    pthread_mutex_unlock(&dumpFileMutex);

    pthread_mutex_destroy(&dumpFileMutex);
    pthread_mutex_destroy(&mutex_logging);
    sem_destroy(&dumpSemaphore);
}
