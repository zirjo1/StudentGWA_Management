#ifndef STUDENT_H
#define STUDENT_H

#define MAX_SUBJECTS 9
#define MAX_NAME 50
#define MAX_ID 12  // 10 digits + null terminator

typedef struct {
    char name[MAX_NAME];
    char id[MAX_ID];
    int subjects;
    float scores[MAX_SUBJECTS];
    float units[MAX_SUBJECTS];
    float gwa;
} Student;

void addStudent(Student **students, int *count);
void displayStudents(Student *students, int count);
void searchStudent(Student *students, int count);
void updateStudentScores(Student *students, int count);
void deleteStudent(Student **students, int *count);
int saveToFile(Student *students, int count, const char *file);
int loadFromFile(Student **students, const char *file);

void pauseScreen(void);
void clearScreen(void);

#endif
