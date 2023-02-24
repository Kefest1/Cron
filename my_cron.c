//
// Created by root on 2/24/23.
//


#include "server.h"
#include "client.h"

#include "my_cron.h"

int cron(int argc, char *argv[]) {

    int sharedBlockId = shmget(ftok(FILE_CONNECTOR, 0), sizeof(char), 0);
    int hasStarted = (errno == ENOENT || sharedBlockId < 0);

    if (hasStarted) {
        puts("Starting server");
        startServer();
    }
    else {
        if (argc < 2)
            return printf("Not enough parameters!\n"), 1;

        puts("Starting client");
        startClient(argc, argv);
    }

    return 0;
}
