/*
 * week4_1_dynamic_array.c
 * Author: ENES YILDIRIM
 * Student ID: 211ADB121
 * Description:
 *   Demonstrates creation and usage of a dynamic array using malloc.
 *   Students should allocate memory for an integer array, fill it with data,
 *   compute something (e.g., average), and then free the memory.
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n;
    int *arr = NULL;

    printf("Enter number of elements: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid size.\n");
        return 1;
    }

    // TODO: Allocate memory for n integers using malloc
    // Example: arr = malloc(n * sizeof(int));
    arr = malloc(n * sizeof(int));
    // TODO: Check allocation success
    if (arr == NULL) {
        printf("Memory allocation failed.");
        return 1;
    }
    // TODO: Read n integers from user input and store in array
    printf("Enter %d numbers:",n);
    // TODO: Compute sum and average
    int summary = 0;
    for (int i = 0; i < n; i++){
        scanf("%d", &arr[i]);
        summary += arr[i];
    }
    int average = summary / n;
    // TODO: Print the results
    printf("Summary: %d, Average: %d", summary, average);
    // TODO: Free allocated memory
    free(arr);
    return 0;
}
