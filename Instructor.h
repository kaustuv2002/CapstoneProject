/*
 * File: Instructor.h
 * Project: Academic Information and Management System
 * Purpose: Declares the Instructor class derived from Person.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include "Person.h"
#include <string>
// INHERITANCE: Instructor also specializes Person and overrides its virtual display.
class Instructor final : public Person {
    std::string department_;
public:
    Instructor(int id, std::string name, std::string dob, std::string contact, std::string department);
    const std::string& department() const { return department_; }
    void setDepartment(std::string department);
    void displayInfo() const override;
};
