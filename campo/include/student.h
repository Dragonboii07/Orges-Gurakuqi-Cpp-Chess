#ifndef STUDENT_H_INCLUDED
#define STUDENT_H_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#include "person.h"

class Student : public Person {
private:
    std::string faculty;
    std::string cls;
    std::vector<double> grades;

public:
    Student() {}
    
    void setFaculty(const std::string& f);
    std::string getFaculty() const;
    
    void setCls(const std::string& c);
    std::string getCls() const;
    void pushData();
    void outputData(const std::vector<double>& v) const;
};

#endif // STUDENT_H_INCLUDED
