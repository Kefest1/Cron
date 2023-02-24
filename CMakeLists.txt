cmake_minimum_required(VERSION 3.24)
project(SCRSY2 C)

set(CMAKE_C_STANDARD 11)

add_executable(SCRSY2 main.c my_logger.c server.c server.h client.c client.h my_task.h task_list.c task_list.h task_list.h my_cron.c my_cron.h)
