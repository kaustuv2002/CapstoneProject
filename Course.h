/*
 * File: Course.h
 * Project: Academic Information and Management System
 * Purpose: Declares the Course class for academic course information.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include <string>
class Course {
    std::string code_, title_, term_;
    int credits_, instructorId_;
public:
    Course(std::string code, std::string title, int credits, std::string term, int instructorId);
    const std::string& code() const { return code_; }
    const std::string& title() const { return title_; }
    const std::string& term() const { return term_; }
    int credits() const { return credits_; }
    int instructorId() const { return instructorId_; }
    void update(std::string title, int credits, std::string term, int instructorId);
    void displayInfo() const;
};
