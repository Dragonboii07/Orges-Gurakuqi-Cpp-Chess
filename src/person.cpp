#include "../include/person.h"

void Person::setName(const std::string& n) {
    name = n;
}

std::string Person::getName() const {
    return name;
}

void Person::setSurname(const std::string& s) {
    surname = s;
}

std::string Person::getSurname() const {
    return surname;
}

void Person::setAge(int a) {
    age = a;
}

int Person::getAge() const {
    return age;
}
