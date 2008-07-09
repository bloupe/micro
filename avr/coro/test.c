#include <stdio.h>
#include <windows.h>
#include <setjmp.h>


jmp_buf tsk[3], mainTsk[3], mainTask;

struct CORO_STATE
{
	int state;
	void *resume;
};

int yielder()
{
	static int time = 0;

	if(time > 5)
	{
		time = 0;
		return 1;
	}
	time ++;
	return 0;
}

#define CORO_START static struct CORO_STATE pt = {0, NULL}; if(pt.resume) { printf("Resuming\r\n"); goto *pt.resume; }
#define CORO_YIELD { __label__ resume; resume: pt.resume = &&resume; } if(yielder()) return 0

int func1(int tskNo);
int func2(int tskNo);

//a simple method to determine the direction of the stack growth.
void stack_growth(char *function_parameter)
{
	char local;
	if(&local > function_parameter) printf("The stack grows up\n");
	else printf("The stack grows down\n");
}

int main(int argc, char **argv)
{

	char c = 'b';
	stack_growth(&c);

	int stackData[20];
	//Save our registers!
	__asm__
	(
		"movl %%ebx, (%%eax)\n" 
		"movl %%esi, (4)(%%eax)\n"
		"movl %%edi, (8)(%%eax)\n"
		"movl %%esp, (12)(%%eax)\n"
		"movl %%ecx, (16)(%%eax)\n"
		"movl %%ebp, (20)(%%eax)\n"
		
		:
		:"a"(stackData)
	);
	printf("ESP OUTER: %d\n", stackData[3]);
	printf("EBP OUTER: %d\n", stackData[5]);

	stackPrint();

	return 0;
	if(!setjmp(mainTask)) func1(0);
	if(!setjmp(mainTask)) func1(1);


	while(1)
	{
		//func1(0);
		//func1(1);
		printf("Main\n");
		if(!setjmp(mainTask)) longjmp(tsk[0], 1);
		if(!setjmp(mainTask)) longjmp(tsk[1], 1);
	}
}


int func1(int tn)
{
	char space[10000];
	space[9999] = 1;
	//CORO_START;
	int state = 0;
	int blah = 0;
	int tskNo = tn;
	printf("I should not get called when resuming: %d\n", tskNo);
	while(1)
	{

		if(!setjmp(tsk[tskNo])) longjmp(mainTask, 1);
		
		/*
		if(!setjmp(tsk[tskNo]))
		{
			CORO_YIELD;
			longjmp(tsk[tskNo], 0);
		}
		*/

		printf("hello from func1(%d): %d\r\n", tskNo, blah ++);
	}
}

int func2(int tskNo)
{
	CORO_START;
	int state = 0;
	int blah = 0;
	printf("I should not get called when resuming\r\n");
	while(1)
	{
		//if(!setjmp(childTask2))
		{
			CORO_YIELD;
			//longjmp(childTask2, 1);
		}

		char buffer[20];
		printf("hello from func2: %d\r\n", blah ++);
	}
}

