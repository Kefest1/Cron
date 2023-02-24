# My cron implementation

This programme is a simple implementation of a Linux Cron utility.
https://en.wikipedia.org/wiki/Cron

Unlike the original it saves all scheduled tasks in RAM memory,
therefore it can work with nanosecond precision, however tasks are canceled at system shutdown.

It also has a possibility of saving logs (implemented in my_logger file).
Using programme with wrong arguments may result in undefined behavior
Tasks are being held in linked list.

Sample usage:
make
./my_cron -> starts a server (arguments are ignored)
After that all my_cron calls are clients:
./my_cron -a ~/my_dump 60 500000000 60 0 -> makes dump after 60.5 seconds, then repeats every 60 seconds
./my_cron -a /bin/touch 900 0 0 0 file.txt data.bin -> creates a file after 900 sec
./my_cron -l -> lists all tasks
./my_cron -d 1 -> cancels task having ID 1
./my_cron -n root/path 30 15 11 19 2 2023 10 0 arg1 arg2 arg3 
        -> creates a timer, that runs at 19/2/2023 at 11.15.30 and repeats every 10 sec 
           programme gets 3 arguments