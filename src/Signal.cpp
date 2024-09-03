/*************************************************************************
    > File Name: src/Signal.cpp
    > Author: ARTC
    > Descripttion:
    > Created Time: 2024-04-08
 ************************************************************************/

#include <signal.h>
#include "Log_Message.h"

void exit_program(void)
{
	sleep(2);
	exit(0);
}

static void handle_sigint(int sig)
{
	switch(sig){
		case 2:
			log_message(INFO, "Captured SIGINT signal !!!");
			break;
		case 11:
			log_message(INFO, "Captured SIGSEGV signal !!!");
			break;
		case 14:
			log_message(INFO, "Captured SIGALRM signal !!!");
			break;
		case 15:
			log_message(INFO, "Captured SIGTERM signal !!!");
			break;
	}
	exit_program();
}

void create_signal_capture(void)
{
	signal(SIGINT, handle_sigint);
	signal(SIGSEGV, handle_sigint);
	signal(SIGTERM, handle_sigint);
	signal(SIGALRM, handle_sigint);
	signal(SIGPIPE, handle_sigint);
}
