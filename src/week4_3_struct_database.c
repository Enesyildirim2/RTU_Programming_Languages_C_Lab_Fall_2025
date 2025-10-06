/*
 * week4_3_struct_database.c
 * Author: [Your Name]
 * Student ID: [Your ID]
 * Description:
 *   Simple in-memory "database" using an array of structs.
 *   Students will use malloc to allocate space for multiple Student records,
 *   then input, display, and possibly search the data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Define struct Student with fields name, id, grade
struct Student {
    char name[100];
    int id;
    float grade;
};

int main(void) {
    int n;
    struct Student *students = NULL;

    printf("Enter number of students: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number.\n");
        return 1;
    }

    // TODO: Allocate memory for n Student structs using malloc
    students = malloc(n * sizeof(struct Student));
    

    // TODO: Read student data in a loop
    if (students == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }
    for (int i = 0; i < n; i++) {
        printf("Enter name, id and grade for student %d: ", i + 1);
        scanf("%s %d %f", students[i].name, &students[i].id, &students[i].grade);
    }


    // TODO: Display all student records in formatted output
    printf("records:\n");
    for (int i = 0; i < n; i++) {
        printf("%s %d %.2f\n", students[i].name, students[i].id, students[i].grade);
    }
    // Optional: Compute average grade or find top student
    float t = 0.0;
    for (int i = 0; i < n; i++) {
        t += students[i].grade;
    }
    float average = t / n;
    printf("average: %.2f\n", average);
    // TODO: Free allocated memory
    free(students);
    return 0;
}
