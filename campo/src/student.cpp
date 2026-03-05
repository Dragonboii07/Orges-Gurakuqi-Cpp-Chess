#include "../include/student.h"
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

void Student::setFaculty(const string& f) {
    faculty = f;
}

string Student::getFaculty() const {
    return faculty;
}

void Student::setCls(const std::string& c) {
    cls = c;
}

std::string Student::getCls() const {
    return cls;
}

void Student::pushData() {
    int n;
    double grade;
    
    cout << "\n--- GERMAN GRADE ENTRY SYSTEM ---" << endl;
    cout << "(enter notes on a scale from 1.0 [best] to 6.0 [worst])" << endl;
    
    // ensure user enters a positive number of grades
    do {
        cout << "\nHow many grades do you want to enter? ";
        cin >> n;
        cin.ignore();
        if (n <= 0) {
            cout << "Invalid input. Number of grades must be positive." << endl;
        }
    } while (n <= 0);

    cout << "\nEnter your grades below:" << endl;
    cout << "─────────────────────────────────────────" << endl;
    
    for (int i = 0; i < n; i++) {
        // each grade must be between 1.0 and 6.0
        do {
            cout << "  Grade " << (i + 1) << " (1.0-6.0): ";
            if (!(cin >> grade)) {
                if (cin.eof()) {
                    cout << "\nEnd of input encountered. Aborting grade entry." << endl;
                    return;
                }
                // non-numeric or other failure; clear and discard rest of line
                cin.clear();
                cin.ignore(10000, '\n');
                grade = 0; // force loop retry
                cout << "  Please enter a numeric value." << endl;
            } else if (grade < 1.0 || grade > 6.0) {
                cout << "  Please enter a value between 1.0 and 6.0." << endl;
            }
        } while (grade < 1.0 || grade > 6.0);
        grades.push_back(grade);
        cin.ignore(10000, '\n'); // consume leftover newline
    }


    cout << "\n--- GRADE SUMMARY REPORT ---" << endl;
    
    double sum = 0;
    for (double g : grades) {
        sum += g;
    }
    double average = grades.empty() ? 0.0 : sum / grades.size();

    auto descriptor = [&](double g) {
        if (g <= 1.5) return string("Sehr gut");
        if (g <= 2.5) return string("Gut");
        if (g <= 3.5) return string("Befriedigend");
        if (g <= 4.0) return string("Ausreichend");
        if (g <= 5.0) return string("Mangelhaft");
        return string("Ungenügend");
    };

    cout << "\nAll Grades Entered:" << endl;
    for (size_t i = 0; i < grades.size(); i++) {
        double g = grades.at(i);
        cout << "  [" << (i + 1) << "] " << g << " (" << descriptor(g) << ")" << endl;
    }
    
    cout << "\n─────────────────────────────────────────" << endl;
    cout << "  Average Grade: " << average;
    cout << " (" << descriptor(average) << ")" << endl;
    cout << "─────────────────────────────────────────" << endl << endl;
}

void Student::outputData(const vector<double>& v) const {
    for (size_t i = 0; i < v.size(); i++) {
        cout << v.at(i) << " ";
    }
    cout << endl;
}
