//
// Created by root on 2/17/23.
//

#ifndef SCRSY2_TASK_LIST_H
#define SCRSY2_TASK_LIST_H

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

struct node_t {
    int idTimer;
    timer_t timer_id;
    struct node_t *next;

    char exePath[128];
    char isCyclic;
};

struct list_task_t {
    int size;
    struct node_t *head;
};

struct list_task_t *listCreate(void);

void displayListDebug(const struct list_task_t *list);

void listAdd(struct list_task_t *list, timer_t timer, int timerID, const char *task, char isCyclic);

struct node_t *getByTimerID(const struct list_task_t *list, int timerID);

int removeTask(struct list_task_t *list, int timer, char removeCyclic);

void listClearNodes(struct list_task_t *list);

#endif //SCRSY2_TASK_LIST_H
