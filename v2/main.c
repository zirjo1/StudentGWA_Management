#include <stdio.h>
#include <stdlib.h>
#include "student.h"

int main() {
    Student *students = NULL;
    int count = loadFromFile(&students, "students.txt");

    while (1) {
        clearScreen();
        printf("\n--- STUDENT GRADE MANAGEMENT SYSTEM ---\n");
        printf("[1] Add Student\n");
        printf("[2] Display Students\n");
        printf("[3] Search Student\n");
        printf("[4] Update Scores\n");
        printf("[5] Delete Student\n");
        printf("[6] Save & Exit\n\n");
        printf("Choice: ");

        int choice;
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        while (getchar()!='\n');

        switch (choice) {
            case 1: addStudent(&students, &count); break;
            case 2: displayStudents(students, count); break;
            case 3: searchStudent(students, count); break;
            case 4: updateStudentScores(students, count); break;
            case 5: deleteStudent(&students, &count); break;
            case 6:
                if (saveToFile(students, count, "students.txt"))
                    printf("\nSaved successfully!\n");
                else
                    printf("\nFailed to save data.\n");
                pauseScreen();
                free(students);
                return 0;
            default:
                printf("\nInvalid choice.\n");
                pauseScreen();
        }
    }
}
