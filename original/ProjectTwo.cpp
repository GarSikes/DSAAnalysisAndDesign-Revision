// Garrett Sikes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

// Course struct
struct Course {
    string courseNumber;
    string courseName;
    vector<string> prerequisites;
};

// BST Node
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(Course c) : course(c), left(nullptr), right(nullptr) {}
};

// Binary Search Tree class
class CourseBST {
private:
    Node* root;

    void insert(Node*& node, Course course) {
        if (node == nullptr) {
            node = new Node(course);
        }
        else if (course.courseNumber < node->course.courseNumber) {
            insert(node->left, course);
        }
        else {
            insert(node->right, course);
        }
    }

    void inOrder(Node* node) {
        if (node != nullptr) {
            inOrder(node->left);
            cout << node->course.courseNumber << ": " << node->course.courseName << endl;
            inOrder(node->right);
        }
    }

    Course* search(Node* node, string courseNumber) {
        if (node == nullptr) return nullptr;
        if (node->course.courseNumber == courseNumber) return &node->course;
        if (courseNumber < node->course.courseNumber)
            return search(node->left, courseNumber);
        else
            return search(node->right, courseNumber);
    }

    void destroy(Node* node) {
        if (node != nullptr) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }

public:
    CourseBST() : root(nullptr) {}
    ~CourseBST() { destroy(root); }

    void insert(Course course) { insert(root, course); }
    void printAll() { inOrder(root); }
    Course* find(string courseNumber) { return search(root, courseNumber); }
};

// Function to split a CSV line into Course
Course parseCourse(string line) {
    stringstream ss(line);
    string item;
    Course course;

    getline(ss, course.courseNumber, ',');
    getline(ss, course.courseName, ',');

    while (getline(ss, item, ',')) {
        course.prerequisites.push_back(item);
    }

    return course;
}

int main() {
    CourseBST bst;
    int choice = 0;

    while (true) {
        cout << "\nMenu:\n";
        cout << "  1. Load Data\n";
        cout << "  2. Print Course List\n";
        cout << "  3. Print Course Information\n";
        cout << "  9. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear(); // clear error state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        cin.ignore();

        switch (choice) {
        case 1: {
            string filename;
            cout << "Enter the CSV file name to load (e.g., courses.csv): ";
            getline(cin, filename);

            ifstream file(filename);

            if (!file.is_open()) {
                cout << "Could not open file: " << filename << endl;
                break;
            }

            string line;
            int lineCount = 0;
            while (getline(file, line)) {
                if (line.empty()) continue;
                Course course = parseCourse(line);
                if (course.courseNumber.empty()) continue;
                bst.insert(course);
                lineCount++;
            }

            file.close();
            cout << "Loaded " << lineCount << " course(s) from " << filename << ".\n";
            break;
        }
        case 2:
            cout << "\nCourse List:\n";
            bst.printAll();
            break;
        case 3: {
            string courseNumber;
            cout << "Enter course number: ";
            cin >> courseNumber;
            Course* course = bst.find(courseNumber);
            if (course) {
                cout << course->courseNumber << ": " << course->courseName << endl;
                if (!course->prerequisites.empty()) {
                    cout << "Prerequisites: ";
                    for (const auto& pre : course->prerequisites) {
                        cout << pre << " ";
                    }
                    cout << endl;
                }
                else {
                    cout << "No prerequisites.\n";
                }
            }
            else {
                cout << "Course not found.\n";
            }
            break;
        }
        case 9:
            cout << "Exiting program.\n";
            return 0;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    }
}