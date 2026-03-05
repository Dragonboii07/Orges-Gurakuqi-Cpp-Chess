#include "../include/teacher.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void Teacher::setTitle(const string& t) {
    title = t;
}

string Teacher::getTitle() const {
    return title;
}

void Teacher::setNrOfClasses(int n) {
    nrOfClasses = n;
}

int Teacher::getNrOfClasses() const {
    return nrOfClasses;
}

void Teacher::insertStudents() {
    int n;
    double average;
    string name;
    // ask for number of students, must be non-negative
    do {
        cout << "How many students are there? ";
        cin >> n;
        if (n < 0) {
            cout << "Number of students cannot be negative." << endl;
        }
    } while (n < 0);
    cin.ignore();
    for (int i = 0; i < n; i++) {
        cout << "Student " << (i+1) << " name: ";
        getline(cin, name);
        
        // German grading system: 1-6, where 1 is best and 6 is worst
        do {
            cout << "German grade for this student (1-6): ";
            cin >> average;
            if (average < 1 || average > 6) {
                cout << "Please enter a valid German grade between 1 and 6." << endl;
            }
        } while (average < 1 || average > 6);
        cin.ignore();

        if (average <= 4) {
            students.push_back(name);
            cout << "  -> " << name << " approved (grade " << average << ")." << endl;
        } else {
            cout << "  -> " << name << " not approved (grade " << average << " - failing)." << endl;
        }
    }
}

void Teacher::outputStudents() const {
    if (students.empty()) {
        cout << "No approved students to list." << endl;
        return;
    }
    cout << "Approved students:" << endl;
    for (size_t i = 0; i < students.size(); ++i) {
        cout << "  " << (i + 1) << ". " << students.at(i) << endl;
    }
}
