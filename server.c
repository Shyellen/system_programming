#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <pthread.h>

#define oops(msg)   {perror(msg); exit(1);}
#define MAX_CLNT 256

void * handle_clnt(void * arg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
char command[BUFSIZ];

int main(int ac, char *av[])
{
	int serv_sock, clnt_sock, str_len, i;
	struct sockaddr_in serv_adr;
	char message[BUFSIZ];		/* to receive message	*/
	pthread_t t_id;

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

	fputs("[INPUT] Input command: ", stdout);
	fgets(command, BUFSIZ, stdin);
	for(i=0; i < BUFSIZ ; i++){
		if(command[i] == '\n'){
			command[i] = 0;
			break;
		}
	}
	printf("[DEBUG] Command is %s\n", command);
	fflush(stdout);
	fputs("-------------------------------\n", stdout);
	fflush(stdout);

	fputs("[NOTICE] Waiting for clients...\n", stdout);
	fflush(stdout);

	while(1)
	{
		clnt_sock = accept(serv_sock, NULL, NULL);
		if( clnt_sock == -1)
			oops("accept() error");

		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);

		printf("[NOTICE] New client is connected! - %d\n", clnt_sock);
		fflush(stdout);
	}

	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len=0, i;
	//char command[BUFSIZ];
	char message[BUFSIZ];

	if( write(clnt_sock, command, strlen(command)) == -1)
		oops("write command  error");

	//get result
	while((str_len = read(clnt_sock, message, BUFSIZ)) != 0)
		write(1, message, str_len);

	//remove disconnected client
	pthread_mutex_lock(&mutx);
	for( i = 0 ; i < clnt_cnt ; i++)
	{
		if(clnt_sock == clnt_socks[i])
		{
			while( i < clnt_cnt){
				clnt_socks[i] = clnt_socks[i+1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	
	close(clnt_sock);
	return NULL;
}
