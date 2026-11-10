/*
 * File: Student.cpp
 * Project: Academic Information and Management System
 * Purpose: Implements Student behavior, virtual display, and overloaded operators.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "Student.h"
#include <iostream>
#include <utility>
Student::Student(int id, std::string name, std::string dob, std::string contact, std::string major)
    : Person(id, std::move(name), std::move(dob), std::move(contact)), major_(std::move(major)) {}
void Student::setMajor(std::string major) { major_ = std::move(major); }
// RUNTIME POLYMORPHISM: implementation of Person::displayInfo().
void Student::displayInfo() const {
    std::cout << "Student #" << id_ << " | " << name_ << " | Major: " << major_
              << " | DOB: " << dob_ << " | Contact: " << contact_ << '\n';
}

// OPERATOR OVERLOADING: equality, ordering, and stream output are implemented below.
bool Student::operator==(const Student& other) const { return id_ == other.id_; }
bool Student::operator<(const Student& other) const {
    return name_ == other.name_ ? id_ < other.id_ : name_ < other.name_;
}
std::ostream& operator<<(std::ostream& out, const Student& student) {
    return out << "Student #" << student.id_ << " | " << student.name_
               << " | Major: " << student.major_;
}
