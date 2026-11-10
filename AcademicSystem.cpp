/*
 * File: AcademicSystem.cpp
 * Project: Academic Information and Management System
 * Purpose: Implements record management, reports, sorting, persistence, and menus.
 * Language: C++17
 * Note: Course concepts are identified in inline comments near their implementations.
 */

#include "AcademicSystem.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iterator>
#include <unordered_map>
#include <set>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>
namespace {
// Portable file helpers: compatible with MinGW GCC 6.3 (no <filesystem>).
bool fileExists(const char* path) {
    std::ifstream file(path, std::ios::binary);
    return static_cast<bool>(file);
}
bool copyFile(const char* source, const char* destination) {
    std::ifstream in(source, std::ios::binary);
    if (!in) return false;
    std::ofstream out(destination, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << in.rdbuf();
    out.flush();
    return static_cast<bool>(out) && (in.eof() || static_cast<bool>(in));
}
std::string trim(std::string s) {
    auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}
// Basic validation without new classes or platform-specific libraries.
bool validDate(const std::string& s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (std::size_t i = 0; i < s.size(); ++i)
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    int y = std::stoi(s.substr(0,4)), m = std::stoi(s.substr(5,2)), d = std::stoi(s.substr(8,2));
    if (y < 1900 || y > 2100 || m < 1 || m > 12) return false;
    const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = days[m-1];
    if (m == 2 && (y%400 == 0 || (y%4 == 0 && y%100 != 0))) ++maxDay;
    return d >= 1 && d <= maxDay;
}
bool validContact(const std::string& s) {
    if (s.empty() || s.size() > 254) return false;
    if (s.find('@') != std::string::npos) {
        auto at = s.find('@');
        auto dot = s.find('.', at + 1);
        if (at == 0 || dot == std::string::npos || dot <= at+1 || dot+1 >= s.size() ||
            s.find('@', at+1) != std::string::npos) return false;
        for (char c : s) if (std::isspace(static_cast<unsigned char>(c))) return false;
        return true;
    }
    int digits = 0;
    for (char c : s) {
        if (std::isdigit(static_cast<unsigned char>(c))) ++digits;
        else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')' && c != '.') return false;
    }
    return digits >= 7 && digits <= 15;
}
std::string readValidated(const std::string& prompt, bool (*validator)(const std::string&),
                          const std::string& error) {
    while (true) {
        std::cout << prompt;
        std::string value;
        if (!std::getline(std::cin, value)) throw std::runtime_error("Input closed");
        value = trim(value);
        if (validator(value)) return value;
        std::cout << error << '\n';
    }
}
std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}
}
AcademicSystem::AcademicSystem() { load(); }
int AcademicSystem::readInt(const std::string& prompt, int min) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) throw std::runtime_error("Input closed");
        std::istringstream in(line);
        int n; char extra;
        if ((in >> n) && !(in >> extra) && n >= min) return n;
        std::cout << "Enter a whole number >= " << min << ".\n";
    }
}
std::string AcademicSystem::readText(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) throw std::runtime_error("Input closed");
        line = trim(line);
        if (!line.empty()) return line;
        std::cout << "This field cannot be empty.\n";
    }
}
double AcademicSystem::gradePoints(const std::string& grade) {
    if (grade == "A") return 4.0;
    if (grade == "B") return 3.0;
    if (grade == "C") return 2.0;
    if (grade == "D") return 1.0;
    if (grade == "F") return 0.0;
    return -1.0;
}
bool AcademicSystem::hasEnrollment(int studentId, const std::string& code) const {
    for (const auto& e : enrollments_) if (e.studentId == studentId && e.courseCode == code) return true;
    return false;
}
bool AcademicSystem::isValidInstructor(int id) const {
    return id == 0 || instructors_.find(id) != instructors_.end();
}
// Weighted GPA: only completed, graded enrollments contribute course credits.
double AcademicSystem::gpa(int studentId, const std::string& term) const {
    double total = 0; int credits = 0;
    for (const auto& e : enrollments_) {
        if (e.studentId != studentId || (term != "" && e.term != term) || e.status != "Completed") continue;
        double pts = gradePoints(e.grade);
        auto it = courses_.find(e.courseCode);
        if (pts < 0 || it == courses_.end()) continue;
        int c = it->second.credits();
        total += pts * c; credits += c;
    }
    return credits ? total / credits : 0;
}
// RECURSION OUTSIDE SORTING: count completed courses by recursively visiting list nodes.
int AcademicSystem::countCompleted(std::list<Enrollment>::const_iterator it,
                                   std::list<Enrollment>::const_iterator end, int studentId) {
    if (it == end) return 0;
    // Bound recursion depth; count the remainder iteratively to avoid stack overflow.
    // The usual small inputs still exercise the recursive course requirement.
    static const int maxDepth = 256;
    // Thread-local depth is reset on each top-level return, including exceptions.
    struct DepthGuard { int& depth; explicit DepthGuard(int& d) : depth(d) { ++depth; }
                        ~DepthGuard() { --depth; } };
    static thread_local int depth = 0;
    if (depth >= maxDepth) {
        int total = 0;
        for (; it != end; ++it)
            if (it->studentId == studentId && it->status == "Completed" && gradePoints(it->grade) >= 0) ++total;
        return total;
    }
    DepthGuard guard(depth);
    const int match = (it->studentId == studentId && it->status == "Completed" && gradePoints(it->grade) >= 0) ? 1 : 0;
    return match + countCompleted(std::next(it), end, studentId);
}
void AcademicSystem::addRecord() {
    int type = readInt("Add: 1 Student, 2 Instructor, 3 Course: ", 1);
    if (type == 1 || type == 2) {
        int id = readInt("ID: ", 1);
        if (students_.find(id) || instructors_.count(id)) { std::cout << "ID already exists.\n"; return; }
        auto name = readText("Full name: ");
        auto dob = readValidated("DOB (YYYY-MM-DD): ", validDate, "Enter a valid date in YYYY-MM-DD format.");
        auto contact = readValidated("Contact (email or phone): ", validContact, "Enter a valid email address or phone number (7-15 digits).");
        if (type == 1) {
            auto major = readText("Major: ");
            students_.insert(id, std::make_shared<Student>(id, name, dob, contact, major));
        } else {
            auto dept = readText("Department: ");
            instructors_.emplace(id, std::make_shared<Instructor>(id, name, dob, contact, dept));
        }
    } else if (type == 3) {
        auto code = upper(readText("Course code: "));
        if (courses_.count(code)) { std::cout << "Course exists.\n"; return; }
        auto title = readText("Course title: ");
        int credits = readInt("Credit hours: ", 1);
        if (credits > 30) { std::cout << "Credits must be at most 30.\n"; return; }
        auto term = readText("Term (e.g. Fall2026): ");
        int instructor = readInt("Instructor ID (0 = unassigned): ");
        if (!isValidInstructor(instructor)) { std::cout << "Instructor not found.\n"; return; }
        courses_.emplace(code, Course(code, title, credits, term, instructor));
    } else { std::cout << "Invalid type.\n"; return; }
    if (save()) std::cout << "Record added.\n"; else std::cerr << "WARNING: Change is in memory only; save failed.\n";
}
void AcademicSystem::updateRecord() {
    int type = readInt("Update: 1 Student, 2 Instructor, 3 Course: ", 1);
    if (type == 1 || type == 2) {
        int id = readInt("ID: ", 1);
        if (type == 1) {
            auto s = students_.find(id);
            if (!s) { std::cout << "Student not found.\n"; return; }
            auto name = readText("Name: ");
            auto dob = readValidated("DOB (YYYY-MM-DD): ", validDate, "Enter a valid date in YYYY-MM-DD format.");
            auto contact = readValidated("Contact (email or phone): ", validContact, "Enter a valid email address or phone number (7-15 digits)."); auto major = readText("Major: ");
            s->update(name, dob, contact); s->setMajor(major);
        } else {
            auto it = instructors_.find(id);
            if (it == instructors_.end()) { std::cout << "Instructor not found.\n"; return; }
            auto name = readText("Name: ");
            auto dob = readValidated("DOB (YYYY-MM-DD): ", validDate, "Enter a valid date in YYYY-MM-DD format.");
            auto contact = readValidated("Contact (email or phone): ", validContact, "Enter a valid email address or phone number (7-15 digits)."); auto department = readText("Department: ");
            it->second->update(name, dob, contact); it->second->setDepartment(department);
        }
    } else if (type == 3) {
        auto code = upper(readText("Course code: "));
        auto it = courses_.find(code);
        if (it == courses_.end()) { std::cout << "Course not found.\n"; return; }
        auto title = readText("Title: "); int credits = readInt("Credits: ", 1);
        if (credits > 30) { std::cout << "Credits must be at most 30.\n"; return; }
        auto term = readText("Term: "); int id = readInt("Instructor ID (0 = unassigned): ");
        if (!isValidInstructor(id)) { std::cout << "Instructor not found.\n"; return; }
        // Preserve GPA integrity: don't change credits or term after enrollment.
        if (std::any_of(enrollments_.begin(), enrollments_.end(), [&](const Enrollment& e){return e.courseCode == code;})
            && (credits != it->second.credits() || term != it->second.term())) {
            std::cout << "Cannot change credits or term for a course with enrollments.\n"; return;
        }
        it->second.update(title, credits, term, id);
    } else { std::cout << "Invalid type.\n"; return; }
    if (save()) std::cout << "Record updated.\n"; else std::cerr << "WARNING: Change is in memory only; save failed.\n";
}
void AcademicSystem::deleteRecord() {
    if (upper(readText("Type YES to confirm deletion: ")) != "YES") { std::cout << "Cancelled.\n"; return; }
    int type = readInt("Delete: 1 Student, 2 Instructor, 3 Course: ", 1);
    if (type == 1) {
        int id = readInt("Student ID: ", 1);
        if (!students_.find(id)) { std::cout << "Student not found.\n"; return; }
        if (std::any_of(enrollments_.begin(), enrollments_.end(), [&](const Enrollment& e){return e.studentId == id;})) {
            std::cout << "Cannot delete student with enrollment history.\n"; return;
        }
        students_.erase(id);
    } else if (type == 2) {
        int id = readInt("Instructor ID: ", 1);
        if (!instructors_.count(id)) { std::cout << "Instructor not found.\n"; return; }
        for (const auto& kv : courses_) if (kv.second.instructorId() == id) {
            std::cout << "Reassign this instructor's courses first.\n"; return;
        }
        instructors_.erase(id);
    } else if (type == 3) {
        auto code = upper(readText("Course code: "));
        if (!courses_.count(code)) { std::cout << "Course not found.\n"; return; }
        if (std::any_of(enrollments_.begin(), enrollments_.end(), [&](const Enrollment& e){return e.courseCode == code;})) {
            std::cout << "Cannot delete a course with enrollment history.\n"; return;
        }
        courses_.erase(code);
    } else { std::cout << "Invalid type.\n"; return; }
    if (save()) std::cout << "Record deleted.\n"; else std::cerr << "WARNING: Change is in memory only; save failed.\n";
}
// POLYMORPHISM: the View All branch invokes displayInfo() through Person pointers.
void AcademicSystem::viewRecord() {
    int type = readInt("View: 1 Student, 2 Instructor, 3 Course, 4 All: ", 1);
    if (type == 1) {
        auto s = students_.find(readInt("Student ID: ", 1));
        if (s) std::cout << *s << '\n'; else std::cout << "Not found.\n";
    } else if (type == 2) {
        int id = readInt("Instructor ID: ", 1);
        auto it = instructors_.find(id);
        if (it != instructors_.end()) it->second->displayInfo(); else std::cout << "Not found.\n";
    } else if (type == 3) {
        auto it = courses_.find(upper(readText("Course code: ")));
        if (it != courses_.end()) it->second.displayInfo(); else std::cout << "Not found.\n";
    } else if (type == 4) {
        // Runtime polymorphism through Person base pointers.
        std::vector<const Person*> people;
        students_.forEach([&](const Student& s){ people.push_back(&s); });
        for (const auto& kv : instructors_) people.push_back(kv.second.get());
        for (const Person* person : people) person->displayInfo();
        for (const auto& kv : courses_) kv.second.displayInfo();
        if (people.empty() && courses_.empty()) std::cout << "No records.\n";
    } else std::cout << "Invalid type.\n";
}
void AcademicSystem::recordMenu() {
    while (true) {
        std::cout << "\n--- Record Management ---\n1 Add\n2 Update\n3 Delete\n4 View\n0 Back\n";
        int n = readInt("Choice: ");
        if (n == 0) return;
        if (n == 1) addRecord(); else if (n == 2) updateRecord();
        else if (n == 3) deleteRecord(); else if (n == 4) viewRecord();
        else std::cout << "Invalid choice.\n";
    }
}
void AcademicSystem::enrollStudent() {
    int id = readInt("Student ID: ", 1);
    if (!students_.find(id)) { std::cout << "Student not found.\n"; return; }
    auto code = upper(readText("Course code: "));
    auto it = courses_.find(code);
    if (it == courses_.end()) { std::cout << "Course not found.\n"; return; }
    if (hasEnrollment(id, code)) { std::cout << "Already enrolled in this course.\n"; return; }
    enrollments_.push_back({id, code, it->second.term(), "", "Enrolled"});
    if (save()) std::cout << "Enrollment created.\n"; else std::cerr << "WARNING: Enrollment is in memory only; save failed.\n";
}
void AcademicSystem::dropEnrollment() {
    int id = readInt("Student ID: ", 1);
    auto code = upper(readText("Course code: "));
    for (auto it = enrollments_.begin(); it != enrollments_.end(); ++it) {
        if (it->studentId == id && it->courseCode == code) {
            if (it->status != "Enrolled") { std::cout << "Completed courses cannot be dropped.\n"; return; }
            if (upper(readText("Type YES to confirm drop: ")) != "YES") { std::cout << "Cancelled.\n"; return; }
            enrollments_.erase(it);
            if (save()) std::cout << "Enrollment dropped.\n";
            else std::cerr << "WARNING: Drop is in memory only; save failed.\n";
            return;
        }
    }
    std::cout << "Enrollment not found.\n";
}
void AcademicSystem::enterGrade() {
    int id = readInt("Student ID: ", 1);
    auto code = upper(readText("Course code: "));
    for (auto& e : enrollments_) if (e.studentId == id && e.courseCode == code) {
        auto grade = upper(readText("Grade (A/B/C/D/F, or P to reset to pending): "));
        if (grade != "P" && gradePoints(grade) < 0) { std::cout << "Invalid grade.\n"; return; }
        if (grade == "P") { e.grade.clear(); e.status = "Enrolled"; }
        else { e.grade = grade; e.status = "Completed"; }
        if (save()) std::cout << (grade == "P" ? "Grade reset to pending. " : "Grade saved. ")
                              << "Cumulative GPA: " << std::fixed << std::setprecision(2) << gpa(id) << '\n';
        else std::cerr << "WARNING: Grade change is in memory only; save failed.\n";
        return;
    }
    std::cout << "Enrollment not found.\n";
}
void AcademicSystem::transcript(int id) const {
    auto s = students_.find(id);
    if (!s) { std::cout << "Student not found.\n"; return; }
    std::cout << "\n--- Student Transcript ---\n";
    s->displayInfo();
    bool any = false;
    for (const auto& e : enrollments_) if (e.studentId == id) {
        any = true;
        auto c = courses_.find(e.courseCode);
        std::cout << e.courseCode << " | " << (c != courses_.end() ? c->second.title() : "Unknown")
                  << " | " << e.term << " | Grade: " << (e.grade.empty() ? "Pending" : e.grade)
                  << " | " << e.status << '\n';
    }
    if (!any) std::cout << "No enrollments.\n";
    std::cout << "Cumulative GPA: " << std::fixed << std::setprecision(2) << gpa(id)
              << " | Completed courses (recursive count): "
              << countCompleted(enrollments_.cbegin(), enrollments_.cend(), id) << '\n';
    std::string term;
    std::cout << "Enter term for semester GPA (or press Enter to skip): ";
    if (!std::getline(std::cin, term)) throw std::runtime_error("Input closed");
    term = trim(term);
    if (!term.empty()) std::cout << "GPA for " << term << ": " << gpa(id, term) << '\n';
}
void AcademicSystem::instructorReport(int id) const {
    auto it = instructors_.find(id);
    if (it == instructors_.end()) { std::cout << "Instructor not found.\n"; return; }
    it->second->displayInfo();
    int count = 0;
    for (const auto& kv : courses_) if (kv.second.instructorId() == id) {
        kv.second.displayInfo(); ++count;
    }
    std::cout << "Teaching load: " << count << " course(s).\n";
}
// Generic stable recursive merge sort for instructor and course reports.
// The original student-specific mergeSort remains in use for GPA/name sorting.
namespace {
template <typename T, typename Compare>
void mergeSortRecords(std::vector<T>& values, std::size_t first, std::size_t last, Compare before) {
    if (last - first < 2) return;
    const auto mid = first + (last - first) / 2;
    mergeSortRecords(values, first, mid, before);
    mergeSortRecords(values, mid, last, before);
    std::vector<T> merged;
    merged.reserve(last - first);
    auto left = first, right = mid;
    while (left < mid && right < last) {
        // Taking the left element on equality makes merge sort stable.
        if (!before(values[right], values[left])) merged.push_back(values[left++]);
        else merged.push_back(values[right++]);
    }
    while (left < mid) merged.push_back(values[left++]);
    while (right < last) merged.push_back(values[right++]);
    for (std::size_t i = 0; i < merged.size(); ++i) values[first + i] = std::move(merged[i]);
}
}

