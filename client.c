#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define oops(msg)		{perror(msg); exit(1);}

void sanitize(char *str);

int main(int ac, char *av[])
{
	int sock;					/* the socket and fd	*/
	struct sockaddr_in serv_adr;/* the number to call	*/
	FILE *sock_fpi, *sock_fpo;
	FILE *pipe_fp;
	char dirname[BUFSIZ];
	char command[BUFSIZ];
	int str_len, c;

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
   
	if((sock_fpi = fdopen(sock, "r")) == NULL)
		oops("fdopen reading");
	if(fgets(dirname, BUFSIZ-5, sock_fpi) == NULL)
		oops("reading dirname");

	sanitize(dirname);

	if((sock_fpo = fdopen(sock, "w")) == NULL)
		oops("fdopen writing");

	sprintf(command, "ls %s", dirname);
	if((pipe_fp = popen(command, "r")) == NULL)
		oops("popen() error");
	
	while((c=getc(pipe_fp)) != EOF)
		putc(c, sock_fpo);

	pclose(pipe_fp);
	fclose(sock_fpo);
	fclose(sock_fpi);
	close(sock);
	return 0;
}

void sanitize(char *str) {
	char *src, *dest;
	
	for(src=dest=str; *src; src++) {
		if(*src == '/' || isalnum(*src))
			*dest++ = *src;
	}
	*dest = '\0';

	return;
}