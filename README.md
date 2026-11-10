# Academic Information and Management System

C++17 console-based academic records management capstone for CSC1061. Uses fictional sample data only.

## Features
- Add, update, delete and view students, instructors and courses
- Enroll students, drop pending enrollments, enter or update grades
- Search by student/instructor ID or course code
- Merge Sort: students by name or GPA; instructors by name or ID; courses by title, code or credits
- Student transcript, cumulative/term credit-weighted GPA and instructor teaching-load report
- Save to `academic_data.txt` using temporary file and backup; validate records on load

## Main menu
- `1` Manage records: add, update, delete, view
- `2` Enrollment and grades: enroll, drop pending, enter/update grade
- `3` Find and browse: search records and sort student/instructor/course listings
- `4` Academic reports: transcript/GPA and teaching load
- `0` Save and exit

## C++ concepts
- `Person` abstract base class, `Student` and `Instructor` inheritance, virtual dispatch and encapsulation
- `Student` operator overloading (`==`, `<`, `<<`)
- Custom templated `HashTable<T>` using singly linked collision chains and rehashing
- `std::list<Enrollment>` for enrollment records; maps for instructors/courses
- Recursive Merge Sort and separate recursive completed-course counter

## Build and run
From the repository directory (MinGW GCC 6.3-compatible; no `<filesystem>`):

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp Person.cpp Student.cpp Instructor.cpp Course.cpp Enrollment.cpp HashTable.cpp AcademicSystem.cpp -o AcademicSystem.exe
```

To run on Windows PowerShell: `\.\AcademicSystem.exe` 



Keep `academic_data.txt` in the working directory. Modifications save automatically, with `.tmp` and `.bak` created at runtime. Do not commit generated backups, executables or real student data.

## UML diagrams
- `UMLClass.png`: classes, attributes, operations, inheritance and relationships
- `UMLActivity.png`: actual main menu and submenu workflows

## Improvements and compatibility
- **Course retakes across terms:** Give each course offering a distinct code, e.g. `CSC101-F26` and `CSC101-S27`. Each is a separate Course record with its own term, so a student may enroll in both. The system does **not** automatically group these offerings under a shared catalog course; the original UML and data format are unchanged.
- **Date and contact validation:** Add/update student and instructor prompts require a real `YYYY-MM-DD` calendar date and a basic email or phone format. Existing records are checked on load as well.
- **Grade corrections:** In Enrollment & Grades > Enter or update a grade, type `P` to reset a completed grade to pending. The GPA and completed-course count then exclude it.
- **Large datasets:** Completed-course counting uses recursion for up to 256 levels, then iterates over the remaining records to prevent stack overflow.
- **Original proposal:** The Module 1 proposal is preserved as originally submitted. In the implemented version, the Student class overloads `==`, `<`, and `<<`; Instructor and Course use `displayInfo()`. GPA is calculated from Enrollment/Course records rather than stored inside Student. These implementation details differ from the early proposal but require no UML changes.

## Limitations and feedback requested
- Each offering needs a unique course code; there is no separate shared catalog-course identifier.
- Letter grades are A/B/C/D/F; `P` resets to pending. Plus/minus, withdrawal and incomplete grades are not supported.
- Date/contact validation is basic, not identity verification. This is not a secure production student information system.
- Large enrollment lists use an iterative fallback after 256 recursive calls.

**Questions for classmates:** Should I prioritize course retakes or stronger validation? Would moving enrollment logic into its own manager make the design clearer? Which additional error cases or reports would you test?


