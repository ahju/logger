#include <stdio.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include "time.h"
#include "logger.h"
using namespace std;

Logger *mylog;

extern void *logholder1(void *);
extern void *logholder2(void *);

main()
{
	mylog = Logger::get_instance();
	mylog->logit_info("main func use logger");
	
	pthread_t thread1_pid;
	if(pthread_create(&thread1_pid,NULL,logholder1,NULL))
	{
		mylog->logit_info("pthread_create():"+string(strerror(errno)));
		exit(1);
	}
	
	pthread_t thread2_pid;
	if(pthread_create(&thread2_pid,NULL,logholder2,NULL))
	{
		mylog->logit_info("pthread_create():"+string(strerror(errno)));
		exit(1);
	}

	int i = 1;
	while(1)
	{
		char loginfo[64] = {0};
		switch(i%4)
		{
			case INFO:
				sprintf(loginfo, "main thread is logging at %d", i++);
				mylog->logit_info(loginfo);
			case WARN:
				sprintf(loginfo, "main thread is logging at %d", i++);
				mylog->logit_warn(loginfo);
			case ERROR:
				sprintf(loginfo, "main thread is logging at %d", i++);
				mylog->logit_error(loginfo);
			default:
				sprintf(loginfo, "main thread is logging at %d", i++);
				mylog->logit_debug(loginfo);
		}
		sleep(2);
	}
}
