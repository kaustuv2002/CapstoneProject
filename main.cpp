/*
 * File: main.cpp
 * Project: Academic Information and Management System
 * Purpose: Program entry point; starts the console-based academic management application.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "AcademicSystem.h"
#include <exception>
#include <iostream>
int main() {
    try { AcademicSystem system; system.run(); }
    catch (const std::exception& e) { std::cerr << "Program stopped: " << e.what() << '\n'; return 1; }
}
