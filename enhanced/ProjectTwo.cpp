// Garrett Sikes
//
// --- Enhancement (Milestone Three: Algorithms and Data Structures) ---
// Original: a plain (unbalanced) Binary Search Tree, which degrades to
// O(n) lookup/insert if course data arrives already sorted by course
// number. Enhancement: converted CourseBST into a self-balancing AVL
// tree (CourseAVL) that stays O(log n) regardless of input order. Also
// fixed a CSV parsing gap (blank names/whitespace weren't caught) and a
// cin.ignore() robustness issue in the menu loop. See the comments next
// to each function below for how and why; see enhancement_diff.txt for
// the full before/after and BENCHMARK_RESULTS.md for measured timing.
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
using namespace std;

// Course struct
struct Course {
    string courseNumber;
    string courseName;
    vector<string> prerequisites;
};

// AVL Node - adds a height field (used to compute balance factors)
// on top of the original BST node.
struct Node {
    Course course;
    Node* left;
    Node* right;
    int height;

    Node(Course c) : course(c), left(nullptr), right(nullptr), height(1) {}
};

// Self-balancing AVL tree (replaces the original CourseBST).
class CourseAVL {
private:
    Node* root;

    // Height of a subtree, treating a null pointer as height 0
    // so the formulas below don't need a separate null check every time.
    int height(Node* node) {
        return node == nullptr ? 0 : node->height;
    }

    // Balance factor = left subtree height minus right subtree height.
    // Anything outside the range [-1, 1] means this node needs a rotation.
    int getBalance(Node* node) {
        return node == nullptr ? 0 : height(node->left) - height(node->right);
    }

    // Standard AVL right rotation, used to fix a "left-heavy" imbalance.
    // Rough shape: y with left child x becomes x with right child y;
    // x's old right subtree (T2) becomes y's new left subtree.
    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;

        return x; // x is the new subtree root
    }

    // Mirror image of rotateRight, used to fix a "right-heavy" imbalance.
    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;

        return y; // y is the new subtree root
    }

    // Recursive AVL insert. Unlike the original BST insert (which took a
    // Node* and modified it in place), this returns the new root of the
    // subtree it was called on, because a rotation can change which node
    // is at the top of that subtree.
    Node* insert(Node* node, Course course) {
        if (node == nullptr) {
            return new Node(course);
        }

    //Duplicate course number. Course number is the tree's key,
    // so treat it as an update-free no-op rather than silently
    // inserting a second, unreachable copy the way the original
    // BST's insert(node->right, course) fallthrough effectively
    // did for anything not strictly less than the current node.

        if (course.courseNumber < node->course.courseNumber) {
            node->left = insert(node->left, course);
        }
        else if (course.courseNumber > node->course.courseNumber) {
            node->right = insert(node->right, course);
        }
        else {

            return node;
        }

        node->height = 1 + max(height(node->left), height(node->right));
        int balance = getBalance(node);

        // Left Left case
        if (balance > 1 && course.courseNumber < node->left->course.courseNumber) {
            return rotateRight(node);
        }
        // Right Right case
        if (balance < -1 && course.courseNumber > node->right->course.courseNumber) {
            return rotateLeft(node);
        }
        // Left Right case
        if (balance > 1 && course.courseNumber > node->left->course.courseNumber) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        // Right Left case
        if (balance < -1 && course.courseNumber < node->right->course.courseNumber) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node; // already balanced
    }

    // Left-root-right traversal. Because every node's key is greater than
    // everything in its left subtree and less than everything in its
    // right subtree, this always visits courses in ascending order by
    // course number, whether the tree is an AVL tree or a plain BST.
    void inOrder(Node* node) {
        if (node != nullptr) {
            inOrder(node->left);
            cout << node->course.courseNumber << ": " << node->course.courseName << endl;
            inOrder(node->right);
        }
    }

    // Standard recursive BST lookup by course number; unchanged by the
    // AVL conversion, since a balanced tree is searched the same way an
    // unbalanced one is, it's just guaranteed to be shallower.
    Course* search(Node* node, string courseNumber) {
        if (node == nullptr) return nullptr;
        if (node->course.courseNumber == courseNumber) return &node->course;
        if (courseNumber < node->course.courseNumber)
            return search(node->left, courseNumber);
        else
            return search(node->right, courseNumber);
    }

    // Post-order delete of every node, so children are freed before their
    // parent. Called from the destructor to avoid leaking memory.
    void destroy(Node* node) {
        if (node != nullptr) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }

public:
    CourseAVL() : root(nullptr) {}
    ~CourseAVL() { destroy(root); }

    // Public wrappers so main() never has to touch Node* or the root
    // pointer directly.
    void insert(Course course) { root = insert(root, course); }
    void printAll() { inOrder(root); }
    Course* find(string courseNumber) { return search(root, courseNumber); }
};

// Trim leading/trailing whitespace (spaces, tabs, stray CR from
// Windows-edited CSV files) from a CSV field.
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Splits one CSV line into a Course. Fixed to trim() every field and to
// leave courseName blank (rather than accepting a stray-space "name")
// when the CSV field is empty or whitespace-only, so the caller can
// reject it - the original neither trimmed fields nor checked the name.
Course parseCourse(string line) {
    stringstream ss(line);
    string item;
    Course course;

    getline(ss, item, ',');
    course.courseNumber = trim(item);

    getline(ss, item, ',');
    course.courseName = trim(item);

    while (getline(ss, item, ',')) {
        string prereq = trim(item);
        if (!prereq.empty()) {
            course.prerequisites.push_back(prereq);
        }
    }

    return course;
}

int main() {
    CourseAVL courseTree;
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

        // Discard the rest of the line after a valid numeric choice.
        // Originally a bare cin.ignore(), which only drops one character
        // and isn't robust to stray characters (e.g. a trailing '\r');
        // now matches the more defensive form used above.
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        // Load Data: read a CSV file line by line, parse each line into
        // a Course, and insert valid courses into the tree. Malformed
        // lines are reported and skipped rather than silently dropped.
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
            int skippedCount = 0;
            while (getline(file, line)) {
                if (line.empty()) continue;
                Course course = parseCourse(line);
                if (course.courseNumber.empty() || course.courseName.empty()) {
                    cout << "  Skipping malformed line (missing course number or name): "
                         << line << endl;
                    skippedCount++;
                    continue;
                }
                courseTree.insert(course);
                lineCount++;
            }

            file.close();
            cout << "Loaded " << lineCount << " course(s) from " << filename << ".\n";
            if (skippedCount > 0) {
                cout << "Skipped " << skippedCount << " malformed line(s).\n";
            }
            break;
        }
        // Print Course List: full in-order traversal, so courses print
        // sorted by course number regardless of insertion order.
        case 2:
            cout << "\nCourse List:\n";
            courseTree.printAll();
            break;
        // Print Course Information: look up a single course by number
        // and display its name and prerequisites, if any.
        case 3: {
            string courseNumber;
            cout << "Enter course number: ";
            cin >> courseNumber;
            Course* course = courseTree.find(courseNumber);
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
