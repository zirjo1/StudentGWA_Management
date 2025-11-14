#include <stdio.h>        // Standard input/output
#include <math.h>         // Math functions (not used)
#include <stdbool.h>      // Boolean type support
#include <string.h>       // String manipulation
#include <stdlib.h>       // Memory allocation and exit
#include <time.h>         // Time functions (not used)
#include <ctype.h>        // Character classification (not used)

#define MAX_SUBJECTS 9     // Maximum number of subjects per student

// Student data structure
typedef struct {
    char name[50];              // Student name
    char ID[20];                // Student ID
    float gwa;                  // Computed GWA value (formerly GPA)
    int subjects;               // Number of subjects taken
    float scores[MAX_SUBJECTS]; // Scores per subject
    float units[MAX_SUBJECTS];  // Units per subject
} Student;

// Function prototypes
void addStudent(Student **students, int *count);
void displayStudents(Student *students, int count);
void calculateGWA(Student *student);
void searchStudent(Student *students, int count, char *id);
void deleteStudent(Student **students, int *count, char *id);
void updateStudentScores(Student *students, int count);
void saveToFile(Student *students, int count, const char *filename);
int loadFromFile(Student **students, const char *filename);

int main() {
    Student *students = NULL;  // Dynamic student array
    int count = loadFromFile(&students, "students.txt");  // Load previous data
    int choice;

    do {
        printf("\n*** Student Grading System ***\n");
        printf("1. Add student\n2. Display students\n3. Search student\n4. Delete student\n5. Update scores\n6. Save & Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        while (getchar() != '\n');

        switch (choice) {
            case 1: addStudent(&students, &count); break;
            case 2: displayStudents(students, count); break;
            case 3: {
                char id[20];
                printf("Enter student ID to search: ");
                scanf(" %[^\n]", id);
                searchStudent(students, count, id);
                break;
            }
            case 4: {
                char id[20];
                printf("Enter student ID to delete: ");
                scanf(" %[^\n]", id);
                deleteStudent(&students, &count, id);
                break;
            }
            case 5: updateStudentScores(students, count); break;
            case 6: saveToFile(students, count, "students.txt"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 6);

    free(students);
    return 0;
}

// Adds a new student to the dynamic array
void addStudent(Student **students, int *count) {
    Student *temp = realloc(*students, (*count + 1) * sizeof(Student));
    if (!temp) { printf("Memory allocation failed.\n"); exit(1); }
    *students = temp;

    Student *s = &(*students)[*count];

    printf("Enter name: ");
    fgets(s->name, sizeof(s->name), stdin);
    s->name[strcspn(s->name, "\n")] = 0;

    printf("Enter student ID: ");
    fgets(s->ID, sizeof(s->ID), stdin);
    s->ID[strcspn(s->ID, "\n")] = 0;

    do {
        printf("Enter number of subjects (1-%d): ", MAX_SUBJECTS);
        scanf("%d", &s->subjects);
        while (getchar() != '\n');
    } while (s->subjects < 1 || s->subjects > MAX_SUBJECTS);

    for (int i = 0; i < s->subjects; i++) {
        printf("Enter score for subject %d: ", i + 1);
        scanf("%f", &s->scores[i]);
        printf("Enter units for subject %d: ", i + 1);
        scanf("%f", &s->units[i]);
    }

    for (int i = s->subjects; i < MAX_SUBJECTS; i++) {
        s->scores[i] = 0;
        s->units[i] = 0;
    }

    calculateGWA(s);

    (*count)++;
    printf("Student added successfully!\n");
}

// Computes GWA using weighted average
void calculateGWA(Student *student) {
    float totalWeighted = 0, totalUnits = 0;
    for (int i = 0; i < student->subjects; i++) {
        totalWeighted += student->scores[i] * student->units[i];
        totalUnits += student->units[i];
    }
    student->gwa = totalUnits > 0 ? totalWeighted / totalUnits : 0;
}

// Displays all students
void displayStudents(Student *students, int count) {
    if (count == 0) {
        printf("No students to display.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("\nName: %s\nID: %s\nGWA: %.2f\n", students[i].name, students[i].ID, students[i].gwa);
        printf("Subjects: %d\n", students[i].subjects);
        for (int j = 0; j < students[i].subjects; j++) {
            printf("  Subject %d -> Score: %.2f | Units: %.2f\n", j + 1, students[i].scores[j], students[i].units[j]);
        }
        printf("----------------------\n");
    }
}

// Searches for a student by ID
void searchStudent(Student *students, int count, char *id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(students[i].ID, id) == 0) {
            printf("\nStudent found:\n");
            displayStudents(&students[i], 1);
            return;
        }
    }
    printf("Student with ID %s not found.\n", id);
}

// Deletes a student by shifting
void deleteStudent(Student **students, int *count, char *id) {
    if (*count == 0) { printf("No students to delete.\n"); return; }

    int index = -1;
    for (int i = 0; i < *count; i++)
        if (strcmp((*students)[i].ID, id) == 0) index = i;

    if (index == -1) {
        printf("Student with ID %s not found.\n", id);
        return;
    }

    for (int i = index; i < *count - 1; i++)
        (*students)[i] = (*students)[i + 1];

    (*count)--;
    *students = realloc(*students, (*count) * sizeof(Student));

    printf("Student deleted successfully.\n");
}

// Updates scores
void updateStudentScores(Student *students, int count) {
    if (count == 0) { printf("No students to update.\n"); return; }

    char id[20];
    printf("Enter student ID to update: ");
    scanf(" %[^\n]", id);

    int index = -1;
    for (int i = 0; i < count; i++)
        if (strcmp(students[i].ID, id) == 0) index = i;

    if (index == -1) {
        printf("Student not found.\n");
        return;
    }

    Student *s = &students[index];
    printf("Updating scores for %s\n", s->name);

    for (int i = 0; i < s->subjects; i++) {
        printf("Subject %d current score: %.2f | units: %.2f\n", i + 1, s->scores[i], s->units[i]);

        printf("Enter new score (or -1 to skip): ");
        float newScore;
        scanf("%f", &newScore);
        if (newScore >= 0) s->scores[i] = newScore;

        printf("Enter new units (or -1 to skip): ");
        float newUnits;
        scanf("%f", &newUnits);
        if (newUnits >= 0) s->units[i] = newUnits;
    }

    calculateGWA(s);
    printf("Updated GWA: %.2f\n", s->gwa);
}

// Saves all student data to file
void saveToFile(Student *students, int count, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("Error saving file.\n"); return; }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s;%s;%d;%.2f;", students[i].name, students[i].ID,
                students[i].subjects, students[i].gwa);
        for (int j = 0; j < MAX_SUBJECTS; j++)
            fprintf(fp, "%.2f,%.2f;", students[i].scores[j], students[i].units[j]);
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Saved successfully to %s.\n", filename);
}

// Loads student data
int loadFromFile(Student **students, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("No saved file found. Starting empty.\n");
        *students = NULL;
        return 0;
    }

    char line[512];
    int count = 0;
    *students = NULL;

    while (fgets(line, sizeof(line), fp)) {
        Student s;
        memset(&s, 0, sizeof(Student));

        char *token = strtok(line, ";");
        if (!token) continue;
        strncpy(s.name, token, sizeof(s.name)-1);

        token = strtok(NULL, ";"); if (!token) continue;
        strncpy(s.ID, token, sizeof(s.ID)-1);

        token = strtok(NULL, ";"); if (!token) continue;
        s.subjects = atoi(token);

        token = strtok(NULL, ";");
        s.gwa = token ? atof(token) : 0;

        for (int j = 0; j < MAX_SUBJECTS; j++) {
            token = strtok(NULL, ";");
            if (!token) break;
            sscanf(token, "%f,%f", &s.scores[j], &s.units[j]);
        }

        Student *temp = realloc(*students, (count + 1) * sizeof(Student));
        if (!temp) { printf("Memory error.\n"); exit(1); }
        *students = temp;

        (*students)[count++] = s;
    }

    fclose(fp);
    printf("Loaded %d students from %s.\n", count, filename);
    return count;
}
