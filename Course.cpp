/*
 * File: Course.cpp
 * Project: Academic Information and Management System
 * Purpose: Implements course updates and formatted display.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "Course.h"
#include <iostream>
#include <utility>
Course::Course(std::string code, std::string title, int credits, std::string term, int instructorId)
    : code_(std::move(code)), title_(std::move(title)), term_(std::move(term)),
      credits_(credits), instructorId_(instructorId) {}
void Course::update(std::string title, int credits, std::string term, int instructorId) {
    title_ = std::move(title); credits_ = credits; term_ = std::move(term); instructorId_ = instructorId;
}
void Course::displayInfo() const {
    std::cout << code_ << " | " << title_ << " | Credits: " << credits_
              << " | Term: " << term_ << " | Instructor ID: " << instructorId_ << '\n';
}
