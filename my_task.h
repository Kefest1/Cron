//
// Created by root on 2/16/23.
//

#ifndef SCRSY2_MY_TASK_T_H
#define SCRSY2_MY_TASK_T_H

#include <stdio.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdatomic.h>

#define MQ_NAME "/mq_task"
#define BROADCAST_SIZE_QUEUE_NAME "/mq_broadcast"
#define BROADCAST_LIST_QUEUE_NAME "/mq_broadcast_list"
#define MAXIMAL_COMMAND_LEN 200

#define FILE_CONNECTOR "main.c"
#define LOG_FILENAME "log"

enum task_type_t {
    REMOVE = 0,
    ABSOLUTE,
    NOT_ABSOLUTE,
    BROADCAST_TASK,
    FINALIZE
};

struct my_task_t {
    char path[MAXIMAL_COMMAND_LEN];
    char args[128][16];
    int argc;

    int id;

    int toDelete;
    struct itimerspec timer_time;
};


#endif //SCRSY2_MY_TASK_T_H
