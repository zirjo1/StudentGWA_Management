// Save/replace your grading_gui.c with this content

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SUBJECTS 9

typedef struct {
    char name[50];
    char ID[20];
    int subjects;
    float grades[MAX_SUBJECTS];
    float units[MAX_SUBJECTS];
    float gpa;
    char remarks[20];
} Student;

Student *students = NULL;
int student_count = 0;

GtkWidget *list_box;
GtkWidget *entry_name, *entry_id, *entry_subjects;
GtkWidget *grade_boxes[MAX_SUBJECTS];
GtkWidget *unit_entries[MAX_SUBJECTS];
GtkWidget *subject_rows[MAX_SUBJECTS];

// ---------- Helpers ----------
void assign_remarks(Student *s) {
    // robust remarks assignment
    if (s->gpa <= 2.75f) {
        strcpy(s->remarks, "Passed");
    } else if (s->gpa >= 3.00f && s->gpa <= 3.50f) {
        strcpy(s->remarks, "Conditional");
    } else if (s->gpa >= 3.75f) {
        strcpy(s->remarks, "Failed");
    } else {
        // e.g. 2.76-2.99 or 3.51-3.74 -> treat as Passed by your rule
        strcpy(s->remarks, "Passed");
    }
}

void calculateGPA(Student *s) {
    float total_units = 0.0f, weighted = 0.0f;
    for (int i = 0; i < s->subjects; i++) {
        weighted += s->grades[i] * s->units[i];
        total_units += s->units[i];
    }
    s->gpa = (total_units > 0.0f) ? (weighted / total_units) : 0.0f;
    assign_remarks(s);
}

// ---------- Save & Load ----------
void saveToFile() {
    FILE *fp = fopen("students.txt", "w");
    if (!fp) return;
    for (int i = 0; i < student_count; i++) {
        Student *s = &students[i];
        fprintf(fp, "%s;%s;%d;%.2f;%s;", s->name, s->ID, s->subjects, s->gpa, s->remarks);
        for (int j = 0; j < s->subjects; j++)
            fprintf(fp, "%.2f,%.2f;", s->grades[j], s->units[j]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void loadFromFile() {
    FILE *fp = fopen("students.txt", "r");
    if (!fp) return;
    free(students);
    students = NULL;
    student_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        Student s = {0};
        // read name;ID;subjects;gpa;remarks;
        // use temporary parsing to find start of grade/unit blocks
        char namebuf[50] = {0}, idbuf[20] = {0}, remarksbuf[20] = {0};
        int subs = 0;
        float gpa = 0.0f;
        if (sscanf(line, "%49[^;];%19[^;];%d;%f;%19[^;];", namebuf, idbuf, &subs, &gpa, remarksbuf) >= 4) {
            strncpy(s.name, namebuf, sizeof(s.name)-1);
            strncpy(s.ID, idbuf, sizeof(s.ID)-1);
            s.subjects = (subs < 1) ? 1 : ((subs > MAX_SUBJECTS) ? MAX_SUBJECTS : subs);
            s.gpa = gpa;
            strncpy(s.remarks, remarksbuf, sizeof(s.remarks)-1);

            // find start of 5th semicolon
            char *p = line;
            for (int k = 0, found = 0; p && k < 5; k++) {
                p = strchr(p, ';');
                if (p) p++;
                else { found = 0; break; }
                if (k == 4) { /* now p points to after 5th semicolon */ }
            }
            // simpler approach: tokenise after the 5th semicolon
            char *copy = strdup(line);
            char *tok = strtok(copy, ";"); // 1 name
            int tok_index = 0;
            while (tok) {
                tok_index++;
                if (tok_index == 5) {
                    // next tokens are grade,unit ; grade,unit ; ...
                    char *rest = strtok(NULL, ";");
                    int idx = 0;
                    while (rest && idx < MAX_SUBJECTS) {
                        float gr = 0.0f, un = 0.0f;
                        sscanf(rest, "%f,%f", &gr, &un);
                        s.grades[idx] = gr;
                        s.units[idx] = un;
                        idx++;
                        rest = strtok(NULL, ";");
                    }
                    break;
                }
                tok = strtok(NULL, ";");
            }
            free(copy);
        }
        students = realloc(students, (student_count + 1) * sizeof(Student));
        students[student_count++] = s;
    }
    fclose(fp);
}

// ---------- UI Refresh ----------
void refresh_list() {
    // clear children then insert labels with GPA (2 decimals) and remarks
    GList *children = gtk_container_get_children(GTK_CONTAINER(list_box));
    for (GList *it = children; it != NULL; it = g_list_next(it))
        gtk_widget_destroy(GTK_WIDGET(it->data));
    g_list_free(children);

    for (int i = 0; i < student_count; i++) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s (%s) — GPA: %.2f — %s",
                 students[i].name, students[i].ID, students[i].gpa, students[i].remarks);
        GtkWidget *lbl = gtk_label_new(buf);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
        gtk_list_box_insert(GTK_LIST_BOX(list_box), lbl, -1);
    }
    gtk_widget_show_all(list_box);
}

