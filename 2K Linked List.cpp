// ============================================================================
// Ultimate Linked List Control Panel (DS Playground)
// A single-file C++17 console application for Windows (MSVC) and Linux (g++/clang)
// ============================================================================
// Build instructions:
//   Linux:   g++ -std=c++17 -O2 main.cpp -o app
//   Windows: cl /std:c++17 /O2 main.cpp
// ============================================================================

#define USE_COLOR 1
#define ENABLE_SFML 0

#include <iostream>
#include <string>
#include <functional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstdlib>

#if ENABLE_SFML
// SFML stub - requires SFML library
#include <SFML/Graphics.hpp>
#endif

// ============================================================================
// Utility namespace
// ============================================================================
namespace util {

    // Clear screen helper
    inline void clearScreen() {
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif
    }

    // ANSI color codes
    inline std::string neonGreen() {
#if USE_COLOR
        return "\033[32m";
#else
        return "";
#endif
    }

    inline std::string colorReset() {
#if USE_COLOR
        return "\033[0m";
#else
        return "";
#endif
    }

    inline std::string cyan() {
#if USE_COLOR
        return "\033[36m";
#else
        return "";
#endif
    }

    inline std::string yellow() {
#if USE_COLOR
        return "\033[33m";
#else
        return "";
#endif
    }

