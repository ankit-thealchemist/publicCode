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
    char *srvr_addr = "127.0.0.1";
    int srvr_port = 9098;

    struct sockaddr_in adr_srvr; /* AF_INET */
    struct sockaddr_in adr_clnt; /* AF_INET */
    int len_inet;
    /* length */
    int s;
    /* Socket */
    int c;
    /* Client socket */
    int n;
    /* bytes */
    time_t td;
    /* Current date&time */
    char dtbuf[128];
    /* Date/Time info */

    /*
     * Create a TCP/IP socket to use:
     */
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1)
        bail("socket()");

    /*
     * Create a server socket address:
     */
    memset(&adr_srvr, 0, sizeof adr_srvr);
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = htons(srvr_port);

    z = inet_aton(srvr_addr, &adr_srvr.sin_addr);

    /** Bind the server address:
     */
    len_inet = sizeof adr_srvr;

    z = bind(s, (struct sockaddr *)&adr_srvr, len_inet);

    if (z == -1)
        bail("bind(2)");

    /*
     * Make it a listening socket:
     */
    z = listen(s, 10);

    if (z == -1)
        bail("listen(2)");

    // server loop
    while (1)
    {
        // wait for the connection
        len_inet = sizeof adr_clnt;

        c = accept(s, (struct sockaddr *)&adr_clnt, &len_inet);
        printf("Client request accepted\r\n");
        printf("Client got conneted from %s and port %d\r\n",inet_ntoa(adr_clnt.sin_addr),ntohs(adr_clnt.sin_port));
        if (c == -1)
            bail("accept(2)");

        /*
         * Generate a time stamp;
         */
        time(&td);

        n = (int)strftime(dtbuf, sizeof dtbuf, "%A %b %d %H:%M:%S %Y\n", localtime(&td));

        /*
         * Write result back to the client:
         */
        z = write(c, dtbuf, n);

        if (z == -1)
            bail("write(2)");

        /*
         * Close this client's connection;
         */
        close(c);
    }

    return 0; // Return 0 for success
}