#ifndef PERSON_H_INCLUDED
#define PERSON_H_INCLUDED

#include <iostream>
#include <string>

class Person {
private:
    std::string name;
    std::string surname;
    int age;

public:
    Person() : age(0) {}
    virtual ~Person() = default;

    void setName(const std::string& n);
    std::string getName() const;
    
    void setSurname(const std::string& s);
    std::string getSurname() const;
    
    void setAge(int a);
    int getAge() const;
};

#endif // PERSON_H_INCLUDED
