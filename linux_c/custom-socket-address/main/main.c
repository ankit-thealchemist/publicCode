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
    /* Status return code */
    int sck_unix;
    /* Socket */
    struct sockaddr_un adr_unix; /* AF_UNIX */
    int len_unix;
    /* length */
    const char pth_unix[]
        /* pathname */
        = "Z*MY-SOCKET*";

    /*
     * Create a AF_UNIX (aka AF_LOCAL) socket:
     */
    sck_unix = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sck_unix == -1)
        bail("socket()");

    // removing the path if already exits in system due to previous run
    // unlink(pth_unix);

    // createing the new address
    memset(&adr_unix, 0, sizeof(adr_unix));

    adr_unix.sun_family = AF_LOCAL;
    strncpy(adr_unix.sun_path, pth_unix, sizeof adr_unix.sun_path - 1);
    adr_unix.sun_path[sizeof adr_unix.sun_path - 1] = 0;

    len_unix = SUN_LEN(&adr_unix);

    /* Now make first byte null */
    adr_unix.sun_path[0] = 0;
    printf("Length of the address is %d\r\n", len_unix);

    /*
     * Now bind the address to the socket:
     */
    z = bind(sck_unix,
             (struct sockaddr *)&adr_unix,
             len_unix);

    if (z == -1)
        bail("bind()");

    // display our socket
    // system("netstat -pa --unix 2>/dev/null |"
    //        "sed -n '/^Active UNIX/,/^Proto/p;"
    //        "/af_unix/p'");
    system("netstat -x | grep MY-SOCKET");

    /*
     * Close and unlink our socket path:
     */
    close(sck_unix);

    unlink(pth_unix);

    return 0; // Return 0 for success
}