    // Wait for user to press ENTER
    inline void waitForEnter() {
        std::cout << neonGreen() << "\n[Press ENTER to continue...]" << colorReset();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Safe input helper
    template<typename T>
    bool safeInput(T& value) {
        if (!(std::cin >> value)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    // Random string generator
    inline std::string randomString(int length, std::mt19937& gen) {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::uniform_int_distribution<> dist(0, static_cast<int>(chars.size()) - 1);
        std::string result;
        result.reserve(length);
        for (int i = 0; i < length; ++i) {
            result += chars[dist(gen)];
        }
        return result;
    }

} // namespace util

// ============================================================================
// Data structures namespace
// ============================================================================
namespace ds {

    // ----------------------------------------------------------------------------
    // Doubly Linked List Template
    // ----------------------------------------------------------------------------
    template<typename T>
    class LinkedList {
    private:
        struct Node {
            T data;
            Node* next;
            Node* prev;

            Node(const T& val) : data(val), next(nullptr), prev(nullptr) {}
        };

        Node* head;
        Node* tail;
        int count;
        bool circular;

        // Helper: Update circular links
        void updateCircularLinks() {
            if (circular && head && tail) {
                head->prev = tail;
                tail->next = head;
            }
            else if (head && tail) {
                head->prev = nullptr;
                tail->next = nullptr;
            }
        }

        // Helper: Get node at index (nullptr if out of bounds)
        Node* getNodeAt(int index) const {
            if (index < 0 || index >= count || !head) return nullptr;

            Node* current = head;
            for (int i = 0; i < index; ++i) {
                current = current->next;
                if (circular && current == head) break; // Safety for circular
            }
            return current;
        }

    public:
        LinkedList() : head(nullptr), tail(nullptr), count(0), circular(false) {}

        ~LinkedList() {
            clear();
        }

        // Delete copy/move to keep it simple
        LinkedList(const LinkedList&) = delete;
        LinkedList& operator=(const LinkedList&) = delete;
        LinkedList(LinkedList&&) = delete;
        LinkedList& operator=(LinkedList&&) = delete;

        // Circular mode toggle
        void setCircular(bool on) {
            circular = on;
            updateCircularLinks();
        }

        bool isCircular() const { return circular; }

        // Insert at tail
        void insertTail(const T& value) {
            Node* newNode = new Node(value);
            if (!head) {
                head = tail = newNode;
            }
            else {
                tail->next = newNode;
                newNode->prev = tail;
                tail = newNode;
            }
            count++;
            updateCircularLinks();
        }

        // Insert at head
        void insertHead(const T& value) {
            Node* newNode = new Node(value);
            if (!head) {
                head = tail = newNode;
            }
            else {
                newNode->next = head;
                head->prev = newNode;
                head = newNode;
            }
            count++;
            updateCircularLinks();
        }

        // Insert at index (clamps to [0..size])
        void insertAtIndex(int index, const T& value) {
            if (index <= 0) {
                insertHead(value);
                return;
            }
            if (index >= count) {
                insertTail(value);
                return;
            }

            Node* current = getNodeAt(index);
            if (!current) {
                insertTail(value);
                return;
            }

            Node* newNode = new Node(value);
            Node* prevNode = current->prev;

            newNode->next = current;
            newNode->prev = prevNode;
            if (prevNode) prevNode->next = newNode;
            current->prev = newNode;

            if (current == head) head = newNode;

            count++;
            updateCircularLinks();
        }

        // Sorted insert with comparator
        void sortedInsert(const T& value, std::function<bool(const T&, const T&)> comp = std::less<T>()) {
            if (!head || comp(value, head->data)) {
                insertHead(value);
                return;
            }

            Node* current = head;
            int steps = 0;
            while (current->next && steps < count) {
                if (circular && current->next == head) break;
                if (comp(value, current->next->data)) break;
                current = current->next;
                steps++;
            }

            if (current == tail || (circular && current->next == head)) {
                insertTail(value);
            }
            else {
                Node* newNode = new Node(value);
                newNode->next = current->next;
                newNode->prev = current;
                if (current->next) current->next->prev = newNode;
                current->next = newNode;
                count++;
                updateCircularLinks();
            }
        }

        // Delete head
        bool deleteHead() {
            if (!head) return false;

            Node* toDelete = head;
            if (head == tail) {
                head = tail = nullptr;
            }
            else {
                head = head->next;
                if (head) head->prev = nullptr;
            }
            delete toDelete;
            count--;
            updateCircularLinks();
            return true;
        }

        // Delete tail
        bool deleteTail() {
            if (!tail) return false;

            Node* toDelete = tail;
            if (head == tail) {
                head = tail = nullptr;
            }
            else {
                tail = tail->prev;
                if (tail) tail->next = nullptr;
            }
            delete toDelete;
            count--;
            updateCircularLinks();
            return true;
        }

        // Delete at index
        bool deleteAtIndex(int index) {
            if (index < 0 || index >= count) return false;

            if (index == 0) return deleteHead();
            if (index == count - 1) return deleteTail();

            Node* toDelete = getNodeAt(index);
            if (!toDelete) return false;

            if (toDelete->prev) toDelete->prev->next = toDelete->next;
            if (toDelete->next) toDelete->next->prev = toDelete->prev;

            delete toDelete;
            count--;
            updateCircularLinks();
            return true;
        }

        // Delete by value (first occurrence)
        bool deleteValue(const T& value) {
            if (!head) return false;

            Node* current = head;
            int steps = 0;
            do {
                if (current->data == value) {
                    if (current == head) return deleteHead();
                    if (current == tail) return deleteTail();

                    if (current->prev) current->prev->next = current->next;
                    if (current->next) current->next->prev = current->prev;
                    delete current;
                    count--;
                    updateCircularLinks();
                    return true;
                }
                current = current->next;
                steps++;
            } while (current && current != head && steps < count);

            return false;
        }

        // Linear search
        bool search(const T& value) const {
            if (!head) return false;

            Node* current = head;
            int steps = 0;
            do {
                if (current->data == value) return true;
                current = current->next;
                steps++;
            } while (current && current != head && steps < count);

            return false;
        }

        // Sorted search using slow/fast pointer mid-finding (binary-style on linked list)
        Node* sortedSearch(const T& value, std::function<int(const T&, const T&)> cmp3way) {
            if (!head) return nullptr;

            // For small lists, just linear
            if (count <= 4) {
                Node* current = head;
                int steps = 0;
                do {
                    int cmp = cmp3way(value, current->data);
                    if (cmp == 0) return current;
                    if (cmp < 0) return nullptr; // Past the value
                    current = current->next;
                    steps++;
                } while (current && current != head && steps < count);
                return nullptr;
            }

            // Binary-style: find mid using slow/fast, recurse
            // Simplified version: split list in half
            Node* slow = head;
            Node* fast = head;
            int steps = 0;

            while (fast && fast->next && steps < count) {
                if (circular && fast->next == head) break;
                slow = slow->next;
                fast = fast->next;
                if (fast && fast != head) fast = fast->next;
                steps++;
            }

            // Check mid
            int cmp = cmp3way(value, slow->data);
            if (cmp == 0) return slow;

            // Linear fallback for simplicity (true O(n log n) would need complex recursion)
            Node* current = head;
            steps = 0;
            do {
                int c = cmp3way(value, current->data);
                if (c == 0) return current;
                if (c < 0) return nullptr;
                current = current->next;
                steps++;
            } while (current && current != head && steps < count);

            return nullptr;
        }

        // Bubble sort
        void bubbleSort(std::function<bool(const T&, const T&)> comp = std::less<T>()) {
            if (count < 2) return;

            bool swapped;
            do {
                swapped = false;
                Node* current = head;
                int steps = 0;

                while (current && current->next && steps < count - 1) {
                    if (circular && current->next == head) break;

                    if (!comp(current->data, current->next->data)) {
                        std::swap(current->data, current->next->data);
                        swapped = true;
                    }
                    current = current->next;
                    steps++;
                }
            } while (swapped);
        }

        // Reverse list
        void reverse() {
            if (!head || count < 2) return;

            Node* current = head;
            Node* temp = nullptr;
            tail = head; // New tail is old head

            int steps = 0;
            do {
                temp = current->prev;
                current->prev = current->next;
                current->next = temp;
                current = current->prev; // Move to next (which is now prev)
                steps++;
            } while (current && current != tail && steps < count);

            if (temp) head = temp->prev;
            updateCircularLinks();
        }

        // Clear all nodes
        void clear() {
            if (circular && head) {
                // Break circular link first
                if (tail) tail->next = nullptr;
            }

            while (head) {
                Node* temp = head;
                head = head->next;
                delete temp;
            }
            head = tail = nullptr;
            count = 0;
            circular = false;
        }

        // Get size
        int size() const { return count; }

        // Check if empty
        bool isEmpty() const { return count == 0; }

        // Get data at index
        T* getAtIndex(int index) {
            Node* node = getNodeAt(index);
            return node ? &(node->data) : nullptr;
        }

        // Update data at index
        bool updateAtIndex(int index, const T& value) {
            Node* node = getNodeAt(index);
            if (!node) return false;
            node->data = value;
            return true;
        }

        // Visualize forward
        void visualizeForward(bool detailed = false) const {
            if (!head) {
                std::cout << util::neonGreen() << "[EMPTY LIST]" << util::colorReset() << "\n";
                return;
            }

            std::cout << util::neonGreen();
            Node* current = head;
            int steps = 0;

            if (detailed) {
                do {
                    std::cout << "[";
                    if (current->prev) {
                        if (circular && current->prev == tail) std::cout << "(T)";
                        else std::cout << "*";
                    }
                    else {
                        std::cout << "X";
                    }
                    std::cout << "|" << current->data << "|";
                    if (current->next) {
                        if (circular && current->next == head) std::cout << "(H)";
                        else std::cout << "*";
                    }
                    else {
                        std::cout << "X";
                    }
                    std::cout << "]";

                    if (current->next && !(circular && current->next == head)) {
                        std::cout << " <-> ";
                    }
                    else if (circular && current->next == head) {
                        std::cout << " <-@-> (circular)";
                        break;
                    }

                    current = current->next;
                    steps++;
                } while (current && current != head && steps < count);
            }
            else {
                std::cout << "HEAD -> ";
                do {
                    std::cout << "[" << current->data << "]";
                    current = current->next;
                    steps++;
                    if (current && current != head && steps < count) {
                        std::cout << " <-> ";
                    }
                    else if (circular && current == head) {
                        std::cout << " -@-> HEAD (circular)";
                        break;
                    }
                } while (current && current != head && steps < count);

                if (!circular) std::cout << " <- TAIL";
            }

            std::cout << util::colorReset() << "\n";
        }

        // Visualize backward
        void visualizeBackward(bool detailed = false) const {
            if (!tail) {
                std::cout << util::neonGreen() << "[EMPTY LIST]" << util::colorReset() << "\n";
                return;
            }

            std::cout << util::cyan();
            Node* current = tail;
            int steps = 0;

            if (detailed) {
                do {
                    std::cout << "[";
                    if (current->next) {
                        if (circular && current->next == head) std::cout << "(H)";
                        else std::cout << "*";
                    }
                    else {
                        std::cout << "X";
                    }
                    std::cout << "|" << current->data << "|";
                    if (current->prev) {
                        if (circular && current->prev == tail) std::cout << "(T)";
                        else std::cout << "*";
                    }
                    else {
                        std::cout << "X";
                    }
                    std::cout << "]";

                    if (current->prev && !(circular && current->prev == tail)) {
                        std::cout << " <-> ";
                    }
                    else if (circular && current->prev == tail) {
                        std::cout << " <-@-> (circular)";
                        break;
                    }

                    current = current->prev;
                    steps++;
                } while (current && current != tail && steps < count);
            }
            else {
                std::cout << "TAIL -> ";
                do {
                    std::cout << "[" << current->data << "]";
                    current = current->prev;
                    steps++;
                    if (current && current != tail && steps < count) {
                        std::cout << " <-> ";
                    }
                    else if (circular && current == tail) {
                        std::cout << " -@-> TAIL (circular)";
                        break;
                    }
                } while (current && current != tail && steps < count);

                if (!circular) std::cout << " <- HEAD";
            }

            std::cout << util::colorReset() << "\n";
        }

        // Get head pointer (for internal use)
        Node* getHead() const { return head; }

        // Get tail pointer (for internal use)
        Node* getTail() const { return tail; }
    };

    // ----------------------------------------------------------------------------
    // Stack built on LinkedList
    // ----------------------------------------------------------------------------
    template<typename T>
    class StackLL {
    private:
        LinkedList<T> list;

    public:
        void push(const T& value) {
            list.insertHead(value);
        }

        bool pop() {
            return list.deleteHead();
        }

        T* top() {
            return list.getAtIndex(0);
        }

        bool empty() const {
            return list.isEmpty();
        }

        int size() const {
            return list.size();
        }
    };

    // ----------------------------------------------------------------------------
    // Queue built on LinkedList
    // ----------------------------------------------------------------------------
    template<typename T>
    class QueueLL {
    private:
        LinkedList<T> list;

    public:
        void enqueue(const T& value) {
            list.insertTail(value);
        }

        bool dequeue() {
            return list.deleteHead();
        }

        T* front() {
            return list.getAtIndex(0);
        }

        bool empty() const {
            return list.isEmpty();
        }

        int size() const {
            return list.size();
        }
    };

    // ----------------------------------------------------------------------------
    // Binary Search Tree
    // ----------------------------------------------------------------------------
    template<typename T>
    class BST {
    private:
        struct Node {
            T data;
            Node* left;
            Node* right;

            Node(const T& val) : data(val), left(nullptr), right(nullptr) {}
        };

        Node* root;

        // Helper: insert recursively
        Node* insertHelper(Node* node, const T& value) {
            if (!node) return new Node(value);

            if (value < node->data) {
                node->left = insertHelper(node->left, value);
            }
            else if (value > node->data) {
                node->right = insertHelper(node->right, value);
            }
            // Ignore duplicates
            return node;
        }

        // Helper: search recursively
        bool searchHelper(Node* node, const T& value) const {
            if (!node) return false;
            if (value == node->data) return true;
            if (value < node->data) return searchHelper(node->left, value);
            return searchHelper(node->right, value);
        }

        // Helper: inorder traversal
        void inorderHelper(Node* node) const {
            if (!node) return;
            inorderHelper(node->left);
            std::cout << node->data << " ";
            inorderHelper(node->right);
        }

        // Helper: clear tree
        void clearHelper(Node* node) {
            if (!node) return;
            clearHelper(node->left);
            clearHelper(node->right);
            delete node;
        }

    public:
        BST() : root(nullptr) {}

        ~BST() {
            clearHelper(root);
        }

        BST(const BST&) = delete;
        BST& operator=(const BST&) = delete;

        void insert(const T& value) {
            root = insertHelper(root, value);
        }

        bool search(const T& value) const {
            return searchHelper(root, value);
        }

        void printInOrder() const {
            std::cout << util::neonGreen() << "InOrder: " << util::colorReset();
            inorderHelper(root);
            std::cout << "\n";
        }

        void clear() {
            clearHelper(root);
            root = nullptr;
        }
    };

} // namespace ds

// ============================================================================
// File I/O functions
// ============================================================================
namespace fileio {

    template<typename T>
    bool saveList(const ds::LinkedList<T>& list, const std::string& path, const std::string& typeName) {
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file for writing: " << path << "\n";
            return false;
        }

        file << "# LIST\n";
        file << "type=" << typeName << "\n";
        file << "circular=" << (list.isCircular() ? "1" : "0") << "\n";
        file << "count=" << list.size() << "\n";
        file << "values:";

        for (int i = 0; i < list.size(); ++i) {
            const T* val = const_cast<ds::LinkedList<T>&>(list).getAtIndex(i);
            if (val) {
                file << " " << *val;
            }
        }
        file << "\n";

        file.close();
        return true;
    }

    template<typename T>
    bool loadList(ds::LinkedList<T>& list, const std::string& path, const std::string& typeNameExpected) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file for reading: " << path << "\n";
            return false;
        }

        list.clear();

        std::string line;
        std::string typeName;
        bool isCircular = false;
        int expectedCount = 0;

        // Parse header
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            if (line.find("type=") == 0) {
                typeName = line.substr(5);
            }
            else if (line.find("circular=") == 0) {
                isCircular = (line.substr(9) == "1");
            }
            else if (line.find("count=") == 0) {
                expectedCount = std::stoi(line.substr(6));
            }
            else if (line.find("values:") == 0) {
                std::istringstream iss(line.substr(7));
                T value;
                while (iss >> value) {
                    list.insertTail(value);
                }
                break;
            }
        }

