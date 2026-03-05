#include <iostream>
#include <vector>
#include <locale>
#include <limits>
#include "../include/person.h"
#include "../include/student.h"
#include "../include/teacher.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ---------------------------------------------------------------------------
// helper functions to break main into logical pieces
// ---------------------------------------------------------------------------

static void setUtf8Console() {
#ifdef _WIN32
    // switch Windows console to UTF‑8 so output renders properly
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

static char promptUserType() {
    char answer;
    cout << "\nAre you a student (S) or a teacher (T)? ";
    cin >> answer;
    while (answer != 's' && answer != 'S' && answer != 't' && answer != 'T') {
        cout << "Please type 'S' for student or 'T' for teacher: ";
        cin >> answer;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return answer;
}

static bool askContinue() {
    char again;
    cout << "\nWould you like to perform another operation? (Y/N): ";
    cin >> again;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return (again == 'Y' || again == 'y');
}

static void handleStudent() {
    string fullname, faculty, cls;
    int age;
    Student s;

    cout << "\n--- STUDENT INFORMATION FORM ---" << endl;
    cout << "Full Name: ";
    getline(cin, fullname);
    cout << "Faculty: ";
    getline(cin, faculty);
    cout << "Class/Grade: ";
    getline(cin, cls);

    do {
        cout << "Age: ";
        if (!(cin >> age)) {
            cin.clear();
            age = -1;
        }
        if (age <= 0) cout << "Age must be a positive integer." << endl;
    } while (age <= 0);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    s.setName(fullname);
    s.setAge(age);
    s.setFaculty(faculty);
    s.setCls(cls);

    cout << "\n--- INFORMATION SUMMARY ---" << endl;
    cout << "Name: " << fullname << "\n"
         << "Faculty: " << faculty << "\n"
         << "Class: " << cls << "\n"
         << "Age: " << age << endl;

    s.pushData();
}

static void handleTeacher() {
    string fullname, title;
    int age, nocls;
    Teacher t;

    cout << "\n--- TEACHER INFORMATION FORM ---" << endl;
    cout << "Full Name: ";
    getline(cin, fullname);
    cout << "Title (Professor/Dr./Instructor): ";
    getline(cin, title);

    do {
        cout << "Age: ";
        if (!(cin >> age)) { cin.clear(); age = -1; }
        if (age <= 0) cout << "Age must be a positive integer." << endl;
    } while (age <= 0);

    do {
        cout << "Number of Classes: ";
        if (!(cin >> nocls)) { cin.clear(); nocls = -1; }
        if (nocls < 0) cout << "Number of classes cannot be negative." << endl;
    } while (nocls < 0);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    t.setName(fullname);
    t.setAge(age);
    t.setTitle(title);
    t.setNrOfClasses(nocls);

    cout << "\n--- TEACHER INFORMATION SUMMARY ---" << endl;
    cout << "Name: " << fullname << "\n"
         << "Title: " << title << "\n"
         << "Age: " << age << "\n"
         << "Classes: " << nocls << endl;

    cout << "\n--- APPROVED STUDENTS REGISTRY ---" << endl;
    cout << "(Only students with German grade 1-4 will be approved)\n";
    t.insertStudents();

    cout << "\n--- APPROVED STUDENTS LIST ---" << endl;
    t.outputStudents();
}

// ---------------------------------------------------------------------------
// entry point
// ---------------------------------------------------------------------------

int main() {
    setUtf8Console();

    cout << "========================================" << endl;
    cout << "   Student & Teacher Management System  " << endl;
    cout << "========================================" << endl;
    cout << "A simple CLI demo with friendly prompts" << endl;

    bool cont = true;
    while (cont) {
        char answer = promptUserType();
        if (answer == 's' || answer == 'S') {
            handleStudent();
        } else {
            handleTeacher();
        }
        cont = askContinue();
    }

    cout << "\nThank you for using the program. Goodbye!" << endl;
    return 0;
}
