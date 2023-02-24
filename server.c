//
// Created by root on 2/16/23.
//
#include "my_logger.h"
#include "server.h"
#include "my_task.h"
#include "task_list.h"
#include <pthread.h>
#include <spawn.h>
static mqd_t mqd;
static mqd_t broadcastSize;
static mqd_t broadcastList;

static struct list_task_t *list;
pthread_mutex_t listSafetyMutex;

static int taskID = 0;

void *taskToDo(void *arg) {
    struct my_task_t *task = (struct my_task_t*) arg;

    pid_t child_pid;

    char *args[128];
    args[0] = task->path;
    for (int i = 0; i < task->argc; i++)
        args[i + 1] = task->args[i];

    args[task->argc + 1] = NULL;

    posix_spawn(&child_pid, args[0], NULL, NULL, args, NULL);

    pthread_mutex_lock(&listSafetyMutex);
    removeTask(list, task->id, 0);
    pthread_mutex_unlock(&listSafetyMutex);

    return NULL;
}



void *waitForTask(void *arg) {

    struct my_task_t task;
    inf_loop:

    puts("Server waiting...");
    mq_receive(mqd, (char *) &task, sizeof(struct my_task_t), NULL);

    if (task.toDelete >= 0) {
        save_log_message(STANDARD, "User deleted task");
        puts("Deleting");
        pthread_mutex_lock(&listSafetyMutex);
        removeTask(list, task.toDelete, 1);
        pthread_mutex_unlock(&listSafetyMutex);
        goto inf_loop;
    }
    else if (task.toDelete == -2) {
        // List
        save_log_message(MIN, "User listed task");
        pthread_mutex_lock(&listSafetyMutex);
        int size = list->size;
        mq_send(broadcastSize, (const char *) &size, sizeof(int), 0);

        struct node_t node;
        struct node_t *buf = list->head;
        for (int i = 0; i < size; i++) {
            node.idTimer = buf->idTimer;
            strcpy(node.exePath, buf->exePath);
            node.timer_id = buf->timer_id;
            node.isCyclic = buf->isCyclic;
            node.next = NULL;
            buf = buf->next;

            mq_send(broadcastList, (const char *) &node, sizeof(struct node_t), 0);
        }

        displayListDebug(list);
        pthread_mutex_unlock(&listSafetyMutex);
        goto inf_loop;
    }
    else if (task.toDelete == -3) {
        save_log_message(MIN, "User ordered termination");
        printf("Finalising...\n");
        finaliseServer();
        return 0;
    }


    struct itimerspec timer_time;
    timer_t timer_id;
    struct sigevent timer_event;

    // TODO puts(task.args);
    timer_event.sigev_notify = SIGEV_THREAD;
    timer_event.sigev_notify_function = (void (*)(__sigval_t)) taskToDo;

    printf("Task.path %s:\n", task.path);
    printf("Got %d args:\n", task.argc);

    for (int i = 0; i < task.argc; i++)
        puts(task.args[i]);

    task.id = taskID;

    timer_event.sigev_value.sival_ptr = &task;
    timer_event.sigev_notify_attributes = NULL;

    timer_create(CLOCK_REALTIME, &timer_event, &timer_id);

    timer_time.it_value.tv_sec = task.timer_time.it_value.tv_sec;
    timer_time.it_value.tv_nsec = task.timer_time.it_value.tv_nsec;
    timer_time.it_interval.tv_sec = task.timer_time.it_interval.tv_sec;
    timer_time.it_interval.tv_nsec = task.timer_time.it_interval.tv_nsec;

    char isCyclic = 1;
    if (task.timer_time.it_interval.tv_sec == 0 && task.timer_time.it_interval.tv_nsec == 0)
        isCyclic = 0;


    pthread_mutex_lock(&listSafetyMutex);
    listAdd(list, timer_id, taskID, task.path, isCyclic);
    taskID++;
    pthread_mutex_unlock(&listSafetyMutex);

    timer_settime(timer_id, 0, &timer_time, NULL);

    goto inf_loop;
}

#include <sys/ipc.h>
#include <sys/shm.h>

void startServer() {
    logger_init(LOG_FILENAME, "DUMP", STANDARD);

    mq_unlink(BROADCAST_SIZE_QUEUE_NAME);
    mq_unlink(MQ_NAME);
    mq_unlink(BROADCAST_LIST_QUEUE_NAME);

    int sharedBlockId = shmget(ftok(FILE_CONNECTOR, 0), sizeof(char), IPC_CREAT);
    shmat(sharedBlockId, NULL, 0);

    pthread_mutex_init(&listSafetyMutex, NULL);

    list = listCreate();
    struct mq_attr attr;

    attr.mq_maxmsg = 10;  // TODO
    attr.mq_msgsize = sizeof(struct my_task_t);
    attr.mq_flags = 0;

    puts("Opening...");
    mqd = mq_open(MQ_NAME, O_RDONLY | O_CREAT, 0644, &attr);

    if (mqd == (mqd_t) -1) {
        save_log_message(MAX, "Failed to create queue\n");
    }

    struct mq_attr attr2;

    attr2.mq_maxmsg = 10;  // TODO
    attr2.mq_msgsize = sizeof(int);
    attr2.mq_flags = 0;

    broadcastSize = mq_open(BROADCAST_SIZE_QUEUE_NAME, O_WRONLY | O_CREAT, 0644, &attr2);

    if (broadcastSize == (mqd_t) -1) {
        save_log_message(MAX, "Failed to create queue\n");
        return;
    }
    puts("Server opened a msg");


    struct mq_attr attr3;

    attr3.mq_maxmsg = 10;  // TODO
    attr3.mq_msgsize = sizeof(struct node_t);
    attr3.mq_flags = 0;

    broadcastList = mq_open(BROADCAST_LIST_QUEUE_NAME, O_WRONLY | O_CREAT, 0644, &attr3);


    if (broadcastList == (mqd_t) -1) {
        save_log_message(MAX, "Failed to create queue\n");
    }

    save_log_message(STANDARD, "User started server\n");

    waitForTask(NULL);
}

void finaliseServer(void) {
    mq_close(mqd);
    mq_close(broadcastSize);
    mq_close(broadcastList);
    save_log_message(STANDARD, "User terminated server\n");

    struct node_t *node = list->head;

    pthread_mutex_lock(&listSafetyMutex);

    while (node) {
        timer_delete(node->timer_id);
        node = node->next;
    }

    listClearNodes(list);
    free(list);

    pthread_mutex_unlock(&listSafetyMutex);

    pthread_mutex_destroy(&listSafetyMutex);

    shmctl(shmget(ftok(FILE_CONNECTOR, 0), sizeof(char), 0), IPC_RMID, NULL);

    puts("Server stopped. All tasks have been canceled");

    logger_destroy();
}