// CUSTOM SORTING ALGORITHM: recursive merge sort; no std::sort is used.
void AcademicSystem::mergeSort(std::vector<std::shared_ptr<Student>>& a, int lo, int hi,
                               bool byGpa, const std::unordered_map<int, double>& cached) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    mergeSort(a, lo, mid, byGpa, cached); mergeSort(a, mid, hi, byGpa, cached);
    std::vector<std::shared_ptr<Student>> tmp;
    int i = lo, j = mid;
    auto before = [&](const std::shared_ptr<Student>& x, const std::shared_ptr<Student>& y) {
        if (byGpa) {
            double gx = cached.at(x->id()), gy = cached.at(y->id());
            if (gx != gy) return gx > gy;
        }
        return *x < *y;
    };
    while (i < mid && j < hi) tmp.push_back(!before(a[j], a[i]) ? a[i++] : a[j++]);
    while (i < mid) tmp.push_back(a[i++]);
    while (j < hi) tmp.push_back(a[j++]);
    for (int k = lo; k < hi; ++k) a[k] = std::move(tmp[k - lo]);
}
void AcademicSystem::sortedStudents() const {
    std::vector<std::shared_ptr<Student>> a;
    students_.forEach([&](const Student& s){ a.push_back(students_.find(s.id())); });
    int option = readInt("Sort by 1 Name (A-Z), 2 GPA (high-low): ", 1);
    if (option != 1 && option != 2) { std::cout << "Invalid choice.\n"; return; }
    std::unordered_map<int, double> cached;
    for (const auto& student : a) cached.emplace(student->id(), gpa(student->id()));
    mergeSort(a, 0, static_cast<int>(a.size()), option == 2, cached);
    for (const auto& s : a) {
        std::cout << s->id() << " | " << s->name() << " | GPA: "
                  << std::fixed << std::setprecision(2) << cached.at(s->id()) << '\n';
    }
    if (a.empty()) std::cout << "No students.\n";
}
void AcademicSystem::sortedInstructors() const {
    std::vector<std::shared_ptr<Instructor>> values;
    for (const auto& entry : instructors_) values.push_back(entry.second);
    int option = readInt("Sort instructors by 1 Name (A-Z), 2 ID (ascending): ", 1);
    if (option != 1 && option != 2) { std::cout << "Invalid choice.\n"; return; }
    mergeSortRecords(values, 0, values.size(), [option](const auto& a, const auto& b) {
        if (option == 2) return a->id() < b->id();
        return a->name() == b->name() ? a->id() < b->id() : a->name() < b->name();
    });
    if (values.empty()) std::cout << "No instructors.\n";
    for (const auto& instructor : values) instructor->displayInfo();
}

