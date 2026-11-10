/*
 * File: HashTable.cpp
 * Project: Academic Information and Management System
 * Purpose: Provides explicit compilation-unit instantiation of the hash table template.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "HashTable.h"
#include "Student.h"
// Template implementation is in the header so all instantiations can use it.
template class HashTable<Student>;
