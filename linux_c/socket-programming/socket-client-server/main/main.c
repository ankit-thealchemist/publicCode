#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[])
{

    printf("Socket server \r\n");

    int z;        /* Status return code */
    int s[2];     /* Pair of sockets */
    char *cp;     /* A work pointer */
    pid_t chpid;  /* Child PID */
    char buf[80]; /* Work buffer */
    char *msgp;   /* A message pointer */
    int mlen;
    /*
     * Create a pair of local sockets:
     */
    z = socketpair(AF_LOCAL, SOCK_STREAM, 0, s);

    if (0 != z)
    {
        fprintf(stderr, "%s", strerror(errno));
        return 1;
    }

    /*
     * Report the socket file descriptors returned:
     */
    printf("s[0] = %d;\n", s[0]);
    printf("s[1] = %d;\n", s[1]);

    /**
     * @brief Forking the process
     *
     */
    chpid = fork();

    if ((pid_t)-1 == chpid)
    {
        fprintf(stderr, "Unable to fork the procedd %s", strerror(errno));
        exit(1);
    }

    else if (0 == chpid)
    {
        /*
         * This is the child process (client):
         */
        char rxbuf[80]; /* Receive buffer */
        printf("Parent PID is %ld\n", (long)getppid());

        close(s[0]);
        s[0] = -1; /*Server uses s[1] */
        /* Forget this unit */
        /*
         * Form the message and its length:
         */
        msgp = "%A %d-%b-%Y %l:%M %p";
        mlen = strlen(msgp);
        printf("Child sending request '%s'\n", msgp);
        fflush(stdout);
        /*
         * Write a request to the server:
         */
        z = write(s[1], msgp, mlen);

        if (z < 0)
        {
            fprintf(stderr, "%s: write(2)\n", strerror(errno));
            exit(1);
        }

        /*
         * Now indicate that we will not be writing
         * anything further to our socket, by shutting
         * down the write side of the socket:
         */
        if (-1 == shutdown(s[1], SHUT_WR))
        {
            fprintf(stderr, "%s: shutdown (2)\n", strerror(errno));
            exit(1);
        }

        /*
         * Receive the reply from the server:
         */
        z = read(s[1], rxbuf, sizeof rxbuf);
        if (z < 0)
        {
            fprintf(stderr, "%s: read(2)\n",
                    strerror(errno));
            exit(1);
        }
        /*
         * Put a null byte at the end of what we
         * received from the server:
         */
        rxbuf[z] = 0;
        /*
         * Report the results:
         */
        printf("Server returned '%s'\n", rxbuf);
        fflush(stdout);
        close(s[1]);
        /* Close our end now */
    }

    else
    {
        /*
         * This is the parent process (server):
         */
        int status; /* Child termination status */
        char txbuf[80];
        /* Reply buffer */

        time_t td;
        /* Current date & time */
        printf("Child PID is %ld\n", (long)chpid);
        fflush(stdout);

        close(s[1]);            /* Client uses s[1] */
        s[1] = -1;              /* Forget this descriptor */

        /*` `
         * Wait for a request from the client:
         */
        z = read(s[0], buf, sizeof buf);

        if (z < 0)
        {
            fprintf(stderr, "%s: read(2)\n",
                    strerror(errno));
            exit(1);
        }

        /*
         * Put a null byte at the end of the
         * message we received from the client:
         */
        buf[z] = 0;

        /*
         * Now perform the server function on the
         * received message:
         */
        time(&td);
        /* Get current time */

        strftime(txbuf, sizeof txbuf,
                 /* Buffer */
                 buf,
                 /* Input format */
                 localtime(&td));       /* Input time */

        /*
         * Send back the response to client:
         */
        z = write(s[0], txbuf, strlen(txbuf));
        if (z < 0)
        {
            fprintf(stderr, "%s: write(2)\n",
                    strerror(errno));
            exit(1);
        }
        /* Close our end of the socket:
         */
        close(s[0]);

        /*
         * Wait for the child process to exit.
         * See text.
         */
        waitpid(chpid, &status, 0);
    }

    printf("Closing program \r\n");
    close(s[0]);
    close(s[1]);
    return 0; // Return 0 for success
}