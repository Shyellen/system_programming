#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define oops(msg)   {perror(msg); exit(1);}

int main(int ac, char *av[])
{
	int serv_sock, clnt_sock, str_len;
	struct sockaddr_in serv_adr;
	char message[BUFSIZ];		/* to receive message	*/
	char command[BUFSIZ];

	if(ac != 2) {
		printf("[USAGE] %s <PORT>\n", av[0]);
		exit(1);
	}

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

	while(1) {
		clnt_sock = accept(serv_sock, NULL, NULL);
		if(clnt_sock == -1)
			oops("accept() error");
		fputs("[NOTICE] New client is connected\n", stdout);

		while(1) {
			fgets(command, BUFSIZ, stdin);
			printf("[DEBUG] command: %s\n", command);

			if(write(clnt_sock, command, strlen(command)) == -1)
				oops("write() error");

			//while((str_len = read(clnt_sock, message, BUFSIZ)) > 0) {
			//	if( write(1, message, str_len) == -1)
			//		oops("write() error");
			//}
		}
		close(clnt_sock);
	}
	close(serv_sock);
	return 0;
}
