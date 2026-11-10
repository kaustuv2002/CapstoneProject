/*
 * File: Enrollment.h
 * Project: Academic Information and Management System
 * Purpose: Declares the Enrollment record used to track registration and grades.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#pragma once
#include <string>
struct Enrollment {
    int studentId;
    std::string courseCode, term, grade, status;
};
