/*
 * File: Person.h
 * Project: Academic Information and Management System
 * Purpose: Declares the abstract Person base class and shared person attributes.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include <string>
// ABSTRACT CLASS + ENCAPSULATION: shared protected state, public accessors, and a pure virtual method.
class Person {
protected:
    int id_;
    std::string name_, dob_, contact_;
public:
    Person(int id, std::string name, std::string dob, std::string contact);
    virtual ~Person() = default;
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& dob() const { return dob_; }
    const std::string& contact() const { return contact_; }
    void update(std::string name, std::string dob, std::string contact);
    // POLYMORPHISM: derived classes must override this virtual interface.
    virtual void displayInfo() const = 0;
};
