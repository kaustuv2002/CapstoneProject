# Academic Information and Management System

A console-based C++17 capstone project for CSC1061. The application manages academic records, course enrollment, grades, and reports. The included sample data is fictional; this is an educational project, not a production student information system.

## Features

- **Record management:** Add, update, delete, and view student, instructor, and course records. Protect records that have dependent enrollment or course assignments.
- **Enrollment and grading:** Enroll students in courses, reject duplicate enrollments, drop pending enrollments, and enter or update grades. Enter `P` in the grading workflow to reset a completed grade to pending.
- **Searching and sorting:** Find students or instructors by ID and courses by code. Use custom Merge Sort to list students by name or GPA, instructors by name or ID, and courses by title, code, or credits.
- **Academic reports:** Generate student transcripts, cumulative and term credit-weighted GPAs, completed-course counts, and instructor teaching-load reports.
- **Data persistence:** Load and validate records from `academic_data.txt`; save changes using a temporary file and backup.

## Main menu

| Option | Function |
| --- | --- |
| `1` | Manage records: add, update, delete, view |
| `2` | Enrollment and grades: enroll, drop pending enrollment, enter/update/reset grade |
| `3` | Find and browse records, including sorted listings |
| `4` | Academic reports: transcript, GPA, and instructor teaching load |
| `0` | Save and exit |

## C++ concepts demonstrated

- **Encapsulation and inheritance:** `Person` is an abstract base class; `Student` and `Instructor` derive from it.
- **Runtime polymorphism:** `displayInfo()` is virtual and overridden by the derived classes.
- **Operator overloading:** `Student` implements `==`, `<`, and stream insertion `<<`.
- **Custom data structures:** A templated `HashTable<T>` stores students by ID, using custom singly linked collision chains and rehashing. The application also uses `std::list<Enrollment>` and maps for other records.
- **Algorithms:** Custom recursive Merge Sort for sorted listings and a separate recursive completed-course counter with an iterative fallback for large lists.
- **File I/O:** Text-based loading, validation, saving, temporary files, and backups; no database required.

## Build and run

Requirements: a C++17-compatible compiler such as `g++`. The code avoids `<filesystem>` for compatibility with older MinGW installations.

From the repository directory, compile:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp Person.cpp Student.cpp Instructor.cpp Course.cpp Enrollment.cpp HashTable.cpp AcademicSystem.cpp -o AcademicSystem.exe
```

On **Windows PowerShell**, run:

```powershell
.\AcademicSystem.exe
```

On **Linux/macOS**, you can instead compile with `-o academic_system` and run `./academic_system`.

Keep `academic_data.txt` in the working directory if you want to load the supplied fictional records. The program writes changes to that file and may create `academic_data.txt.tmp` and `academic_data.txt.bak`. Do not commit executables, generated backups, or real student information.

## Course retakes and grade corrections

The existing data model identifies an enrollment by **student ID and course code**. To represent the same subject offered in multiple terms, create distinct course-offering codes, for example `CSC101-F26` and `CSC101-S27`, with their respective terms. A student can enroll in both offerings. The program does not automatically group them under a shared catalog-course identifier.

Supported completed grades are `A`, `B`, `C`, `D`, and `F`. Enter `P` when updating a grade to return the enrollment to pending; pending enrollments do not contribute to GPA or the completed-course count.

## Capstone documents and diagrams

- [Module 1 Capstone Proposal](CapstoneProposal.pdf)
- [UML Class Diagram](UMLClassDiagram.drawio.png)
- [UML Activity Diagram](UMLActivityDiagram.drawio.png)

The original Module 1 proposal is retained to show the initial design. The final implementation differs in a few details: operator overloads are implemented in `Student` (not in `Instructor` or `Course`), and GPA is calculated from enrollment and course data rather than stored in `Student`. The UML files are kept unchanged.

## Current limitations and possible improvements

- Course offerings use unique codes; a separate course catalog and automatic retake handling could be added in the future.
- Plus/minus grades, withdrawals, and incompletes are not supported.
- Date validation checks real calendar dates in `YYYY-MM-DD` format, and contact validation checks basic email or phone formatting; these checks do not verify identity or ownership.
- Data is stored in local plain-text files, not a secure database. Backup recovery helps with some failures but cannot guarantee protection against every interruption or disk failure.
- The recursive completed-course counter switches to iteration after a bounded recursion depth; more extensive performance and reliability testing would be useful for very large datasets.

## Questions for classmates / feedback requested

1. Is the menu structure clear and easy to navigate? Which operation would you reorganize?
2. Should I prioritize a separate course catalog for retakes or support for additional grade types?
3. What additional invalid-input cases or data-file recovery scenarios should I test?
4. Would moving enrollment and grading logic into a separate manager improve the design?

Feedback on usability, error handling, data structures, and possible future features is welcome.