void AcademicSystem::sortedCourses() const {
    std::vector<const Course*> values;
    for (const auto& entry : courses_) values.push_back(&entry.second);
    int option = readInt("Sort courses by 1 Title (A-Z), 2 Code (A-Z), 3 Credits (high-low): ", 1);
    if (option < 1 || option > 3) { std::cout << "Invalid choice.\n"; return; }
    mergeSortRecords(values, 0, values.size(), [option](const Course* a, const Course* b) {
        if (option == 1 && a->title() != b->title()) return a->title() < b->title();
        if (option == 3 && a->credits() != b->credits()) return a->credits() > b->credits();
        return a->code() < b->code();
    });
    if (values.empty()) std::cout << "No courses.\n";
    for (const Course* course : values) course->displayInfo();
}

void AcademicSystem::enrollmentMenu() {
    while (true) {
        std::cout << "\n--- Enrollment & Grades ---\n"
                  << "1 Enroll student in a course\n"
                  << "2 Drop a pending enrollment\n"
                  << "3 Enter or update a grade\n"
                  << "0 Back to main menu\n";
        int choice = readInt("Choice: ");
        if (choice == 0) return;
        if (choice == 1) enrollStudent();
        else if (choice == 2) dropEnrollment();
        else if (choice == 3) enterGrade();
        else std::cout << "Invalid choice.\n";
    }
}

