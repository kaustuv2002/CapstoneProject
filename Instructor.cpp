/*
 * File: Instructor.cpp
 * Project: Academic Information and Management System
 * Purpose: Implements Instructor-specific behavior and virtual display.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "Instructor.h"
#include <iostream>
#include <utility>
Instructor::Instructor(int id, std::string name, std::string dob, std::string contact, std::string department)
    : Person(id, std::move(name), std::move(dob), std::move(contact)), department_(std::move(department)) {}
void Instructor::setDepartment(std::string department) { department_ = std::move(department); }
// RUNTIME POLYMORPHISM: implementation of Person::displayInfo().
void Instructor::displayInfo() const {
    std::cout << "Instructor #" << id_ << " | " << name_ << " | Department: " << department_
              << " | DOB: " << dob_ << " | Contact: " << contact_ << '\n';
}
