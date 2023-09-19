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
    int x;

    struct servent *sp;

    while (1)
    {

        sp = getservent();
        if (NULL == sp)
            break;
        errno = 0;

        printf("%s:\n"
               "\tPort:% d\n "
               "\tProtocol: %s\n"
               "\tAliases: ",
               sp->s_name, ntohs(sp->s_port), sp->s_proto);
        for (x = 0; sp->s_aliases[x] != NULL; ++x)
            printf("%s ", sp->s_aliases[x]);
        putchar('\n');
    }

    if (errno != 0 &&
        errno != ENOENT) /* For RH-6.0 */
        fprintf(stderr,
                "%s: getservent(3) %d\n",
                strerror(errno), errno);
    return 0; // Return 0 for success
}