#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>

#define oops(msg)   {perror(msg); exit(1);}
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void *get_command(void *a);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int ac, char *av[])
{
	int serv_sock, clnt_sock, str_len;
	struct sockaddr_in serv_adr;
	pthread_t t_id, sender;

	if(ac != 2) {
		printf("[USAGE] %s <PORT>\n", av[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		oops("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(av[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		oops("bind() error");

	if(listen(serv_sock, 10) != 0)
		oops("listen() error");

	fputs("[NOTICE] Waiting for clients...\n", stdout);
	pthread_create(&sender, NULL, get_command, (void*)&ac);
	
	while(1)
	{
		clnt_sock = accept(serv_sock, NULL, NULL);
		if(clnt_sock == -1)
			oops("accept() error");

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
	}
	pthread_join(sender, NULL);
	close(serv_sock);

	return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len=0, i;
	char message[BUFSIZ];
	int fd;

	printf("\n[NOTICE] New #%d client is connected.\n", clnt_sock);

	while(1) {
		while(1) {
			str_len = read(clnt_sock, message, BUFSIZ);
			if(str_len == -1)
				oops("read() error");
			printf("[NOTICE] Client #%d result ------------------\n", clnt_sock);
			if(write(1, message, str_len) == -1)
					oops("write() error");
			printf("---------------------------------------------\n");
			if(str_len < BUFSIZ)
					break;
		}
	}

	close(clnt_sock);
	return NULL;
}


void *get_command(void *a)
{
	char command[BUFSIZ], i;
	char filename[BUFSIZ];
	char clnt_num[10];
	int clnt, str_len;

	while(1) {
		if(clnt_cnt == 0)
			continue;
		sleep(1);
		fputs("[INPUT] input command: ", stdout);
		fgets(command, BUFSIZ, stdin);

		/* copy command */
		if(!strcmp(command, "copy\n"))
		{	
			command[strlen(command)-1] = 0;
			fputs("[COPY] From which client do you want to copy? Enter number\n", stdout);
			read(0, clnt_num, sizeof(clnt_num));
			
			fputs("[COPY] Enter filename!\n", stdout);
			read(0, filename, BUFSIZ);

			sprintf(command, "%s %s", command, filename);
			
			clnt = clnt_num[0] -'0';

			if( write(clnt, command, strlen(command)) == -1)
				oops("write() command error");

		}

		/* normal command  ex)ls   */
		else
		{
			for(i=0; i<clnt_cnt; i++) {
				if(write(clnt_socks[i], command, strlen(command)) == -1)
					oops("write() command error");
			}
			if(!strcmp(command, "q\n")) {
				exit(1);
			}
		}
	}

	return NULL;
}
