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
#include <glib-unix.h>
#include <glib/gprintf.h>
#include <sys/wait.h>
#include "linenoise.h"

typedef struct
{
	pid_t pid;
	int addr;
	int enabled;
	int saveddata;
}Breakpoint_t ;

int run_debugger(pid_t pid);
int handle_command(char *line,pid_t pid);
int continue_execution(pid_t pid);
int breakpoint_is_enabled(Breakpoint_t *b);
int breakpoint_get_address(Breakpoint_t *b);
void breakpoint_enable(Breakpoint_t *b,pid_t pid);
void breakpoint_disable(Breakpoint_t *b,pid_t pid);
void set_breakpoint_at_address(pid_t pid, gint addr);

void SigHandler(int signo){
	if (signo==SIGINT)
		g_printf("Received SigInt\n");
}

int main(int argc, char *argv[]){

	signal(SIGINT,SigHandler);

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
	else if ( g_str_has_prefix(commands[0], "break") == 1){
		gint addr = (gint) g_ascii_strtoll(commands[1], NULL, 16);
		set_breakpoint_at_address(pid, addr);
	}
	else
		g_printf("Unknown command\n");

	return 0;
}

int continue_execution(pid_t pid){

	ptrace(PTRACE_CONT, pid,NULL,NULL);
	return 0;
}

int breakpoint_is_enabled(Breakpoint_t *b){

	return b->enabled;
}
int breakpoint_get_address(Breakpoint_t *b){
	return b->addr;
}

void breakpoint_enable(Breakpoint_t *b,pid_t pid){

	int data = ptrace(PTRACE_PEEKDATA,pid, b->addr,NULL);
	b->saveddata = (gint8)(data&0xff);
	gint8 int3 = 0xcc;
	gint64 data_with_int3 = ((data&0xff) | int3);
	ptrace(PTRACE_POKEDATA , pid, b->addr, data_with_int3);

	b->enabled = 1;
}

void breakpoint_disable(Breakpoint_t *b,pid_t pid)
{
	int data = ptrace(PTRACE_PEEKDATA,pid, b->addr,NULL);
	int restored_data = ptrace(( data & ~0xff) | b->saveddata);
	ptrace(PTRACE_POKEDATA,pid,b->addr,restored_data);
	b->enabled  = 0;
}

void set_breakpoint_at_address(pid_t pid, gint addr){

	Breakpoint_t *b = g_new(Breakpoint_t,1);
	b->addr = addr;
	b->pid = pid;
	breakpoint_enable(b, b->pid);
	g_printf("Breakpoint set at addr 0x%x \n", addr);
	//Add it to global list
}




