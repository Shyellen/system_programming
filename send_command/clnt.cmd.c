#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <pthread.h>

#define MAXARGS	20
#define oops(msg)		{perror(msg); exit(1);}

void *send_msg(void *arg);
void *recv_msg(void *arg);
int rpipe[2];

int main(int ac, char *av[])
{
	int sock;					/* the socket and fd	*/
	struct sockaddr_in serv_adr;/* the number to call	*/
	char combuf[BUFSIZ];
	char *command[MAXARGS];
	int str_len, numargs=0;
	pthread_t snd_thread, rcv_thread;

	if(ac != 3) {
		printf("[USAGE] %s <IP> <PORT>\n", av[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		oops("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(av[1]);
	serv_adr.sin_port=htons(atoi(av[2]));

	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) != 0)
		oops("connect() error");
	printf("Successfully connected.\n");

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, NULL);
	pthread_join(rcv_thread, NULL);
	
	close(sock);  
	return 0;
}

void *send_msg(void *arg)
{
	int sock=*((int*)arg);
	char result[BUFSIZ];

	printf("[DEBUG] This is send function.\n");

	//if(dup2(rpipe[0], 0) == -1)
	//	oops("dup2() stdin error");
	//close(rpipe[0]);

	while(1) {
		fgets(result, BUFSIZ, stdin);
		write(sock, result, strlen(result));
	}
	return NULL;
}

void *recv_msg(void *arg)
{
	int sock=*((int*)arg);
	char *com[100], command[BUFSIZ];
	int str_len, pid, i=0;

	printf("[DEBUG] This is recv function.\n");

	//if(dup2(rpipe[1], 1) == -1)
	//	oops("dup2() stdout error");
	//close(rpipe[1]);

	while(1)
	{
		for(i=i; i>=0; i--)
			com[i] = 0;
		i=0;
		str_len=read(sock, command, BUFSIZ);
		if(str_len==-1) 
			oops("read() error");
		command[str_len-1]=0;

		com[i] = strtok(command, " ");
		while(com[i] != NULL)
			com[++i] = strtok(NULL, " ");

		pid = fork();
		if(pid == 0)
			execvp(com[0], com);
	}
	return NULL;
}
