/*
 * File: Person.cpp
 * Project: Academic Information and Management System
 * Purpose: Implements the shared Person constructor and update behavior.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "Person.h"
#include <utility>
Person::Person(int id, std::string name, std::string dob, std::string contact)
    : id_(id), name_(std::move(name)), dob_(std::move(dob)), contact_(std::move(contact)) {}
void Person::update(std::string name, std::string dob, std::string contact) {
    name_ = std::move(name); dob_ = std::move(dob); contact_ = std::move(contact);
}
