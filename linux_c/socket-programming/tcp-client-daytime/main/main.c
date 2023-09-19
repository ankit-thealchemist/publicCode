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
#include <netdb.h>

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
    char *srvr_addr = NULL;
    struct sockaddr_in adr_srvr; /* AF_INET */
    int len_inet;
    /* length */
    int s;
    /* Socket */
    struct servent *sp;
    /* Service entry */
    char dtbuf[128];
    /* Date/Time info */

    srvr_addr = "127.0.0.1";

    // getting the service

    sp = getservbyname("daytime", "tcp");

    if (!sp)
    {
        fputs("Unknown service \n", stderr);
        exit(1);
    }

    /*
     * Create a server socket address:
     */
    memset(&adr_srvr, 0, sizeof adr_srvr);
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = sp->s_port;

    inet_aton(srvr_addr, &adr_srvr.sin_addr);

    len_inet = sizeof adr_srvr;

    /*
     * Create a TCP/IP socket to use:
     */
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1)
        bail("socket()");

    /*
     * Connect to the server:
     */
    z = connect(s,(struct sockaddr *) &adr_srvr, len_inet);
    if (z == -1)
        bail("connect(2)");

    /*
     * Read the date/time info:
     */
    z = read(s, &dtbuf, sizeof dtbuf - 1);
    if (z == -1)
        bail("read(2)");

    /* Report the Date & Time:*/
    dtbuf[z] = 0;
    /* null terminate string */

    printf("Date & Time is: %s\n", dtbuf);

    /*
     * Close the socket and exit:
     */
    close(s);
    putchar('\n');
    return 0; // Return 0 for success
}