        file.close();

        if (typeName != typeNameExpected) {
            std::cerr << "Warning: Type mismatch. Expected " << typeNameExpected
                << " but got " << typeName << "\n";
        }

        list.setCircular(isCircular);

        std::cout << util::neonGreen() << "Loaded " << list.size() << " items from "
            << path << util::colorReset() << "\n";

        return true;
    }

} // namespace fileio

// ============================================================================
// Performance testing
// ============================================================================
namespace perf {

    template<typename T>
    void timeBulkInsert(ds::LinkedList<T>& list, int n, std::mt19937& gen) {
        auto start = std::chrono::high_resolution_clock::now();

        std::uniform_int_distribution<> dist(1, 1000);
        for (int i = 0; i < n; ++i) {
            if constexpr (std::is_same_v<T, int>) {
                list.insertTail(dist(gen));
            }
            else if constexpr (std::is_same_v<T, double>) {
                list.insertTail(static_cast<double>(dist(gen)) / 10.0);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                list.insertTail(util::randomString(5, gen));
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << util::yellow() << "Bulk Insert (" << n << " items): "
            << duration.count() << " µs" << util::colorReset() << "\n";
    }

    template<typename T>
    void timeLinearSearch(ds::LinkedList<T>& list, int lookups, std::mt19937& gen) {
        if (list.isEmpty()) {
            std::cout << "List is empty, cannot time search.\n";
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();

        std::uniform_int_distribution<> dist(0, list.size() - 1);
        int found = 0;

        for (int i = 0; i < lookups; ++i) {
            T* val = list.getAtIndex(dist(gen));
            if (val && list.search(*val)) {
                found++;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << util::yellow() << "Linear Search (" << lookups << " lookups, "
            << found << " found): " << duration.count() << " µs"
            << util::colorReset() << "\n";
    }

    template<typename T>
    void timeSort(ds::LinkedList<T>& list) {
        auto start = std::chrono::high_resolution_clock::now();
        list.bubbleSort();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << util::yellow() << "Bubble Sort (" << list.size() << " items): "
            << duration.count() << " µs" << util::colorReset() << "\n";
    }

} // namespace perf

// ============================================================================
// Self-test function
// ============================================================================
void selfTest() {
    std::cout << util::cyan() << "\n=== Running Self-Tests ===\n" << util::colorReset();

    ds::LinkedList<int> list;

    // Insert tests
    list.insertTail(10);
    list.insertTail(30);
    list.insertTail(20);
    std::cout << "After insertTail(10,30,20): ";
    list.visualizeForward(false);

    // Sort test
    list.bubbleSort();
    std::cout << "After sort: ";
    list.visualizeForward(false);

    // Sorted insert test
    list.sortedInsert(15);
    std::cout << "After sortedInsert(15): ";
    list.visualizeForward(false);

    // Reverse test
    list.reverse();
    std::cout << "After reverse: ";
    list.visualizeForward(false);

    // Search test
    std::cout << "search(15): " << (list.search(15) ? "FOUND" : "NOT FOUND") << "\n";
    std::cout << "search(999): " << (list.search(999) ? "FOUND" : "NOT FOUND") << "\n";

    // Delete test
    list.deleteValue(15);
    std::cout << "After deleteValue(15): ";
    list.visualizeForward(false);
    std::cout << "search(15) after delete: " << (list.search(15) ? "FOUND" : "NOT FOUND") << "\n";

    // Circular mode test
    list.setCircular(true);
    std::cout << "After setCircular(true): ";
    list.visualizeForward(false);

    list.setCircular(false);
    std::cout << "After setCircular(false): ";
    list.visualizeForward(false);

    // Stack test
    ds::StackLL<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    std::cout << "Stack top: " << (stack.top() ? std::to_string(*stack.top()) : "null") << "\n";
    stack.pop();
    std::cout << "Stack top after pop: " << (stack.top() ? std::to_string(*stack.top()) : "null") << "\n";

    // Queue test
    ds::QueueLL<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    std::cout << "Queue front: " << (queue.front() ? std::to_string(*queue.front()) : "null") << "\n";
    queue.dequeue();
    std::cout << "Queue front after dequeue: " << (queue.front() ? std::to_string(*queue.front()) : "null") << "\n";

    // BST test
    ds::BST<int> bst;
    bst.insert(50);
    bst.insert(30);
    bst.insert(70);
    bst.insert(20);
    bst.insert(40);
    bst.printInOrder();
    std::cout << "BST search(40): " << (bst.search(40) ? "FOUND" : "NOT FOUND") << "\n";

    // File I/O test
    std::cout << "Saving list to test.txt...\n";
    fileio::saveList(list, "test.txt", "int");

    ds::LinkedList<int> list2;
    std::cout << "Loading list from test.txt...\n";
    fileio::loadList(list2, "test.txt", "int");
    std::cout << "Loaded list: ";
    list2.visualizeForward(false);

    std::cout << util::cyan() << "=== Self-Tests Complete ===\n\n" << util::colorReset();
}

// ============================================================================
// UI namespace
// ============================================================================
namespace ui {

    void printHeader() {
        std::cout << util::neonGreen();
        std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║      ULTIMATE LINKED LIST CONTROL PANEL (DS Playground)       ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
        std::cout << util::colorReset();
    }

    void printMenu() {
        std::cout << util::neonGreen();
        std::cout << "\n┌─ MAIN MENU ──────────────────────────────────────────────────┐\n";
        std::cout << "│ [1]  Choose Type (int/double/string)                         │\n";
        std::cout << "│ [2]  Toggle Circular Mode                                     │\n";
        std::cout << "│ [3]  Insert Head                                              │\n";
        std::cout << "│ [4]  Insert Tail                                              │\n";
        std::cout << "│ [5]  Insert At Index                                          │\n";
        std::cout << "│ [6]  Sorted Insert                                            │\n";
        std::cout << "│ [7]  Delete Head                                              │\n";
        std::cout << "│ [8]  Delete Tail                                              │\n";
        std::cout << "│ [9]  Delete At Index                                          │\n";
        std::cout << "│ [10] Delete By Value                                          │\n";
        std::cout << "│ [11] Search (Linear)                                          │\n";
        std::cout << "│ [12] Sorted Search                                            │\n";
        std::cout << "│ [13] Reverse List                                             │\n";
        std::cout << "│ [14] Sort List (Bubble)                                       │\n";
        std::cout << "│ [15] Get Size / IsEmpty                                       │\n";
        std::cout << "│ [16] Get At Index                                             │\n";
        std::cout << "│ [17] Update At Index                                          │\n";
        std::cout << "│ [18] Visualize Forward (compact/detailed)                     │\n";
        std::cout << "│ [19] Visualize Backward (compact/detailed)                    │\n";
        std::cout << "│ [20] Generate Random Data                                     │\n";
        std::cout << "│ [21] Clear List                                               │\n";
        std::cout << "│ [22] Save To File                                             │\n";
        std::cout << "│ [23] Load From File                                           │\n";
        std::cout << "│ [24] Stack Operations                                         │\n";
        std::cout << "│ [25] Queue Operations                                         │\n";
        std::cout << "│ [26] BST Operations                                           │\n";
        std::cout << "│ [27] Performance Timing Suite                                 │\n";
        std::cout << "│ [28] Toggle Color (ON/OFF)                                    │\n";
        std::cout << "│ [0]  Quit                                                     │\n";
        std::cout << "└───────────────────────────────────────────────────────────────┘\n";
        std::cout << util::colorReset();
    }

} // namespace ui

// ============================================================================
// Main application controller
// ============================================================================
class AppController {
private:
    ds::LinkedList<int> listInt;
    ds::LinkedList<double> listDouble;
    ds::LinkedList<std::string> listString;

    ds::StackLL<int> stackInt;
    ds::StackLL<double> stackDouble;
    ds::StackLL<std::string> stackString;

    ds::QueueLL<int> queueInt;
    ds::QueueLL<double> queueDouble;
    ds::QueueLL<std::string> queueString;

    ds::BST<int> bstInt;
    ds::BST<double> bstDouble;
    ds::BST<std::string> bstString;

    std::string currentType;
    std::mt19937 rng;

    template<typename T>
    ds::LinkedList<T>& getList();

    template<typename T>
    ds::StackLL<T>& getStack();

    template<typename T>
    ds::QueueLL<T>& getQueue();

    template<typename T>
    ds::BST<T>& getBST();

public:
    AppController() : currentType("int") {
        std::random_device rd;
        rng.seed(rd());
    }

    void run() {
        bool running = true;

        while (running) {
            util::clearScreen();
            ui::printHeader();

            std::cout << util::cyan() << "Current Type: " << currentType
                << " | Size: ";

            if (currentType == "int") {
                std::cout << listInt.size();
            }
            else if (currentType == "double") {
                std::cout << listDouble.size();
            }
            else {
                std::cout << listString.size();
            }
            std::cout << util::colorReset() << "\n";

            ui::printMenu();

            std::cout << util::neonGreen() << "\nEnter choice: " << util::colorReset();
            int choice;
            if (!util::safeInput(choice)) {
                std::cout << "Invalid input. Try again.\n";
                util::waitForEnter();
                continue;
            }

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            switch (choice) {
            case 0: running = false; break;
            case 1: handleChooseType(); break;
            case 2: handleToggleCircular(); break;
            case 3: handleInsertHead(); break;
            case 4: handleInsertTail(); break;
            case 5: handleInsertAtIndex(); break;
            case 6: handleSortedInsert(); break;
            case 7: handleDeleteHead(); break;
            case 8: handleDeleteTail(); break;
            case 9: handleDeleteAtIndex(); break;
            case 10: handleDeleteByValue(); break;
            case 11: handleSearch(); break;
            case 12: handleSortedSearch(); break;
            case 13: handleReverse(); break;
            case 14: handleSort(); break;
            case 15: handleSizeEmpty(); break;
            case 16: handleGetAtIndex(); break;
            case 17: handleUpdateAtIndex(); break;
            case 18: handleVisualizeForward(); break;
            case 19: handleVisualizeBackward(); break;
            case 20: handleGenerateRandom(); break;
            case 21: handleClearList(); break;
            case 22: handleSaveToFile(); break;
            case 23: handleLoadFromFile(); break;
            case 24: handleStackOps(); break;
            case 25: handleQueueOps(); break;
            case 26: handleBSTOps(); break;
            case 27: handlePerformanceTiming(); break;
            case 28: handleToggleColor(); break;
            default:
                std::cout << "Invalid choice!\n";
                util::waitForEnter();
            }
        }

        std::cout << util::neonGreen() << "\nGoodbye!\n" << util::colorReset();
    }

private:
    void handleChooseType() {
        std::cout << "\nChoose type:\n";
        std::cout << "1. int\n";
        std::cout << "2. double\n";
        std::cout << "3. string\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!util::safeInput(choice)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        switch (choice) {
        case 1: currentType = "int"; break;
        case 2: currentType = "double"; break;
        case 3: currentType = "string"; break;
        default: std::cout << "Invalid choice.\n";
        }

        std::cout << "Type set to: " << currentType << "\n";
        util::waitForEnter();
    }

    void handleToggleCircular() {
        if (currentType == "int") {
            listInt.setCircular(!listInt.isCircular());
            std::cout << "Circular mode: " << (listInt.isCircular() ? "ON" : "OFF") << "\n";
        }
        else if (currentType == "double") {
            listDouble.setCircular(!listDouble.isCircular());
            std::cout << "Circular mode: " << (listDouble.isCircular() ? "ON" : "OFF") << "\n";
        }
        else {
            listString.setCircular(!listString.isCircular());
            std::cout << "Circular mode: " << (listString.isCircular() ? "ON" : "OFF") << "\n";
        }
        util::waitForEnter();
    }

    void handleInsertHead() {
        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                listInt.insertHead(val);
                std::cout << "Inserted " << val << " at head.\n";
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                listDouble.insertHead(val);
                std::cout << "Inserted " << val << " at head.\n";
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            listString.insertHead(val);
            std::cout << "Inserted " << val << " at head.\n";
        }
        util::waitForEnter();
    }

    void handleInsertTail() {
        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                listInt.insertTail(val);
                std::cout << "Inserted " << val << " at tail.\n";
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                listDouble.insertTail(val);
                std::cout << "Inserted " << val << " at tail.\n";
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            listString.insertTail(val);
            std::cout << "Inserted " << val << " at tail.\n";
        }
        util::waitForEnter();
    }

    void handleInsertAtIndex() {
        std::cout << "Enter index: ";
        int index;
        if (!util::safeInput(index)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                listInt.insertAtIndex(index, val);
                std::cout << "Inserted " << val << " at index " << index << ".\n";
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                listDouble.insertAtIndex(index, val);
                std::cout << "Inserted " << val << " at index " << index << ".\n";
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            listString.insertAtIndex(index, val);
            std::cout << "Inserted " << val << " at index " << index << ".\n";
        }
        util::waitForEnter();
    }

    void handleSortedInsert() {
        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                listInt.sortedInsert(val);
                std::cout << "Sorted insert of " << val << " completed.\n";
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                listDouble.sortedInsert(val);
                std::cout << "Sorted insert of " << val << " completed.\n";
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            listString.sortedInsert(val);
            std::cout << "Sorted insert of " << val << " completed.\n";
        }
        util::waitForEnter();
    }

    void handleDeleteHead() {
        bool result = false;
        if (currentType == "int") {
            result = listInt.deleteHead();
        }
        else if (currentType == "double") {
            result = listDouble.deleteHead();
        }
        else {
            result = listString.deleteHead();
        }
        std::cout << (result ? "Head deleted." : "List is empty.") << "\n";
        util::waitForEnter();
    }

    void handleDeleteTail() {
        bool result = false;
        if (currentType == "int") {
            result = listInt.deleteTail();
        }
        else if (currentType == "double") {
            result = listDouble.deleteTail();
        }
        else {
            result = listString.deleteTail();
        }
        std::cout << (result ? "Tail deleted." : "List is empty.") << "\n";
        util::waitForEnter();
    }

    void handleDeleteAtIndex() {
        std::cout << "Enter index: ";
        int index;
        if (!util::safeInput(index)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        bool result = false;
        if (currentType == "int") {
            result = listInt.deleteAtIndex(index);
        }
        else if (currentType == "double") {
            result = listDouble.deleteAtIndex(index);
        }
        else {
            result = listString.deleteAtIndex(index);
        }
        std::cout << (result ? "Deleted at index." : "Invalid index.") << "\n";
        util::waitForEnter();
    }

    void handleDeleteByValue() {
        bool result = false;
        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                result = listInt.deleteValue(val);
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                result = listDouble.deleteValue(val);
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            result = listString.deleteValue(val);
        }
        std::cout << (result ? "Value deleted." : "Value not found.") << "\n";
        util::waitForEnter();
    }

    void handleSearch() {
        bool result = false;
        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                result = listInt.search(val);
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                result = listDouble.search(val);
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            result = listString.search(val);
        }
        std::cout << (result ? "Value FOUND." : "Value NOT FOUND.") << "\n";
        util::waitForEnter();
    }

