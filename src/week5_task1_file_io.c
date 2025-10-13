// week5_task1_file_io.c
// Task 1: Read and write data from text files
// Week 5 – Files & Modular Programming
// TODO: Fill in the missing parts marked below.

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *fp;
    char filename[100]  = "data.txt";
    char line[256];
    // TODO: 1. Open file for writing (mode = "w")
    fp = fopen(filename, "w");
    // TODO: 2. Check if file opened successfully
    if (fp == NULL) {
        perror("Cannot open file");
        return 1;
    }
    // TODO: 3. Write 2–3 lines of text to the file using fprintf()
    fprintf(fp, "This is my computer\n");
    fprintf(fp, "My name is Enes\n");
    fprintf(fp, "My surname is YILDIRIM\n");
    // TODO: 4. Close the file
    fclose(fp);

    // TODO: 5. Open file again for reading (mode = "r")
    fp = fopen(filename, "r");
    // TODO: 2. Check if file opened successfully
    if (fp == NULL) {
        perror("Cannot open file");
        return 1;
    }
    // TODO: 6. Use fgets() in a loop to read and print each line to the console
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    int line_count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
       printf("%s", line);
        line_count++;
   }
   printf("%d lines\n", line_count);

    // TODO: 7. Close the file
    fclose(fp);
    

    // BONUS: ask user for filename instead of using default "data.txt"
    // BONUS: count number of lines read
    

    

    return 0;
}
