#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> 
#include <math.h>
#include "parsing.h"
#define MAXSIZE 128

char user1[], user2[];
int send_counter = 0, rcv_counter = 0;
int msqid, pid1, pid2, status = 1;
int msgflg = IPC_CREAT | 0666;
size_t buflen;
key_t key = 1234;

struct msgbuf
{
	long mtype;
	char mtext[MAXSIZE];
};

struct msgbuf sbuf;
struct msgbuf rcvbuffer;

void funk(char *s)
{
	perror(s);
	exit(1);
}

void notify()
{
	char command[100];
	strcpy(command, "notify-send ");
	strcat(command, user2);
	strcat(command, " \'");
	strcat(command, rcvbuffer.mtext);
	strcat(command, "\' -t 5000 -i \'/root/Desktop/msg_icon.jpg\'");
	system(command);
}

void rcvmsg()
{

	if ((msqid = msgget(key, 0666)) < 0)
		funk("msgget()");
	//Pranimi i pergjigjes nga mesazhi i tipit 1
	if (msgrcv(msqid, &rcvbuffer, MAXSIZE, 1, 0) < 0)
		funk("msgrcv");
	if(rcv_counter == 0)
		strcpy(user2, rcvbuffer.mtext);
	else if (rcv_counter == 1)
		pid2 = toInt(rcvbuffer.mtext);
	else
	{
		sleep(1);
		printf(user2);	
		printf(": %s\n", rcvbuffer.mtext);
		printf("\a");	
		notify();
	}
	rcv_counter = rcv_counter + 1;
	status = 0;
}

void sendmsg()
{
	//Merr ID-ne e radhes se mesazhit per celesin e caktuar
	if ((msqid = msgget(key, msgflg)) < 0)
		funk("msgget");
	// Tipi i mesazhit
	sbuf.mtype = 1;
	if (send_counter == 0)
	{
		printf("Sheno username-in tend: ");
		scanf("%[^\n]", sbuf.mtext);
		getchar();
		strcpy(user1, sbuf.mtext);
	}
	else if(send_counter == 1)
		toString(sbuf.mtext, pid1);	
	else
	{
		printf(user1);
		printf(": ");
		scanf("%[^\n]", sbuf.mtext);
		getchar();		
	}

	buflen = strlen(sbuf.mtext) + 1;
	if (msgsnd(msqid, &sbuf, buflen, IPC_NOWAIT) < 0)
	{
		printf("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buflen);
		funk("msgsnd");
	}
	else
	{
		status = 1;
		send_counter = send_counter + 1;
	}
}

void handle_sigint() 
{ 
	printf("\nPerfundim i bisedes.\n");
	kill(pid2, SIGUSR1);
	sleep(5); 
	exit(0);
} 

void handle_sigusr1()
{
	printf("Te fala nga ");
	printf(user2);
	printf(".\nBiseda mbaroi.\n");
	sleep(5);
	exit(0);
}

int main()
{
	pid1 = getpid();
	
	signal(SIGINT, handle_sigint);
	signal(SIGUSR1, handle_sigusr1);

	while (1)
	{
		while (status != 0)
		{
			rcvmsg();
		}
		sendmsg();
	}

	exit(0);
}