    void handleSortedSearch() {
        std::cout << "Note: List should be sorted for optimal results.\n";

        if (currentType == "int") {
            std::cout << "Enter int value: ";
            int val;
            if (util::safeInput(val)) {
                auto cmp = [](const int& a, const int& b) -> int {
                    if (a < b) return -1;
                    if (a > b) return 1;
                    return 0;
                    };
                auto* node = listInt.sortedSearch(val, cmp);
                std::cout << (node ? "Value FOUND." : "Value NOT FOUND.") << "\n";
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter double value: ";
            double val;
            if (util::safeInput(val)) {
                auto cmp = [](const double& a, const double& b) -> int {
                    if (a < b) return -1;
                    if (a > b) return 1;
                    return 0;
                    };
                auto* node = listDouble.sortedSearch(val, cmp);
                std::cout << (node ? "Value FOUND." : "Value NOT FOUND.") << "\n";
            }
        }
        else {
            std::cout << "Enter string value: ";
            std::string val;
            std::cin >> val;
            auto cmp = [](const std::string& a, const std::string& b) -> int {
                if (a < b) return -1;
                if (a > b) return 1;
                return 0;
                };
            auto* node = listString.sortedSearch(val, cmp);
            std::cout << (node ? "Value FOUND." : "Value NOT FOUND.") << "\n";
        }
        util::waitForEnter();
    }

    void handleReverse() {
        if (currentType == "int") {
            listInt.reverse();
        }
        else if (currentType == "double") {
            listDouble.reverse();
        }
        else {
            listString.reverse();
        }
        std::cout << "List reversed.\n";
        util::waitForEnter();
    }

    void handleSort() {
        if (currentType == "int") {
            listInt.bubbleSort();
        }
        else if (currentType == "double") {
            listDouble.bubbleSort();
        }
        else {
            listString.bubbleSort();
        }
        std::cout << "List sorted.\n";
        util::waitForEnter();
    }

    void handleSizeEmpty() {
        int size = 0;
        bool empty = true;

        if (currentType == "int") {
            size = listInt.size();
            empty = listInt.isEmpty();
        }
        else if (currentType == "double") {
            size = listDouble.size();
            empty = listDouble.isEmpty();
        }
        else {
            size = listString.size();
            empty = listString.isEmpty();
        }

        std::cout << "Size: " << size << "\n";
        std::cout << "Empty: " << (empty ? "YES" : "NO") << "\n";
        util::waitForEnter();
    }

    void handleGetAtIndex() {
        std::cout << "Enter index: ";
        int index;
        if (!util::safeInput(index)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        if (currentType == "int") {
            int* val = listInt.getAtIndex(index);
            if (val) {
                std::cout << "Value at index " << index << ": " << *val << "\n";
            }
            else {
                std::cout << "Invalid index.\n";
            }
        }
        else if (currentType == "double") {
            double* val = listDouble.getAtIndex(index);
            if (val) {
                std::cout << "Value at index " << index << ": " << *val << "\n";
            }
            else {
                std::cout << "Invalid index.\n";
            }
        }
        else {
            std::string* val = listString.getAtIndex(index);
            if (val) {
                std::cout << "Value at index " << index << ": " << *val << "\n";
            }
            else {
                std::cout << "Invalid index.\n";
            }
        }
        util::waitForEnter();
    }

    void handleUpdateAtIndex() {
        std::cout << "Enter index: ";
        int index;
        if (!util::safeInput(index)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        bool result = false;
        if (currentType == "int") {
            std::cout << "Enter new int value: ";
            int val;
            if (util::safeInput(val)) {
                result = listInt.updateAtIndex(index, val);
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter new double value: ";
            double val;
            if (util::safeInput(val)) {
                result = listDouble.updateAtIndex(index, val);
            }
        }
        else {
            std::cout << "Enter new string value: ";
            std::string val;
            std::cin >> val;
            result = listString.updateAtIndex(index, val);
        }
        std::cout << (result ? "Updated successfully." : "Invalid index.") << "\n";
        util::waitForEnter();
    }

    void handleVisualizeForward() {
        std::cout << "Detailed view? (0=compact, 1=detailed): ";
        int choice;
        bool detailed = false;
        if (util::safeInput(choice)) {
            detailed = (choice == 1);
        }

        std::cout << "\nForward visualization:\n";
        if (currentType == "int") {
            listInt.visualizeForward(detailed);
        }
        else if (currentType == "double") {
            listDouble.visualizeForward(detailed);
        }
        else {
            listString.visualizeForward(detailed);
        }
        util::waitForEnter();
    }

    void handleVisualizeBackward() {
        std::cout << "Detailed view? (0=compact, 1=detailed): ";
        int choice;
        bool detailed = false;
        if (util::safeInput(choice)) {
            detailed = (choice == 1);
        }

        std::cout << "\nBackward visualization:\n";
        if (currentType == "int") {
            listInt.visualizeBackward(detailed);
        }
        else if (currentType == "double") {
            listDouble.visualizeBackward(detailed);
        }
        else {
            listString.visualizeBackward(detailed);
        }
        util::waitForEnter();
    }

    void handleGenerateRandom() {
        std::cout << "Enter count: ";
        int count;
        if (!util::safeInput(count) || count <= 0) {
            std::cout << "Invalid count.\n";
            util::waitForEnter();
            return;
        }

        std::cout << "Enter seed (0 for random): ";
        int seed;
        if (util::safeInput(seed) && seed != 0) {
            rng.seed(seed);
        }

        if (currentType == "int") {
            std::cout << "Enter min value: ";
            int minVal;
            std::cout << "Enter max value: ";
            int maxVal;
            if (util::safeInput(minVal) && util::safeInput(maxVal)) {
                std::uniform_int_distribution<> dist(minVal, maxVal);
                for (int i = 0; i < count; ++i) {
                    listInt.insertTail(dist(rng));
                }
                std::cout << "Generated " << count << " random integers.\n";
            }
        }
        else if (currentType == "double") {
            std::cout << "Enter min value: ";
            double minVal;
            std::cout << "Enter max value: ";
            double maxVal;
            if (util::safeInput(minVal) && util::safeInput(maxVal)) {
                std::uniform_real_distribution<> dist(minVal, maxVal);
                for (int i = 0; i < count; ++i) {
                    listDouble.insertTail(dist(rng));
                }
                std::cout << "Generated " << count << " random doubles.\n";
            }
        }
        else {
            std::cout << "Enter string length: ";
            int length;
            if (util::safeInput(length) && length > 0) {
                for (int i = 0; i < count; ++i) {
                    listString.insertTail(util::randomString(length, rng));
                }
                std::cout << "Generated " << count << " random strings.\n";
            }
        }
        util::waitForEnter();
    }

    void handleClearList() {
        if (currentType == "int") {
            listInt.clear();
        }
        else if (currentType == "double") {
            listDouble.clear();
        }
        else {
            listString.clear();
        }
        std::cout << "List cleared.\n";
        util::waitForEnter();
    }

    void handleSaveToFile() {
        std::cout << "Enter filename: ";
        std::string filename;
        std::cin >> filename;

        bool result = false;
        if (currentType == "int") {
            result = fileio::saveList(listInt, filename, "int");
        }
        else if (currentType == "double") {
            result = fileio::saveList(listDouble, filename, "double");
        }
        else {
            result = fileio::saveList(listString, filename, "string");
        }

        std::cout << (result ? "Saved successfully." : "Save failed.") << "\n";
        util::waitForEnter();
    }

    void handleLoadFromFile() {
        std::cout << "Enter filename: ";
        std::string filename;
        std::cin >> filename;

        bool result = false;
        if (currentType == "int") {
            result = fileio::loadList(listInt, filename, "int");
        }
        else if (currentType == "double") {
            result = fileio::loadList(listDouble, filename, "double");
        }
        else {
            result = fileio::loadList(listString, filename, "string");
        }

        std::cout << (result ? "Loaded successfully." : "Load failed.") << "\n";
        util::waitForEnter();
    }

    void handleStackOps() {
        std::cout << "\nStack Operations:\n";
        std::cout << "1. Push\n";
        std::cout << "2. Pop\n";
        std::cout << "3. Top\n";
        std::cout << "4. Size\n";
        std::cout << "5. Empty\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!util::safeInput(choice)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        if (currentType == "int") {
            handleStackOpsTyped(stackInt);
        }
        else if (currentType == "double") {
            handleStackOpsTyped(stackDouble);
        }
        else {
            handleStackOpsTyped(stackString);
        }
    }

    template<typename T>
    void handleStackOpsTyped(ds::StackLL<T>& stack) {
        std::cout << "Enter operation choice: ";
        int choice;
        if (!util::safeInput(choice)) {
            util::waitForEnter();
            return;
        }

        switch (choice) {
        case 1: {
            std::cout << "Enter value: ";
            T val;
            if constexpr (std::is_same_v<T, std::string>) {
                std::cin >> val;
            }
            else {
                if (!util::safeInput(val)) {
                    util::waitForEnter();
                    return;
                }
            }
            stack.push(val);
            std::cout << "Pushed " << val << "\n";
            break;
        }
        case 2: {
            bool result = stack.pop();
            std::cout << (result ? "Popped." : "Stack empty.") << "\n";
            break;
        }
        case 3: {
            T* val = stack.top();
            if (val) {
                std::cout << "Top: " << *val << "\n";
            }
            else {
                std::cout << "Stack empty.\n";
            }
            break;
        }
        case 4: {
            std::cout << "Size: " << stack.size() << "\n";
            break;
        }
        case 5: {
            std::cout << "Empty: " << (stack.empty() ? "YES" : "NO") << "\n";
            break;
        }
        default:
            std::cout << "Invalid choice.\n";
        }
        util::waitForEnter();
    }

    void handleQueueOps() {
        std::cout << "\nQueue Operations:\n";
        std::cout << "1. Enqueue\n";
        std::cout << "2. Dequeue\n";
        std::cout << "3. Front\n";
        std::cout << "4. Size\n";
        std::cout << "5. Empty\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!util::safeInput(choice)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        if (currentType == "int") {
            handleQueueOpsTyped(queueInt);
        }
        else if (currentType == "double") {
            handleQueueOpsTyped(queueDouble);
        }
        else {
            handleQueueOpsTyped(queueString);
        }
    }

    template<typename T>
    void handleQueueOpsTyped(ds::QueueLL<T>& queue) {
        std::cout << "Enter operation choice: ";
        int choice;
        if (!util::safeInput(choice)) {
            util::waitForEnter();
            return;
        }

        switch (choice) {
        case 1: {
            std::cout << "Enter value: ";
            T val;
            if constexpr (std::is_same_v<T, std::string>) {
                std::cin >> val;
            }
            else {
                if (!util::safeInput(val)) {
                    util::waitForEnter();
                    return;
                }
            }
            queue.enqueue(val);
            std::cout << "Enqueued " << val << "\n";
            break;
        }
        case 2: {
            bool result = queue.dequeue();
            std::cout << (result ? "Dequeued." : "Queue empty.") << "\n";
            break;
        }
        case 3: {
            T* val = queue.front();
            if (val) {
                std::cout << "Front: " << *val << "\n";
            }
            else {
                std::cout << "Queue empty.\n";
            }
            break;
        }
        case 4: {
            std::cout << "Size: " << queue.size() << "\n";
            break;
        }
        case 5: {
            std::cout << "Empty: " << (queue.empty() ? "YES" : "NO") << "\n";
            break;
        }
        default:
            std::cout << "Invalid choice.\n";
        }
        util::waitForEnter();
    }

    void handleBSTOps() {
        std::cout << "\nBST Operations:\n";
        std::cout << "1. Insert\n";
        std::cout << "2. Search\n";
        std::cout << "3. Print InOrder\n";
        std::cout << "4. Clear\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!util::safeInput(choice)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        if (currentType == "int") {
            handleBSTOpsTyped(bstInt);
        }
        else if (currentType == "double") {
            handleBSTOpsTyped(bstDouble);
        }
        else {
            handleBSTOpsTyped(bstString);
        }
    }

    template<typename T>
    void handleBSTOpsTyped(ds::BST<T>& bst) {
        std::cout << "Enter operation choice: ";
        int choice;
        if (!util::safeInput(choice)) {
            util::waitForEnter();
            return;
        }

        switch (choice) {
        case 1: {
            std::cout << "Enter value: ";
            T val;
            if constexpr (std::is_same_v<T, std::string>) {
                std::cin >> val;
            }
            else {
                if (!util::safeInput(val)) {
                    util::waitForEnter();
                    return;
                }
            }
            bst.insert(val);
            std::cout << "Inserted " << val << "\n";
            break;
        }
        case 2: {
            std::cout << "Enter value: ";
            T val;
            if constexpr (std::is_same_v<T, std::string>) {
                std::cin >> val;
            }
            else {
                if (!util::safeInput(val)) {
                    util::waitForEnter();
                    return;
                }
            }
            bool result = bst.search(val);
            std::cout << (result ? "Value FOUND." : "Value NOT FOUND.") << "\n";
            break;
        }
        case 3: {
            bst.printInOrder();
            break;
        }
        case 4: {
            bst.clear();
            std::cout << "BST cleared.\n";
            break;
        }
        default:
            std::cout << "Invalid choice.\n";
        }
        util::waitForEnter();
    }

    void handlePerformanceTiming() {
        std::cout << "\nPerformance Timing Suite:\n";
        std::cout << "1. Time Bulk Insert\n";
        std::cout << "2. Time Linear Search\n";
        std::cout << "3. Time Sort\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!util::safeInput(choice)) {
            std::cout << "Invalid input.\n";
            util::waitForEnter();
            return;
        }

        switch (choice) {
        case 1: {
            std::cout << "Enter count: ";
            int count;
            if (util::safeInput(count) && count > 0) {
                if (currentType == "int") {
                    perf::timeBulkInsert(listInt, count, rng);
                }
                else if (currentType == "double") {
                    perf::timeBulkInsert(listDouble, count, rng);
                }
                else {
                    perf::timeBulkInsert(listString, count, rng);
                }
            }
            break;
        }
        case 2: {
            std::cout << "Enter lookup count: ";
            int lookups;
            if (util::safeInput(lookups) && lookups > 0) {
                if (currentType == "int") {
                    perf::timeLinearSearch(listInt, lookups, rng);
                }
                else if (currentType == "double") {
                    perf::timeLinearSearch(listDouble, lookups, rng);
                }
                else {
                    perf::timeLinearSearch(listString, lookups, rng);
                }
            }
            break;
        }
        case 3: {
            if (currentType == "int") {
                perf::timeSort(listInt);
            }
            else if (currentType == "double") {
                perf::timeSort(listDouble);
            }
            else {
                perf::timeSort(listString);
            }
            break;
        }
        default:
            std::cout << "Invalid choice.\n";
        }
        util::waitForEnter();
    }

