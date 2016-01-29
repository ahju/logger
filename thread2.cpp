#include <stdio.h>
#include "logger.h"

extern Logger *mylog;

void *logholder2(void *)
{
	sleep(3);
	int i = 1;
	while(1)
	{
		char loginfo[64] = {0};
		switch(i%4)
		{
			case INFO:
				sprintf(loginfo, "thread2 is logging at %d", i++);
				mylog->logit_info(loginfo);
			case WARN:
				sprintf(loginfo, "thread2 is logging at %d", i++);
				mylog->logit_warn(loginfo);
			case ERROR:
				sprintf(loginfo, "thread2 is logging at %d", i++);
				mylog->logit_error(loginfo);
			default:
				sprintf(loginfo, "thread2 is logging at %d", i++);
				mylog->logit_debug(loginfo);
		}
		sleep(2);
	}
	return NULL;
}