// ---------- Add Student ----------
void on_add_student(GtkWidget *button, gpointer user_data) {
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const char *subj_text = gtk_entry_get_text(GTK_ENTRY(entry_subjects));
    int subjects = atoi(subj_text);

    if (strlen(name) == 0 || strlen(id) == 0 || subjects < 1 || subjects > MAX_SUBJECTS) {
        GtkWidget *d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Please fill out Name, ID and 1–%d for subjects.", MAX_SUBJECTS);
        gtk_dialog_run(GTK_DIALOG(d));
        gtk_widget_destroy(d);
        return;
    }

    Student s = {0};
    strncpy(s.name, name, sizeof(s.name)-1);
    strncpy(s.ID, id, sizeof(s.ID)-1);
    s.subjects = subjects;

    for (int i = 0; i < subjects; i++) {
        gchar *gtext = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(grade_boxes[i]));
        const char *unit_text = gtk_entry_get_text(GTK_ENTRY(unit_entries[i]));
        if (!gtext || strlen(unit_text) == 0) {
            if (gtext) g_free(gtext);
            GtkWidget *d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Please select a grade and enter units for each subject.");
            gtk_dialog_run(GTK_DIALOG(d));
            gtk_widget_destroy(d);
            return;
        }
        s.grades[i] = atof(gtext);
        s.units[i] = atof(unit_text);
        g_free(gtext);
    }

    calculateGPA(&s);

    students = realloc(students, (student_count + 1) * sizeof(Student));
    students[student_count++] = s;

    saveToFile();
    refresh_list();

    GtkWidget *d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Student added.\nGPA: %.2f — %s", s.gpa, s.remarks);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

// ---------- Remove selected student (existing) ----------
void on_remove_selected(GtkWidget *button, gpointer user_data) {
    GtkListBoxRow *row = gtk_list_box_get_selected_row(GTK_LIST_BOX(list_box));
    if (!row) {
        GtkWidget *d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
            "Select a student in the list first.");
        gtk_dialog_run(GTK_DIALOG(d));
        gtk_widget_destroy(d);
        return;
    }
    int idx = gtk_list_box_row_get_index(row);
    if (idx < 0 || idx >= student_count) return;
    // confirm
    GtkWidget *conf = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Delete student %s (%s)?", students[idx].name, students[idx].ID);
    gint res = gtk_dialog_run(GTK_DIALOG(conf));
    gtk_widget_destroy(conf);
    if (res != GTK_RESPONSE_YES) return;

    for (int i = idx; i < student_count - 1; i++)
        students[i] = students[i+1];
    student_count--;
    students = realloc(students, (student_count > 0) ? (student_count * sizeof(Student)) : 0);
    saveToFile();
    refresh_list();
}

