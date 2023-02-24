//
// Created by root on 12/20/22.
//

#ifndef SCRPROJEKT1_MY_LOGGER_H
#define SCRPROJEKT1_MY_LOGGER_H

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <bits/types/siginfo_t.h>

#define LOGGING_ON 1
#define LOGGING_OFF 0

typedef enum {
    OFF,
    MIN,
    STANDARD,
    MAX
} details_level_t;

static void switchLogging(int sig);
int logger_init(const char *logFilename, const char *dumpFilename, details_level_t detailsLevel);

void save_dump_file();

                                                      // Printf-like //
int save_log_message(details_level_t logDetailsLevel, const char *log_message_format, ...);
// Should have a mutex for multithreading applications //
// Two signals -> to set details_level and turn logging off (atomic global variable) //

void save_dump();
void wait_for_signal(void *ptr);
// Signal handler to save dump file //

void logger_destroy();
// Destroy semaphores, stop threads, close files, close signal handlers, deallocate memory //

static void dump(int sig);
void saveDumpDebug();
static void setLevel(int sig, siginfo_t *si, void *ucontext);
void setLoggingLevel(details_level_t detailsLevel);
void checkLevel();

#endif //SCRPROJEKT1_MY_LOGGER_H
