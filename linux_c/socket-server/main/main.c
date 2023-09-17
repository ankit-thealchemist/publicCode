#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{

    printf("Socket server \r\n");

    int errno; /* Status return code */
    int s[2];  /* Pair of sockets */
    char *cp;  /* A work pointer */

    char buf[80]; /* Work buffer */
    /*
     * Create a pair of local sockets:
     */
    errno = socketpair(AF_LOCAL, SOCK_STREAM, 0, s);

    if (0 != errno)
    {
        fprintf(stderr, "%s", strerror(errno));
        return 1;
    }

    /*
     * Report the socket file descriptors returned:
     */
    printf("s[0] = %d;\n", s[0]);
    printf("s[1] = %d;\n", s[1]);
    system("netstat --unix -p ");
    // system("netstat|grep tcp");

    /**
     *Write a message to socket s[1] : */
    errno = write(s[1], cp = "Hello?", 6);
    if (errno < 0)
    {
        fprintf(stderr,
                "%s: write(%d,\"%s\",%d)\n",
                strerror(errno), s[1], cp, strlen(cp));
        return 2; /* Failed write */
    }

    printf("Wrote message '%s' on s[1]\n", cp);

    /*
     * Read from socket s[0]:
     */
    errno = read(s[0], buf, sizeof buf);
    if (errno < 0)
    {
        fprintf(stderr,
                "%s: read(%d,buf,%d)\n",
                strerror(errno), s[0], sizeof buf);
        return 3;
        /* Failed read */
    }

    buf[errno] = 0; /* NUL terminate */
    printf("Received message '%s' from socket s[0]\n",
           buf);

    /*
     * Send a reply back to s[1] from s[0]:
     */
    errno = write(s[0], cp = "Go away!", 8);
    if (errno < 0)
    {
        fprintf(stderr,
                "%s: write(%d,\"%s\",%d)\n",
                strerror(errno), s[0], cp, strlen(cp));
        return 4;
        /* Failed write */
    }
    printf("Wrote message '%s' on s[0]\n", cp);

    /*
     * Read from socket s[1]:
     */
    errno = read(s[1], buf, sizeof buf);

    if (errno < 0)
    {
        fprintf(stderr,
                "%s: read(%d,buf,%d)\n",
                strerror(errno), s[1], sizeof buf);
        return 3;
        /* Failed read */
    }

    /*
     * Report message received by s[0]:
     */
    buf[errno] = 0;
    /* NUL terminate */
    printf("Received message '%s' from socket s[1]\n",
           buf);
    /**
     * @brief closing sockets
     *
     */
    printf("Closing sockets \r\n");
    close(s[0]);
    close(s[1]);
    return 0; // Return 0 for success
}