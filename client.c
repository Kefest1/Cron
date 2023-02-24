//
// Created by root on 2/16/23.
//

#include "client.h"
#include "my_logger.h"
#include "task_list.h"

static mqd_t mqd;

void startClient(int argc, char *argv[]) {

    struct mq_attr attr;

    attr.mq_maxmsg = 10;  // TODO
    attr.mq_msgsize = sizeof(struct my_task_t);
    attr.mq_flags = 0;

    mqd = mq_open(MQ_NAME, O_WRONLY);

    broadcastTask(argc, argv);

    finaliseClient(mqd);
}

int broadcastTask(int argc, char *argv[]) {
    char taskCode = *(*(argv + 1) + 1);

    enum task_type_t taskType;
    if (taskCode == 'r')      taskType = REMOVE;
    else if (taskCode == 'l') taskType = BROADCAST_TASK;
    else if (taskCode == 'a') taskType = ABSOLUTE;
    else if (taskCode == 'n') taskType = NOT_ABSOLUTE;
    else if (taskCode == 'q') taskType = FINALIZE;
    else                      return 2;


    struct my_task_t *task = calloc(1, sizeof(struct my_task_t));

    if (taskType == REMOVE) {
        int toDelete = atoi(argv[2]);
        task->toDelete = toDelete;
        mq_send(mqd, (const char *) task, sizeof(struct my_task_t), 0);
    }
    else if (taskType == BROADCAST_TASK) {
        task->toDelete = -2;
        mq_send(mqd, (const char *) task, sizeof(struct my_task_t), 0);

        struct mq_attr attr;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = 1024 * sizeof(struct node_t);
        attr.mq_flags = 0;

        struct list_task_t taskList;
        mqd_t mqd1 = mq_open(BROADCAST_SIZE_QUEUE_NAME, O_RDONLY);
        int size;
        puts("Waiting...");
        mq_receive(mqd1, (char *) &size, sizeof(int), NULL);
        if (size)
            printf("Currently %d tasks waiting\n-----------------------", size);
        else printf("No tasks waiting");
        mqd_t mqd2 = mq_open(BROADCAST_LIST_QUEUE_NAME, O_RDONLY);

        struct node_t buffer;
        for (int i = 0; i < size; i++) {
            mq_receive(mqd2, (char *) &buffer, sizeof(struct node_t), NULL);
            printf("Task with ID %d\n", buffer.idTimer);
            puts(buffer.exePath);
            printf("%d\n", buffer.isCyclic);
            puts("-----------------------");
        }

        mq_close(mqd1);
        mq_close(mqd2);
    }
    else if (taskType == ABSOLUTE) {

        long times[4];
        for (int i = 0; i < 4; i++)
            times[i] = atoi(argv[3 + i]);

        struct itimerspec timer_time;

        timer_time.it_value.tv_sec = times[0];
        timer_time.it_value.tv_nsec = times[1];
        timer_time.it_interval.tv_sec = times[2];
        timer_time.it_interval.tv_nsec = times[3];

        strcpy(task->path, argv[2]);

        int argCount = argc - 7;

        task->argc = argCount;
        for (int i = 0; i < argCount; i++)
            strcpy(*(task->args + i), argv[i + 7]);


        task->timer_time = timer_time;
        task->toDelete = -1;
        mq_send(mqd, (const char *) task, sizeof(struct my_task_t), 0);
    }
    else if (taskType == NOT_ABSOLUTE) {
        struct tm* local_time;
        struct tm* userTime = calloc(1, sizeof(struct tm));
        time_t current_time = time(NULL);
        local_time = localtime(&current_time);

        userTime->tm_sec = atoi(argv[3]);
        userTime->tm_min = atoi(argv[4]);
        userTime->tm_hour = atoi(argv[5]);
        userTime->tm_mday = atoi(argv[6]);
        userTime->tm_mon = atoi(argv[7]) - 1;
        userTime->tm_year = atoi(argv[8]) - 1900;

        struct itimerspec timer_time;

        double diff_seconds = difftime(mktime(userTime), mktime(local_time));


        timer_time.it_value.tv_sec = (__time_t) diff_seconds;
        timer_time.it_value.tv_nsec = (__time_t) 0;
        timer_time.it_interval.tv_sec = atol(argv[9]);
        timer_time.it_interval.tv_nsec = atol(argv[10]);

        strcpy(task->path, argv[2]);
        int argCount = argc - 11;

        task->argc = argCount;
        for (int i = 0; i < argCount; i++)
            strcpy(*(task->args + i), argv[i + 11]);
        for (int i = 0; i < argCount; i++)
            printf("Arg no. %d: %s\n", i, argv[i + 11]);


        task->timer_time = timer_time;
        task->toDelete = -1;

        mq_send(mqd, (const char *) task, sizeof(struct my_task_t), 0);
    }
    else {
        task->toDelete = -3;
        mq_send(mqd, (const char *) task, sizeof(struct my_task_t), 0);
    }

    return 0;
}


void finaliseClient(mqd_t mq) {
    mq_close(mq);
}
