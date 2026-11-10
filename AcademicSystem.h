/*
 * File: AcademicSystem.h
 * Project: Academic Information and Management System
 * Purpose: Declares the application controller, collections, and workflows.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include "Student.h"
#include "Instructor.h"
#include "Course.h"
#include "Enrollment.h"
#include "HashTable.h"
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
// ENCAPSULATION: private collections and helpers; public constructor and run interface.
class AcademicSystem {
    // CUSTOM DATA STRUCTURE: students are indexed by ID in our handwritten hash table.
    HashTable<Student> students_;
    std::map<int, std::shared_ptr<Instructor>> instructors_;
    std::map<std::string, Course> courses_;
    std::list<Enrollment> enrollments_;
    static int readInt(const std::string& prompt, int min = 0);
    static std::string readText(const std::string& prompt);
    static double gradePoints(const std::string& grade);
    bool hasEnrollment(int studentId, const std::string& code) const;
    bool isValidInstructor(int instructorId) const;
    double gpa(int studentId, const std::string& term = "") const;
    void recordMenu();
    void addRecord(); void updateRecord(); void deleteRecord(); void viewRecord();
    void enrollStudent(); void dropEnrollment(); void enterGrade();
    void enrollmentMenu(); void searchMenu(); void reportMenu();
    void transcript(int studentId) const;
    void instructorReport(int instructorId) const;
    void sortedStudents() const;
    void sortedInstructors() const;
    void sortedCourses() const;
    static void mergeSort(std::vector<std::shared_ptr<Student>>& data, int lo, int hi, bool byGpa, const std::unordered_map<int, double>& cached);
    // Recursion independent of sorting: walks the linked list to count completed courses.
    static int countCompleted(std::list<Enrollment>::const_iterator it,
                              std::list<Enrollment>::const_iterator end, int studentId);
    bool save() const;
    void load();
public:
    AcademicSystem();
    void run();
};