void AcademicSystem::searchMenu() {
    while (true) {
        std::cout << "\n--- Find & Browse Records ---\n"
                  << "1 Find student by ID\n"
                  << "2 Find instructor by ID\n"
                  << "3 Find course by code\n"
                  << "4 List students (Merge Sort)\n"
                  << "5 List instructors (Merge Sort)\n"
                  << "6 List courses (Merge Sort)\n"
                  << "0 Back to main menu\n";
        int choice = readInt("Choice: ");
        if (choice == 0) return;
        if (choice == 1) {
            auto student = students_.find(readInt("Student ID: ", 1));
            if (student) student->displayInfo(); else std::cout << "Student not found.\n";
        } else if (choice == 2) {
            auto it = instructors_.find(readInt("Instructor ID: ", 1));
            if (it != instructors_.end()) it->second->displayInfo(); else std::cout << "Instructor not found.\n";
        } else if (choice == 3) {
            auto it = courses_.find(upper(readText("Course code: ")));
            if (it != courses_.end()) it->second.displayInfo(); else std::cout << "Course not found.\n";
        } else if (choice == 4) sortedStudents();
        else if (choice == 5) sortedInstructors();
        else if (choice == 6) sortedCourses();
        else std::cout << "Invalid choice.\n";
    }
}

