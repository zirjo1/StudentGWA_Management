#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "student.h"

// ------------ Utility Functions ------------

void pauseScreen(void) {
    printf("\nPress ENTER to continue...");
    while (getchar() != '\n');
}

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}






// ------------ GWA Calculation ------------
float calculateGWA(Student *s) {
    float totalWeighted = 0, totalUnits = 0;
    for (int i = 0; i < s->subjects; i++) {
        totalWeighted += s->scores[i] * s->units[i];
        totalUnits += s->units[i];
    }
    return (totalUnits > 0) ? totalWeighted / totalUnits : 0;
}






// ------------ Check duplicate ID ------------ 
int idExists(Student *students, int count, const char *id) {
    for (int i = 0; i < count; i++)
        if (strcmp(students[i].id, id) == 0)
            return 1;
    return 0;
}













// ------------ Add Student ------------ 
void addStudent(Student **students, int *count) {
    Student *temp = realloc(*students, (*count + 1) * sizeof(Student));
    if (!temp) { printf("Memory error!\n"); return; }
    *students = temp;

    Student *s = &(*students)[*count];

    printf("Enter name: ");
    fgets(s->name, MAX_NAME, stdin);
    s->name[strcspn(s->name, "\n")] = 0;

    
    
    
    
    
    
    
    // 10 Digit & Dupe Prevention
    do {
        printf("Enter Student ID: ");
        fgets(s->id, MAX_ID, stdin);
        s->id[strcspn(s->id, "\n")] = 0;

        if (strlen(s->id) != 10 || strspn(s->id, "0123456789") != 10)
            printf("Only 10 Digit ID #.\n");
        else if (idExists(*students, *count, s->id))
            printf("ID already exists.\n");
        else
            break;
    } while (1);

    do {
        printf("Enter number of subjects (1-%d): ", MAX_SUBJECTS);
        scanf("%d", &s->subjects);
        while (getchar() != '\n');
    } while (s->subjects < 1 || s->subjects > MAX_SUBJECTS);

    for (int i = 0; i < s->subjects; i++) {
        printf("Score for subject %d: ", i + 1);
        scanf("%f", &s->scores[i]);
        printf("Units for subject %d: ", i + 1);
        scanf("%f", &s->units[i]);
        while (getchar() != '\n');
    }

    s->gwa = calculateGWA(s);
    (*count)++;

    printf("\nStudent added!\n");
    pauseScreen();
}







// ------------ Display Students ------------ 
void displayStudents(Student *students, int count) {
    if (count == 0) {
        printf("\nNo records found.\n");
        return pauseScreen();
    }

    for (int i = 0; i < count; i++) {
        printf("\n[%d] %s | ID: %s\nGWA: %.2f\n", 
            i + 1, students[i].name, students[i].id, students[i].gwa);
        for (int j = 0; j < students[i].subjects; j++)
            printf("  (%d) Score: %.2f | Units: %.2f\n",
                j+1, students[i].scores[j], students[i].units[j]);
        printf("----------------------------------\n");
    }
    pauseScreen();
}









// ------------ Search Student ------------
void searchStudent(Student *students, int count) {
    char id[MAX_ID];
    printf("Enter ID to search: ");
    fgets(id, MAX_ID, stdin);
    id[strcspn(id, "\n")] = 0;

    for (int i = 0; i < count; i++) {
        if (strcmp(id, students[i].id) == 0) {
            printf("\nFOUND: %s  (GWA: %.2f)\n", students[i].name, students[i].gwa);
            return pauseScreen();
        }
    }
    printf("\nStudent not found / No Record.\n");
    pauseScreen();
}









// ------------ Update Scores ------------ 
void updateStudentScores(Student *students, int count) {
    char id[MAX_ID];
    printf("Enter ID to update student grade: ");
    fgets(id, MAX_ID, stdin);
    id[strcspn(id, "\n")] = 0;

    for (int i = 0; i < count; i++) {
        if (strcmp(id, students[i].id) == 0) {
            Student *s = &students[i];
            for (int j = 0; j < s->subjects; j++) {
                printf("Subject %d current score %.2f units %.2f\n",
                       j+1, s->scores[j], s->units[j]);
                printf("Update score (-1 skip): ");
                float sc; scanf("%f", &sc);
                if (sc >= 0) s->scores[j] = sc;
                printf("Update units: ");
                float u; scanf("%f", &u);
                if (u >= 0) s->units[j] = u;
                while (getchar() != '\n');
            }
            s->gwa = calculateGWA(s);
            printf("\nUpdated successfully.\n");
            return pauseScreen();
        }
    }
    printf("Student not found / No Record.\n");
    pauseScreen();
}










// ------------ Delete Student ------------ 
void deleteStudent(Student **students, int *count) {
    char id[MAX_ID];
    printf("Enter ID to delete: ");
    fgets(id, MAX_ID, stdin);
    id[strcspn(id, "\n")] = 0;

    for (int i = 0; i < *count; i++) {
        if (strcmp(id, (*students)[i].id) == 0) {
            for (int j = i; j < *count - 1; j++)
                (*students)[j] = (*students)[j+1];
            (*count)--;
            *students = realloc(*students, (*count)*sizeof(Student));
            printf("\nRecord removed.\n");
            return pauseScreen();
        }
    }
    printf("\nNot found.\n");
    pauseScreen();
}












// ------------ File Save/Load ------------
int saveToFile(Student *students, int count, const char *file) {
    FILE *fp = fopen(file, "w");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(fp,"%s;%s;%d;%.2f;", students[i].name, students[i].id,
                students[i].subjects, students[i].gwa);
        for (int j = 0; j < students[i].subjects; j++)
            fprintf(fp,"%.2f,%.2f;", students[i].scores[j], students[i].units[j]);
        fprintf(fp,"\n");
    }
    fclose(fp);
    return 1;
}

int loadFromFile(Student **students, const char *file) {
    FILE *fp = fopen(file, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        Student s = {0};
        char *token = strtok(line, ";");
        strncpy(s.name, token, MAX_NAME);
        token = strtok(NULL, ";"); strncpy(s.id, token, MAX_ID);
        token = strtok(NULL, ";"); s.subjects = atoi(token);
        token = strtok(NULL, ";"); s.gwa = atof(token);

        for (int j = 0; j < s.subjects; j++) {
            token = strtok(NULL, ";");
            sscanf(token, "%f,%f", &s.scores[j], &s.units[j]);
        }

        Student *temp = realloc(*students, (count + 1)*sizeof(Student));
        *students = temp;
        (*students)[count++] = s;
    }
    fclose(fp);
    return count;
}
