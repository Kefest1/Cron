my_cron: main.c
	gcc -o my_cron -lrt -lpthread task_list.c main.c my_cron.c server.c my_logger.c client.c
