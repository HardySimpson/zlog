/*
 * This file is part of the zlog Library.
 *
 * Copyright (C) 2017 by Philippe Corbes <philippe.corbes@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING in base directory.
 *
 * This test programm start NB_THREADS threads.
 * Each thread loop and log an Info message every THREAD_LOOP_DELAY us (=10ms).
 * The main loop check configuration file modification every seconds and reload configuration on file update.
 * The main loop force reload configuration every RELOAD_DELAY " (=10").
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "zlog.h"

#define CONFIG            "test_multithread.conf"
#define NB_THREADS        200
#define THREAD_LOOP_DELAY 10000	/* 0.01" */
#define RELOAD_DELAY      10

struct thread_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;    /* ID returned by pthread_create() */
	int       thread_num;   /* Application-defined thread # */
	zlog_category_t *zc;    /* The logger category struc address; (All threads will use the same category, so he same address) */
	long long int loop;     /* Counter incremented to check the thread's health */
};

struct thread_info *tinfo;


void intercept(int sig)
{
	int i;

    printf("\nIntercept signal %d\n\n", sig);

    signal (sig, SIG_DFL);
    raise (sig);

	printf("You can import datas below in a spreadsheat and check if any thread stopped increment the Loop counter during the test.\n\n");
	printf("Thread;Loop\n");
	for (i=0; i<NB_THREADS; i++)
	{
		printf("%d;%lld\n", tinfo[i].thread_num, tinfo[i].loop);
    }

    free(tinfo);

    zlog_fini();
}

void *myThread(void *arg)
{
    struct thread_info *tinfo = arg;

    tinfo->zc = zlog_get_category("thread");
	if (!tinfo->zc) {
		printf("get thread %d cat fail\n", tinfo->thread_num);
	}
	else
	{
		while(1)
		{
			usleep(THREAD_LOOP_DELAY);
			zlog_info(tinfo->zc, "%d;%lld", tinfo->thread_num, tinfo->loop++);
		}
	}

	return NULL;
}

int main(int argc, char** argv)
{
	int rc;
	zlog_category_t *zc;
	int i = 0;
	struct stat stat_0, stat_1;

	/* Create the logging directory if not yet ceated */
	mkdir("./test_multithread-logs", 0777);

	if (stat(CONFIG, &stat_0))
	{
		printf("Configuration file not found\n");
		return -1;
	}

	rc = zlog_init(CONFIG);
	if (rc) {
		printf("main init failed\n");
		return -2;
	}

	zc = zlog_get_category("main");
	if (!zc) {
		printf("main get cat fail\n");
		zlog_fini();
		return -3;
	}

	/* Interrupt (ANSI).		<Ctrl-C> */
	if (signal(SIGINT, intercept) == SIG_IGN )
	{
		zlog_fatal(zc, "Can't caught the signal SIGINT, Interrupt (ANSI)");
		signal(SIGINT, SIG_IGN );
		return -4;
	}

	// start threads
    tinfo = calloc(NB_THREADS, sizeof(struct thread_info));
	for (i=0; i<NB_THREADS; i++)
	{
        tinfo[i].thread_num = i + 1;
        tinfo[i].loop = 0;
		if(pthread_create(&tinfo[i].thread_id, NULL, myThread, &tinfo[i]) != 0)
		{
			zlog_fatal(zc, "Unable to start thread %d", i);
			zlog_fini();
			return(-5);
		}
    }

	/* Wait and log thread informations */
	sleep(1);
	for (i=0; i<NB_THREADS; i++)
	{
		zlog_info(zc, "Thread [%d], zlog_category:@%p", tinfo[i].thread_num, tinfo[i].zc);
    }

	/* Log main loop status */
	i=0;
	while(1)
	{
		int reload;

		sleep(1);
		i++;
		zlog_info(zc, "Running time: %02d:%02d:%02d", i/3600, (i/60)%60, i%60);

		/* Check configuration file update */
		stat(CONFIG, &stat_1);

		/* Is configuration file modified */
		reload = (stat_0.st_mtime != stat_1.st_mtime);

		/* Or do we want to reload periodicaly the configuration file */
		if ( ! reload)
			if ( RELOAD_DELAY > 0)
				reload = (i % RELOAD_DELAY == 0);

		if (reload)
		{
			zlog_info(zc, "Will reload configuration...");
			rc = zlog_reload(CONFIG);
			if (rc) {
				printf("main init failed\n");
				return -6;
			}
			zlog_info(zc, "Configuration reloaded :)");
			stat(CONFIG, &stat_0);
		}
	}
	
    exit(EXIT_SUCCESS);
}
