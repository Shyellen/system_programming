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

int rpipe[2];
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

	pipe(rpipe);

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

	printf("[NOTICE] New #%d client is connected.\n", clnt_sock);

	while(1) {
		while(1) {
			printf("Go on!\n");	

			str_len = read(clnt_sock, message, BUFSIZ);
			if(str_len == -1)
				oops("read() error");
			printf("[NOTICE] This client is #%d ----------------\n", clnt_sock);
			if(write(1, message, str_len) == -1)
					oops("write() error");
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
		fputs("[get_command()] Input command!!\n", stdout);
		fgets(command, BUFSIZ, stdin);

		/* quit */
		if( !strcmp(command, "q\n"))
			exit(1);

		printf("[get_command()] clnt_cnt: %d, command: %s", clnt_cnt, command);

		/* copy command */
		if( !strcmp(command, "cp\n"))
		{	

			command[strlen(command)-1] = 0;
			fputs("[COPY] From which client do you want to copy? Enter number\n", stdout);
			read(0, clnt_num, sizeof(clnt_num));
			
			fputs("[COPY] Enter filename!\n", stdout);
			read(0, filename, BUFSIZ);
			printf("[DEBUG] filename: %s", filename);

			sprintf(command, "%s %s", command, filename);
			printf("[DEBUG] command + filename -> clnt_num : %s -> %s", command, clnt_num);
			
			clnt = clnt_num[0] -'0';
			printf("[DEBUG] clnt : %d\n", clnt);

			if( write(clnt, command, strlen(command)) == -1)
				oops("write() command error");
			fputs("[COPY] wrote! \n", stdout);

		}

		/* normal command  ex)ls   */
		else
		{
			for(i=0; i<clnt_cnt; i++) {
				if(write(clnt_socks[i], command, strlen(command)) == -1)
					oops("write() command error");
				fputs("[get_command()] wrote! \n", stdout);
			}
		}
	}

	return NULL;
}
