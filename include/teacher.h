#ifndef TEACHER_H_INCLUDED
#define TEACHER_H_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#include "person.h"

class Teacher : public Person {
private:
    std::string title;
    int nrOfClasses;
    std::vector<std::string> students;

public:
    Teacher() : nrOfClasses(0) {}
    
    void setTitle(const std::string& t);
    std::string getTitle() const;
    
    void setNrOfClasses(int n);
    int getNrOfClasses() const;
    
    void insertStudents();
    void outputStudents() const;
};

#endif // TEACHER_H_INCLUDED
