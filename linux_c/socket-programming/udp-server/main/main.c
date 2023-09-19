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
    char *srvr_addr = NULL;
    struct sockaddr_in adr_inet; /* AF_INET */
    struct sockaddr_in adr_clnt; /* AF_INET client */
    int len_inet;
    /* length */
    int s;
    /* Socket */
    char dgram[512];
    /* Recv buffer */
    char dtfmt[512];
    /* Date/Time Result */
    time_t td;
    /* Current Time and Date */
    struct tm tm;
    /* Date time values */

    srvr_addr = "127.0.0.3";
    /*
     * Create a UDP socket to use:
     */
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1)
        bail("socket()");

    /*
     * Create a socket address, for use
     * with bind(2):
     */
    memset(&adr_inet, 0, sizeof adr_inet);

    adr_inet.sin_family = AF_INET;
    adr_inet.sin_port = htons(9090);
    // adr_inet.sin_addr.s_addr = inet_addr(srvr_addr);
    // converting the address to the address type
    z = inet_aton(srvr_addr, &adr_inet.sin_addr);
    if (0 == z)
    {
        bail("inet_aton()");
    }

    len_inet = sizeof adr_inet;

    // binding socket
    /*
     * Bind a address to our socket, so that
     * client programs can contact this
     * server:
     */
    z = bind(s,
             (struct sockaddr *)&adr_inet,
             len_inet);
    if (z == -1)
        bail("bind()");

    // waiting for the request

    while (1)
    {
        /*
         * Block until the program receives a
         * datagram at our address and port:
         */
        len_inet = sizeof adr_clnt;
        z = recvfrom(s,
                     /* Socket */
                     dgram,
                     /* Receiving buffer */
                     sizeof dgram,
                     /* Max recv buf size */
                     0,
                     /* Flags: no options */
                     (struct sockaddr *)&adr_clnt, /* Addr */
                     &len_inet);
        /* Addr len, in & out */
        if (z < 0)
            bail("recvfrom(2)");

        /*
         * Process the request:
         */
        dgram[z] = 0;
        printf("Received from client %s\n",dgram);
        /* null terminate */
        if (!strcasecmp(dgram, "QUIT")) // quit as soon as possible when receive quit
            break;                      /* Quit server */

        /*
         * Get the current date and time:
         */
        time(&td);
        /* Get current time & date */
        tm = *localtime(&td); /* Get components */

        /* Format a new date and time string,
         * based upon the input format string:
         */
        strftime(dtfmt,
                 /* Formatted result */
                 sizeof dtfmt,
                 /* Max result size */
                 dgram,
                 /* Input date/time format */
                 &tm);
        /* Input date/time values */

        // sending back the formatted result back to client
         z = sendto(s,
                    dtfmt,
                    strlen(dtfmt),
                    0,
                    (struct sockaddr *)&adr_clnt,
                    len_inet);
        if( 0 > z)
        {
            bail("sendto(2)");
        }
       

    }

    // closing the socket 
    close(s);

    return 0; // Return 0 for success
}