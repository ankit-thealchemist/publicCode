#include <stdio.h>
#include <stdint.h>
#include <cjson/cJSON.h>



int main(int argc, char *argv[]) {
    
    FILE *file_ptr = fopen("config.json","r");
      if (file_ptr == NULL) {
        fprintf(stderr, "Error opening file:\n");
        return 1;
    }
    rewind(file_ptr);
    fseek(file_ptr,0,SEEK_END);
    size_t file_size = ftell(file_ptr);

    rewind(file_ptr);

    char buffer[1000] ={0};

    fread(buffer,file_size,1,file_ptr);
    fclose(file_ptr);

    file_ptr = fopen("config-2.json","r");
      if (file_ptr == NULL) {
        fprintf(stderr, "Error opening file:\n");
        return 1;
    }
    rewind(file_ptr);
    fseek(file_ptr,0,SEEK_END);
    file_size = ftell(file_ptr);

    rewind(file_ptr);

    char buffer_2[1000] ={0};

    fread(buffer_2,file_size,1,file_ptr);
    fclose(file_ptr);

    cJSON *replace_obj = cJSON_Parse(buffer_2);

    cJSON *root = cJSON_Parse(buffer);

    cJSON_ReplaceItemInObject(root,"development",replace_obj);
    printf("%s\r\n",cJSON_Print(root));


    cJSON_Delete(root);

    return 0;  // Return 0 for success
}