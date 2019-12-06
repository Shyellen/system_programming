// remote find -- server

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#define PORTNUM 15000
#define HOSTLEN 256
#define oops(msg)   {perror(msg); exit(1);}

int main(int ac, char *av[])
{
        struct sockaddr_in saddr;
        struct hostent *hp;
        char hostname[HOSTLEN];
        int sock_id, sock_fd;
        int n_read;
        char buffer[BUFSIZ];

/*
        if(ac != 3){
                printf("Usage : %s <PORT> <dirname>\n", av[0]);
                exit(1);
        }
*/
        //socket()
        sock_id = socket(PF_INET, SOCK_STREAM, 0);
        if(sock_id == -1)
                oops("socket");

        //bind()
        bzero((void *)&saddr, sizeof(saddr));
        gethostname(hostname, HOSTLEN);
        hp = gethostbyname(hostname);
        bcopy((void*)hp->h_addr, (void*)&saddr.sin_addr, hp->h_length);
        saddr.sin_port=htons(PORTNUM);
        //saddr.sin_port=htons(atoi(av[1]));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr=htonl(INADDR_ANY);
        if(bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
                oops("bind");

        //listen()
        if(listen(sock_id, 1) != 0)
                oops("listen");

        sock_fd = accept(sock_id, NULL, NULL);                  //accept()
        if(sock_fd == -1)
                oops("accept");

        //main
        //if(write(sock_fd, av[2], strlen(av[1])) == -1)
        if(write(sock_fd, av[1], strlen(av[1])) == -1)          //write dirname to client
                oops("write dirname");
        if(write(sock_fd, "\n", 1) == -1)
                oops("write");

        while( (n_read = read(sock_fd, buffer, BUFSIZ)) > 0)    //read result
                if(write(1, buffer, n_read) == -1)              //write to stdout
                        oops("write");

        close(sock_id);
        return 0;

}
