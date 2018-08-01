/*
 * wdb.c
 *
 *  Created on: Aug 1, 2018
 *      Author: waqas
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <sys/wait.h>
#include "linenoise.h"

int run_debugger(pid_t pid);
int handle_command(char *line,pid_t pid);
int continue_execution(pid_t pid);


typedef struct Breakpoint
{
	pid_t pid;
	void *addr;
	int enabled;
	gint8 saveddata;
}Breakpoint_t ;

int main(int argc, char *argv[]){

	if (argc < 2){
		printf("Program name not specified \n");
		return -1;
	}

	char *prog = argv[1];
	pid_t pid= fork ();
	if (pid == 0){
		// Child process
		ptrace(PTRACE_TRACEME, 0 , NULL,NULL);
		execl (prog, prog, NULL);

	}
	else if (pid >= 1){
		//Debugger
		run_debugger(pid);
	}
}


int run_debugger(pid_t pid){
	int waitstatus;
	int options = 0;

	waitpid(pid,&waitstatus,options);

	char *line=NULL;

	while((line=linenoise("wdb> ")) != NULL){
		handle_command(line,pid);
		linenoiseHistoryAdd(line);
		linenoiseFree(line);
	}
	return 0;
}


int handle_command(char *line,pid_t pid){
	gchar **commands = g_strsplit((gchar*)line, " ", -1);

	if (g_str_has_prefix(commands[0] , "continue") == 1 )
		continue_execution(pid);
	else
		g_printf("Unknown command\n");

	return 0;
}

int continue_execution(pid_t pid){

	ptrace(PTRACE_CONT, pid,NULL,NULL);
	return 0;
}