    void handleToggleColor() {
        // This is a compile-time setting, but we can inform the user
#if USE_COLOR
        std::cout << "Color is currently ENABLED (compile-time setting).\n";
        std::cout << "To disable, set USE_COLOR to 0 and recompile.\n";
#else
        std::cout << "Color is currently DISABLED (compile-time setting).\n";
        std::cout << "To enable, set USE_COLOR to 1 and recompile.\n";
#endif
        util::waitForEnter();
    }
};

// Template specializations for getList
template<>
ds::LinkedList<int>& AppController::getList<int>() { return listInt; }

template<>
ds::LinkedList<double>& AppController::getList<double>() { return listDouble; }

template<>
ds::LinkedList<std::string>& AppController::getList<std::string>() { return listString; }

template<>
ds::StackLL<int>& AppController::getStack<int>() { return stackInt; }

template<>
ds::StackLL<double>& AppController::getStack<double>() { return stackDouble; }

template<>
ds::StackLL<std::string>& AppController::getStack<std::string>() { return stackString; }

template<>
ds::QueueLL<int>& AppController::getQueue<int>() { return queueInt; }

template<>
ds::QueueLL<double>& AppController::getQueue<double>() { return queueDouble; }

template<>
ds::QueueLL<std::string>& AppController::getQueue<std::string>() { return queueString; }