void AcademicSystem::reportMenu() {
    while (true) {
        std::cout << "\n--- Academic Reports ---\n"
                  << "1 Student transcript and GPA\n"
                  << "2 Instructor teaching-load report\n"
                  << "0 Back to main menu\n";
        int choice = readInt("Choice: ");
        if (choice == 0) return;
        if (choice == 1) transcript(readInt("Student ID: ", 1));
        else if (choice == 2) instructorReport(readInt("Instructor ID: ", 1));
        else std::cout << "Invalid choice.\n";
    }
}
// FILE I/O: persist records in a local text file; no database is required.
bool AcademicSystem::save() const {
    // Simple educational persistence format. Files are local and NOT encrypted.
    std::ofstream out("academic_data.txt.tmp", std::ios::trunc);
    if (!out) { std::cerr << "Cannot save academic_data.txt\n"; return false; }
    students_.forEach([&](const Student& s){
        out << "S " << s.id() << ' ' << std::quoted(s.name()) << ' ' << std::quoted(s.dob())
            << ' ' << std::quoted(s.contact()) << ' ' << std::quoted(s.major()) << '\n';
    });
    for (const auto& kv : instructors_) {
        const auto& t = *kv.second;
        out << "I " << t.id() << ' ' << std::quoted(t.name()) << ' ' << std::quoted(t.dob())
            << ' ' << std::quoted(t.contact()) << ' ' << std::quoted(t.department()) << '\n';
    }
    for (const auto& kv : courses_) {
        const auto& c = kv.second;
        out << "C " << std::quoted(c.code()) << ' ' << std::quoted(c.title()) << ' '
            << c.credits() << ' ' << std::quoted(c.term()) << ' ' << c.instructorId() << '\n';
    }
    for (const auto& e : enrollments_) {
        out << "E " << e.studentId << ' ' << std::quoted(e.courseCode) << ' '
            << std::quoted(e.term) << ' ' << std::quoted(e.grade) << ' ' << std::quoted(e.status) << '\n';
    }
    out.flush();
    if (!out) { std::cerr << "Save failed.\n"; return false; }
    out.close();
    if (!out) { std::cerr << "Save failed while closing file.\n"; return false; }
    const char* original = "academic_data.txt";
    const char* backup = "academic_data.txt.bak";
    const char* temp = "academic_data.txt.tmp";
    if (fileExists(original) && !copyFile(original, backup)) {
        std::cerr << "Cannot create data backup. Original file preserved.\n";
        return false;
    }
    // Windows C rename() cannot overwrite an existing destination.
    // Keep the backup before removing the original.
    if (fileExists(original) && std::remove(original) != 0) {
        std::cerr << "Cannot replace original data file.\n";
        return false;
    }
    if (std::rename(temp, original) != 0) {
        std::cerr << "Cannot install new data file.\n";
        if (!fileExists(original) && fileExists(backup)) {
            if (!copyFile(backup, original))
                std::cerr << "WARNING: Restore failed; recover from .bak manually.\n";
        }
        return false;
    }
    return true;
}
void AcademicSystem::load() {
    const char* original = "academic_data.txt";
    const char* backup = "academic_data.txt.bak";
    const char* temp = "academic_data.txt.tmp";
    if (!fileExists(original)) {
        const char* candidate = fileExists(backup) ? backup : temp;
        if (fileExists(candidate)) {
            if (!copyFile(candidate, original))
                throw std::runtime_error("Could not recover academic_data.txt");
            std::cerr << "Recovered data from " << candidate << '\n';
        } else return;
    }
    std::ifstream in(original);
    if (!in) throw std::runtime_error("Cannot open academic_data.txt");
    struct Row { int line; char kind; std::string body; };
    std::vector<Row> rows;
    std::string line; int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (lineNo == 1 && line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF) line.erase(0, 3);
        std::istringstream ss(line); char kind;
        if (!(ss >> kind)) continue;
        if (kind != 'S' && kind != 'I' && kind != 'C' && kind != 'E')
            throw std::runtime_error("Unknown record type on line " + std::to_string(lineNo));
        rows.push_back({lineNo, kind, line});
    }
    if (!in.eof()) throw std::runtime_error("Error reading academic_data.txt");
    // Three passes permit any ordering of people, courses and enrollments.
    std::set<std::pair<int, std::string>> enrollmentKeys;
    for (char pass : {'P', 'C', 'E'}) for (const Row& row : rows) {
        if ((pass == 'P' && row.kind != 'S' && row.kind != 'I') ||
            (pass != 'P' && row.kind != pass)) continue;
        std::istringstream ss(row.body); char kind; ss >> kind;
        auto validEnd = [&]() { ss >> std::ws; return ss.eof(); };
        auto invalid = [&]() { throw std::runtime_error("Invalid or duplicate data on line " + std::to_string(row.line) + ". Original file preserved."); };
        if (kind == 'S' || kind == 'I') {
            int id; std::string name, dob, contact, extra;
            if (!(ss >> id >> std::quoted(name) >> std::quoted(dob) >> std::quoted(contact) >> std::quoted(extra)) ||
                !validEnd() || id <= 0 || name.empty() || !validDate(dob) || !validContact(contact) || extra.empty() ||
                students_.find(id) || instructors_.count(id)) invalid();
            if (kind == 'S') students_.insert(id, std::make_shared<Student>(id, name, dob, contact, extra));
            else instructors_.emplace(id, std::make_shared<Instructor>(id, name, dob, contact, extra));
        } else if (kind == 'C') {
            std::string code, title, term; int credits, instructor;
            if (!(ss >> std::quoted(code) >> std::quoted(title) >> credits >> std::quoted(term) >> instructor) || !validEnd()) invalid();
            code = upper(code);
            if (credits < 1 || credits > 30 || code.empty() || title.empty() || term.empty() ||
                courses_.count(code) || !isValidInstructor(instructor)) invalid();
            courses_.emplace(code, Course(code, title, credits, term, instructor));
        } else {
            Enrollment e;
            if (!(ss >> e.studentId >> std::quoted(e.courseCode) >> std::quoted(e.term) >>
                      std::quoted(e.grade) >> std::quoted(e.status)) || !validEnd()) invalid();
            e.courseCode = upper(e.courseCode);
            auto course = courses_.find(e.courseCode);
            if (e.studentId <= 0 || !students_.find(e.studentId) || course == courses_.end() ||
                e.term != course->second.term() ||
                !((e.status == "Enrolled" && e.grade.empty()) ||
                  (e.status == "Completed" && gradePoints(e.grade) >= 0)) ||
                !enrollmentKeys.insert({e.studentId, e.courseCode}).second) invalid();
            enrollments_.push_back(std::move(e));
        }
    }
}
void AcademicSystem::run() {
    std::cout << "=== Academic Information and Management System ===\n";
    while (true) {
        std::cout << "\n========== MAIN MENU ==========\n"
                  << "1 Manage records (add / update / delete / view)\n"
                  << "2 Enrollment and grades\n"
                  << "3 Find and browse records\n"
                  << "4 Academic reports\n"
                  << "0 Save and exit\n";
        int choice = readInt("Choice: ");
        if (choice == 0) {
            if (!save()) { std::cerr << "Save failed. Please resolve the issue before exiting.\n"; continue; }
            std::cout << "Goodbye.\n";
            return;
        }
        if (choice == 1) recordMenu();
        else if (choice == 2) enrollmentMenu();
        else if (choice == 3) searchMenu();
        else if (choice == 4) reportMenu();
        else std::cout << "Invalid choice.\n";
    }
}
