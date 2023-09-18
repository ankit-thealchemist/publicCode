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

    int z;                       /* Status return code */
    int sck_inet;                /* Socket */
    struct sockaddr_in adr_inet; /* AF_INET */
    int len_inet;                /* length */
    const unsigned char IPno[] = {
        127, 0, 0, 23
        /* Local loopback */
    };

    const char* ip_address = "127.0.0.232";
    int port = 9000;

    /* Create an IPv4 Internet Socket */
    sck_inet = socket(AF_INET, SOCK_STREAM, 0);

    if (sck_inet == -1)
        bail("socket()");

    /* Create an AF_INET address */
    memset(&adr_inet, 0, sizeof adr_inet);

    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = htons(port);
    // memcpy(&adr_inet.sin_addr.s_addr, IPno, 4);
    adr_inet.sin_addr.s_addr = inet_addr(ip_address); 
    len_inet = sizeof adr_inet;
    printf("Size of address is %d\r\n", len_inet);

    // binding the socket with the address

    z = bind(sck_inet, (struct sockaddr *)&adr_inet, len_inet);

    if (-1 == z)
    {
        bail("bind()");
    }

    // displaying the bound address
    system("netstat -pa --tcp 2>/dev/null | sed -n '1,/^Proto/p;/af_inet/p'");
    // system("netstat -tuln | grep '127.0.0.23:9000'");

    close(sck_inet);
    return 0; // Return 0 for success
}