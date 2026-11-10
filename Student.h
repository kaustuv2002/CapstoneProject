/*
 * File: Student.h
 * Project: Academic Information and Management System
 * Purpose: Declares the Student class, its academic attributes, and overloaded operators.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include "Person.h"
#include <string>
#include <iosfwd>
// INHERITANCE: Student specializes the abstract Person base class.
class Student final : public Person {
    std::string major_;
public:
    Student(int id, std::string name, std::string dob, std::string contact, std::string major);
    const std::string& major() const { return major_; }
    void setMajor(std::string major);
    void displayInfo() const override;
    // OPERATOR OVERLOADING (1 of 3): equality comparison by student ID.
    bool operator==(const Student& other) const; 
    // OPERATOR OVERLOADING (2 of 3): ordering for student comparisons.
    bool operator<(const Student& other) const;  
    // OPERATOR OVERLOADING (3 of 3): formatted output to a stream.
    friend std::ostream& operator<<(std::ostream& out, const Student& student);
};
