#include <stdio.h>
#include <stdint.h>



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

    return 0;  // Return 0 for success
}