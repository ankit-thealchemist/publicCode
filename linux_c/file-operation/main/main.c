#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define RFID_CHAR_SIZE 8
#define RFID_TXT_FILE_NAME "rfid.txt"

int main(int argc, char *argv[])
{
    // Check the number of command-line arguments
    // if (argc < 2) {
    //     printf("Usage: %s <argument1> [argument2 ...]\n", argv[0]);
    //     return 1;  // Return a non-zero exit status to indicate an error
    // }

    // Your code here

    // Print the command-line arguments
    printf("Number of arguments: %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("Argument %d: %s\n", i, argv[i]);
    }

    FILE *file = fopen(RFID_TXT_FILE_NAME, "a+");

    if (NULL == file)
    {
        printf("Unalbe to open the rfid file");
        fclose(file);
        return -1;
    }

    // fprintf(file,"0083e19f");
    // fprintf(file,"\n");
    // fprintf(file,"066FEA18");
    // fprintf(file,"\n");
    // fprintf(file,"0C1C04A8");
    // fprintf(file,"\n");
    fprintf(file, "aabbccdd");
    fprintf(file, "\n");
    fflush(file);
    // return 0;
    // Move the file position to the beginning for reading
    rewind(file);

    char line[20];
    char idtag[] = "aabbccdd";
    char idTagValue[20]={0};
    strcpy(idTagValue,idtag);
    idTagValue[strlen(idtag)] = '\n';
     // Open a temporary file for writing
    FILE *outputFile = fopen("temp.txt", "w");

    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(outputFile);
        return 1;
    }

    // Read each line from the input file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Remove the specific line (e.g., "066FEA18")
        if (strcmp(line, idTagValue) != 0)
        {
            // Write the line to the output file
            fputs(line, outputFile);
        }
    }

    fclose(file);
    fclose(outputFile);

    // Replace the original file with the modified file
    if (rename("temp.txt", RFID_TXT_FILE_NAME) != 0) {
        perror("Error renaming file");
        return 1;
    }

    return 0; // Return 0 for success
}