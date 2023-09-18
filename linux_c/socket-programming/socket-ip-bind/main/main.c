#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * This function reports the error and
 * exits back to the shell:
 */
static void
bail(const char *on_what)
{
    perror(on_what);
    exit(1);
}

int main(int argc, char *argv[])
{

    int z;
    struct sockaddr_in adr_inet; /* AF_INET */
    int len_inet;
    /* length */
    int sck_inet;
    /* Socket */
    /* Create a Socket */
    sck_inet = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket number is %d\n",sck_inet);
    if (sck_inet == -1)
        bail("socket()");
    /* Establish address */
    memset(&adr_inet, 0, sizeof(adr_inet));
    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = htons(9000);
    adr_inet.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (adr_inet.sin_addr.s_addr == INADDR_NONE)
        bail("bad address.");
    len_inet = sizeof adr_inet;

    // binding sockeet

    z = bind(sck_inet, (struct sockaddr *)&adr_inet, len_inet);

    printf("bind return is %d\r\n",z);
    if (-1 == z)
    {
        bail("bind()");
    }

    listen(sck_inet,10);  // listing so that it  may show in netstat/lsof

    /* Display our socket address */
    z = system("lsof -i tcp |grep socket-se");
    if(-1 == z)
    {
        bail("system()");
    }

    return 0; // Return 0 for success
}