// ---------- Search by ID ----------
void on_search_by_id(GtkWidget *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Search Student by ID",
        NULL, GTK_DIALOG_MODAL, "Close", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter student ID");
    gtk_container_add(GTK_CONTAINER(content), entry);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_CLOSE) {
        gtk_widget_destroy(dialog);
        return;
    }
    // user closed, but we handle with manual button below (we instead add a separate "Find" button)
    gtk_widget_destroy(dialog);
}

// helper: show details popup for given student index
void show_student_details(int idx) {
    if (idx < 0 || idx >= student_count) return;
    Student *s = &students[idx];
    GString *msg = g_string_new(NULL);
    g_string_printf(msg, "Name: %s\nID: %s\nGPA: %.2f — %s\nSubjects: %d\n",
                    s->name, s->ID, s->gpa, s->remarks, s->subjects);
    for (int i = 0; i < s->subjects; i++) {
        g_string_append_printf(msg, "  Subj %d: Grade %.2f, Units %.2f\n", i+1, s->grades[i], s->units[i]);
    }
    GtkWidget *d = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg->str);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
    g_string_free(msg, TRUE);
}

// real search dialog that runs 'Find' button inside
void on_search_dialog_run(GtkWidget *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Search Student by ID",
        NULL, GTK_DIALOG_MODAL,
        "Find", GTK_RESPONSE_OK,
        "Close", GTK_RESPONSE_CLOSE,
        NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter student ID");
    gtk_container_add(GTK_CONTAINER(content), entry);
    gtk_widget_show_all(dialog);

    gint resp;
    while ((resp = gtk_dialog_run(GTK_DIALOG(dialog))) != GTK_RESPONSE_CLOSE) {
        if (resp == GTK_RESPONSE_OK) {
            const char *id = gtk_entry_get_text(GTK_ENTRY(entry));
            if (!id || strlen(id) == 0) {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Please enter an ID.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                continue;
            }
            int found = -1;
            for (int i = 0; i < student_count; i++) {
                if (strcmp(students[i].ID, id) == 0) { found = i; break; }
            }
            if (found >= 0) {
                gtk_widget_destroy(dialog);
                show_student_details(found);
                return;
            } else {
                GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Student with ID \"%s\" not found.", id);
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else break;
    }
    gtk_widget_destroy(dialog);
}

// ---------- Delete by ID ----------
void on_delete_by_id(GtkWidget *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Delete Student by ID",
        NULL, GTK_DIALOG_MODAL,
        "Delete", GTK_RESPONSE_OK,
        "Cancel", GTK_RESPONSE_CANCEL,
        NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter student ID to delete");
    gtk_container_add(GTK_CONTAINER(content), entry);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *id = gtk_entry_get_text(GTK_ENTRY(entry));
        if (!id || strlen(id) == 0) {
            GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Please enter an ID.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            gtk_widget_destroy(dialog);
            return;
        }
        int found = -1;
        for (int i = 0; i < student_count; i++) if (strcmp(students[i].ID, id) == 0) { found = i; break; }
        if (found >= 0) {
            // confirm deletion
            GtkWidget *conf = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Delete student %s (%s)?", students[found].name, students[found].ID);
            gint r = gtk_dialog_run(GTK_DIALOG(conf));
            gtk_widget_destroy(conf);
            if (r == GTK_RESPONSE_YES) {
                for (int k = found; k < student_count - 1; k++) students[k] = students[k+1];
                student_count--;
                students = realloc(students, (student_count > 0) ? (student_count * sizeof(Student)) : 0);
                saveToFile();
                refresh_list();
            }
        } else {
            GtkWidget *err = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Student with ID \"%s\" not found.", id);
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        }
    }

    gtk_widget_destroy(dialog);
}

