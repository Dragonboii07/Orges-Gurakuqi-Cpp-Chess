# Orges Gurakuqi — C++ Project

A small C++ demo project that models `Person`, `Student` and `Teacher` classes and provides
**Prerequisites:**
- `g++` (MinGW / MSYS2 / TDM-GCC) installed and available on `PATH` (or use WSL).

# Student & Teacher Management System

A C++ application for managing student and teacher information with dynamic grade and class tracking.

## Features

- **Student Management**: Track student information including name, surname, age, faculty, class, and grades
- **Teacher Management**: Manage teacher details including name, surname, age, title, number of classes, and student list
- **Dynamic Data Entry**: Input and validate student/teacher data at runtime
- **Grade Tracking**: Record and display student grades
- **German Grading System**: Teachers use the German grading scale (1-6, where 1 is best and grades 1-4 are approved)

## Project Structure

```
├── src/
│   ├── main.cpp              # Main application entry point
│   ├── person.cpp            # Base Person class implementation
│   ├── student.cpp           # Student class implementation
│   └── teacher.cpp           # Teacher class implementation
├── include/
│   ├── person.h              # Base Person class definition
│   ├── student.h             # Student class definition
│   └── teacher.h             # Teacher class definition
├── bin/
│   └── Debug/
│       └── project.exe       # Compiled executable
├── obj/
│   └── Debug/                # Object files
├── .vscode/
│   ├── tasks.json            # VS Code build tasks
│   ├── c_cpp_properties.json # IntelliSense configuration
│   └── launch.json           # Debugger configuration
└── README.md                 # This file
```

## Requirements

- **Compiler**: GCC 13.2 or later (via MSYS2)
- **Build Tool**: g++ (MinGW-w64)
- **C++ Standard**: C++17 or later


### Using Command Line

```bash
cd "C:\orges programming\orges c++"

# Build
g++ -g -I./include -o ./bin/Debug/project.exe \
    src/main.cpp src/person.cpp src/student.cpp src/teacher.cpp

# Run
./bin/Debug/project.exe
```

## Class Hierarchy

```
Person (Base Class)
├── Student
└── Teacher
```

## Class Descriptions

### Person
Base class for both Student and Teacher
- Properties: `name`, `surname`, `age`
- Methods: getters and setters for all properties

### Student : public Person
Extends Person class with academic information
- Additional Properties: `faculty`, `class`, `grades` (vector)
- Methods: 
  - `setFaculty()` / `getFaculty()`
  - `setCls()` / `getCls()`
  - `pushData()` - Input student grades
  - `outputData()` - Display grade vector

### Teacher : public Person
Extends Person class with professional information
- Additional Properties: `title`, `nrOfClasses`, `students` (vector)
- Methods:
  - `setTitle()` / `getTitle()`
  - `setNrOfClasses()` / `getNrOfClasses()`
  - `insertStudents()` - Add students with German grade validation (1-6 scale, approval threshold: grades 1-4)
  - `outputStudents()` - Display approved students list

## Usage Example

```
Welcome to our system.
======================================
Are you a student (S) or a teacher(T)?
T
Full Name:
John Smith
Title (Professor/Dr./Instructor):
Dr.
Age:
35
Number of Classes:
2

--- APPROVED STUDENTS REGISTRY ---
(Only students with German grade 1-4 will be approved)
How many students are there? 2
Student 1 name: Anna Mueller
German grade for this student (1-6): 2
  -> Anna Mueller approved (grade 2).
Student 2 name: Bob Weber
German grade for this student (1-6): 5
  -> Bob Weber not approved (grade 5 - failing).
```

## Future Improvements

- [ ] Persistent data storage (file/database)
- [ ] Input validation and error handling
- [ ] Student search and filtering
- [ ] Grade statistics and averages
- [ ] Teacher-student relationship management
- [ ] Graphical User Interface (GUI)

## Author

Orges Gurakuqi

## License

This project is for educational purposes.

## Build Status

Last Build: ✅ Successful (g++ 15.2.0)  
Last Build Date: February 28, 2026