template<>
ds::BST<int>& AppController::getBST<int>() { return bstInt; }

template<>
ds::BST<double>& AppController::getBST<double>() { return bstDouble; }

template<>
ds::BST<std::string>& AppController::getBST<std::string>() { return bstString; }

// ============================================================================
// Optional SFML GUI stub
// ============================================================================
#if ENABLE_SFML
namespace gui {

    class GUIWindow {
    private:
        sf::RenderWindow window;

    public:
        GUIWindow() : window(sf::VideoMode(800, 600), "DS Playground GUI") {}

        void run() {
            while (window.isOpen()) {
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        window.close();
                    }
                }

                window.clear(sf::Color::Black);

                // Draw nodes as boxes with lines
                // TODO: Implement node visualization

                window.display();
            }
        }
    };

} // namespace gui
#endif

// ============================================================================
// Main entry point
// ============================================================================
int main() {
    // Run self-tests first
    selfTest();

    std::cout << util::neonGreen()
        << "\nSelf-tests complete. Starting main application...\n"
        << util::colorReset();

    util::waitForEnter();

    // Run main application
    AppController app;
    app.run();

#if ENABLE_SFML
    // Optional GUI
    std::cout << "\nLaunch GUI? (y/n): ";
    char choice;
    std::cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        gui::GUIWindow guiWindow;
        guiWindow.run();
    }
#endif

    return 0;
}
