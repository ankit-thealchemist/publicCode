#include <stdio.h>
#include <stdint.h>
#include "string.h"


char uri[] = "wss://ocpp-uat.hiev.network/ocpp/ELTST7401" ;
int main(int argc, char *argv[]) {
    // Check the number of command-line arguments
    // if (argc < 2) {
    //     printf("Usage: %s <argument1> [argument2 ...]\n", argv[0]);
    //     return 1;  // Return a non-zero exit status to indicate an error
    // }

    // Your code here

    // Print the command-line arguments
    printf("Number of arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }

    char url[200];
    memset(url,0,sizeof(url));
    char cpid[50];
    memset(cpid,0,sizeof(cpid));



    char *cpid_copy = strrchr(uri,'/');
    printf("%s\r\n",cpid+1);

    strcpy(cpid,cpid_copy+1);
    printf("%s\r\n",cpid);
    memcpy(url,uri,cpid_copy-uri);
    printf("%s\r\n",url);

    return 0;  // Return 0 for success
}