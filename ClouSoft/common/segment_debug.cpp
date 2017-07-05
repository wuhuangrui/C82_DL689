
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000

void *buffer[SIZE];

int system_debug_step = 0;

void fault_trap(int n, struct siginfo *siginfo, void *myact)
{
	/*int i, num;
	char **calls;
	printf("Fault address:%x\n", siginfo->si_addr);
	num = backtrace(buffer,SIZE);
	calls = backtrace_symbols(buffer, num);
	for (i = 0; i < num; i++)
		printf("%s\n", calls[i]);

	printf("SYSTEM_DEBUG_STEP(%d)\r\n", system_debug_step);

	exit(1);*/
}

void fault_trap1(int signo)
{
#ifdef SYS_LINUX
	int i, num;
	char **calls;
	num = backtrace(buffer,SIZE);
	calls = backtrace_symbols(buffer, num);
	for (i = 0; i < num; i++)
		printf("%s\n", calls[i]);

	printf("SYSTEM_DEBUG_STEP(%d)\r\n", system_debug_step);

	exit(1);
#endif
}

void sys_signal_setup(void)
{
#ifdef SYS_LINUX
	/*struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = fault_trap;
	sigaction(SIGSEGV, &act, NULL);*/
	
	signal(SIGSEGV, fault_trap1);
#endif
}

/*
int main(int argc, char **argv)
{
	int *ptr = NULL;
	
	printf("hello 1 error\n");

	setuptrap();

	*ptr = 199;
	printf("hello 2 error\n");

	pause();

	return 0;
}

*/