// ---------- Build subject rows ----------
void build_subject_rows(GtkWidget *container) {
    const char *grades[] = {
        "1.00","1.25","1.50","1.75","2.00","2.25","2.50","2.75","3.00",
        "3.25","3.50","3.75","4.00","4.25","4.50","4.75","5.00"
    };
    for (int i = 0; i < MAX_SUBJECTS; i++) {
        subject_rows[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

        grade_boxes[i] = gtk_combo_box_text_new();
        for (int g = 0; g < (int)(sizeof(grades)/sizeof(grades[0])); g++)
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(grade_boxes[i]), grades[g]);
        gtk_combo_box_set_active(GTK_COMBO_BOX(grade_boxes[i]), 0); 

        gtk_box_pack_start(GTK_BOX(subject_rows[i]), gtk_label_new("Grade:"), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(subject_rows[i]), grade_boxes[i], TRUE, TRUE, 0);

        gtk_box_pack_start(GTK_BOX(subject_rows[i]), gtk_label_new("Units:"), FALSE, FALSE, 0);
        unit_entries[i] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(unit_entries[i]), 6);
        gtk_box_pack_start(GTK_BOX(subject_rows[i]), unit_entries[i], FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(container), subject_rows[i], FALSE, FALSE, 0);
    }
}

// ---------- Subject count change ----------
void on_subject_count_change(GtkEditable *editable, gpointer user_data) {
    const char *txt = gtk_entry_get_text(GTK_ENTRY(entry_subjects));
    int subjects = atoi(txt);
    if (subjects < 1) subjects = 1;
    if (subjects > MAX_SUBJECTS) subjects = MAX_SUBJECTS;
    for (int i = 0; i < MAX_SUBJECTS; i++)
        gtk_widget_set_visible(subject_rows[i], i < subjects);
}

// ---------- Main ----------
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    loadFromFile();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Student Grading System");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Student name");
    gtk_box_pack_start(GTK_BOX(vbox), entry_name, FALSE, FALSE, 0);

    entry_id = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_id), "Student ID");
    gtk_box_pack_start(GTK_BOX(vbox), entry_id, FALSE, FALSE, 0);

    entry_subjects = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_subjects), "Number of subjects (1-9)");
    g_signal_connect(entry_subjects, "changed", G_CALLBACK(on_subject_count_change), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), entry_subjects, FALSE, FALSE, 0);

    GtkWidget *subjects_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_pack_start(GTK_BOX(vbox), subjects_container, FALSE, FALSE, 0);
    build_subject_rows(subjects_container);

    GtkWidget *hbuttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *btn_add = gtk_button_new_with_label("Add Student");
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_student), NULL);
    gtk_box_pack_start(GTK_BOX(hbuttons), btn_add, FALSE, FALSE, 0);

    GtkWidget *btn_remove_sel = gtk_button_new_with_label("Remove Selected");
    g_signal_connect(btn_remove_sel, "clicked", G_CALLBACK(on_remove_selected), NULL);
    gtk_box_pack_start(GTK_BOX(hbuttons), btn_remove_sel, FALSE, FALSE, 0);

    GtkWidget *btn_search = gtk_button_new_with_label("Search by ID");
    g_signal_connect(btn_search, "clicked", G_CALLBACK(on_search_dialog_run), NULL);
    gtk_box_pack_start(GTK_BOX(hbuttons), btn_search, FALSE, FALSE, 0);

    GtkWidget *btn_delete_id = gtk_button_new_with_label("Delete by ID");
    g_signal_connect(btn_delete_id, "clicked", G_CALLBACK(on_delete_by_id), NULL);
    gtk_box_pack_start(GTK_BOX(hbuttons), btn_delete_id, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbuttons, FALSE, FALSE, 0);

    GtkWidget *sc = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(sc, -1, 250);
    gtk_box_pack_start(GTK_BOX(vbox), sc, TRUE, TRUE, 0);

    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_SINGLE);
    gtk_container_add(GTK_CONTAINER(sc), list_box);

    refresh_list();
    on_subject_count_change(GTK_EDITABLE(entry_subjects), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    // cleanup
    free(students);
    return 0;
}
