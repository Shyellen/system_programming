// remote ls -- client

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#define PORTNUM 15000
#define oops(msg)               {perror(msg); exit(1);}

void sanitize(char *str);

int main(int ac, char *av[])
{
        struct sockaddr_in servadd;
        struct hostent *hp;
        int sock_id;
        FILE *sock_fpi, *sock_fpo;
        FILE *pipe_fp;
        char dirname[BUFSIZ];
        char command[BUFSIZ];
        int c;

/*
        if(ac != 3) {
                printf("Usage : %s <IP> <PORT>\n", av[0]);
                exit(1);
        }
*/
        //socket()
        sock_id = socket(AF_INET, SOCK_STREAM, 0);
        if(sock_id == -1)
                oops("socket");

        //bind()
        bzero(&servadd, sizeof(servadd));
        hp = gethostbyname(av[1]);
        if(hp == NULL)
                oops(av[1]);
        bcopy(hp->h_addr, (struct sockaddr*)&servadd.sin_addr, hp->h_length);
        //servadd.sin_port = htons(atoi(av[2]));
        servadd.sin_port = htons(PORTNUM);
        servadd.sin_family = AF_INET;

        //connect()
        if(connect(sock_id, (struct sockaddr*)&servadd, sizeof(servadd)) != 0)
                oops("connect");

        //main
        if((sock_fpi = fdopen(sock_id, "r")) == NULL)           //fd open for reading
                oops("fdopen reading");

        if((fgets(dirname, BUFSIZ-5, sock_fpi)) == NULL)        //get dirname
                oops("reading dirname");

        sanitize(dirname);                                      //repair dirname
        sprintf(command, "ls %s", dirname);

        if((sock_fpo = fdopen(sock_id, "w")) == NULL)           //fd open for writing
                oops("fdopen writing");

        if((pipe_fp = popen(command, "r")) == NULL)             //execute command
                oops("popen");

        while( (c=getc(pipe_fp)) != EOF)                        //get result from pipe
                putc(c, sock_fpo);                              //put to socket


        //close
        pclose(pipe_fp);
        fclose(sock_fpo);
        fclose(sock_fpi);


        return 0;
}

//remove execpt slashes and alphanumber
void sanitize(char *str) {
        char *src, *dest;

        for(src=dest=str; *src; src++) {
                if(*src == '/' || isalnum(*src))
                        *dest++ = *src;
        }
        *dest = '\0';
}
