//
// Created by root on 2/17/23.
//

#include "task_list.h"


struct list_task_t *listCreate(void) {
    return calloc(1, sizeof(struct list_task_t));
}

void listAdd(struct list_task_t *list, timer_t timer, int timerID, const char *task, char isCyclic) {
    if (!list->size) {
        list->head = calloc(1, sizeof(struct node_t));
        strcpy(list->head->exePath, task);
        list->head->timer_id = timer;
        list->head->idTimer = timerID;
        list->head->isCyclic = isCyclic;
    }
    else {
        struct node_t *ptr = list->head;

        while (ptr->next)
            ptr = ptr->next;

        ptr->next = calloc(1, sizeof(struct node_t));
        strcpy(ptr->next->exePath, task);
        ptr->next->timer_id = timer;
        ptr->next->idTimer = timerID;
        ptr->next->isCyclic = isCyclic;
    }

    list->size++;
}

struct node_t *getByTimerID(const struct list_task_t *list, int timerID) {
    if (list->size == 0)
        return NULL;

    struct node_t *ret = list->head;

    while (ret) {
        if (ret->idTimer == timerID)
            return ret;
        ret = ret->next;
    }

    return ret;
}

int removeTask(struct list_task_t *list, int timer, char removeCyclic) {
    struct node_t *taskToRemove = getByTimerID(list, timer);

    if (!taskToRemove)
        return 1;

    if (removeCyclic == 0 && taskToRemove->isCyclic == 1)
        return 2;


    timer_delete(taskToRemove->timer_id);

    if (list->size == 1) {
        list->head = NULL;
        list->size = 0;
        return 0;
    }

    struct node_t *ret = list->head;

    while (ret->next) {
        if (ret->next->idTimer == timer)
            break;
        ret = ret->next;
    }

    if (!ret->next) {
        list->head = list->head->next;
        list->size--;
    }
    else {
        ret->next = taskToRemove->next;
        free(taskToRemove);
        list->size--;
    }

    return 0;
}

void displayListDebug(const struct list_task_t *list) {
    if (!list)
        puts("No tasks. List is empty");
    else {
        struct node_t *buffer = list->head;
        while (buffer) {
            printf("%d\n", buffer->idTimer);
            puts(buffer->exePath);
            buffer = buffer->next;
        }
        putchar(10);
    }
}

void listClearNodes(struct list_task_t *list) {
    if (!list || !list->head) return;
    if (list->head->next == NULL) {
        free(list->head);
        return;
    }

    struct node_t *ptr = list->head;

    while (list->head->next) {
        ptr = list->head;
        while (ptr->next->next)
            ptr = ptr->next;

        free(ptr->next);
        ptr->next = NULL;
    }

    free(list->head);

}
