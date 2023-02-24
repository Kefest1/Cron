//
// Created by root on 2/16/23.
//

#ifndef SCRSY2_CLIENT_H
#define SCRSY2_CLIENT_H

#include "my_task.h"

void startClient(int argc, char *argv[]);
void finaliseClient(mqd_t mq);
int getCommand(void);
int broadcastTask(int argc, char *argv[]);

#endif //SCRSY2_CLIENT_H
