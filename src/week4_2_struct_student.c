/*
 * week4_2_struct_student.c
 * Author: ENES YILDIRIM
 * Student ID: 211ADB121
 * Description:
 *   Demonstrates defining and using a struct in C.
 *   Students should define a 'Student' struct with fields like name, id, and grade.
 *   Then create a few instances and print them.
 */

#include <stdio.h>
#include <string.h>

// TODO: Define struct Student with fields: name (char[]), id (int), grade (float)
// Example:
 struct Student {
     char name[50];
     int id;
     float grade;
 };

int main(void) {
    // TODO: Declare one or more Student variables
    struct Student student1;
    struct Student student2;

    // TODO: Assign values (either manually or via scanf)
    strcpy(student1.name, "Enes");
    student1.id = 211121;
    student1.grade = 75.2;
    strcpy(student2.name, "Jack");
    student2.id = 123124;
    student2.grade = 79.9;

    // TODO: Print struct contents using printf
    printf("%s %d %.2f\n", student1.name, student1.id, student1.grade);
    printf("%s %d %.2f\n", student2.name, student2.id, student2.grade);
    return 0;
}
