#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <future>

// Red-Black Tree Implementation
template <typename T>
class RedBlackTree {
private:
    struct Node {
        T value;
        bool color; // true for Red, false for Black
        std::shared_ptr<Node> left, right, parent;

        Node(const T& val) : value(val), color(true), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    std::shared_ptr<Node> root;

    void leftRotate(std::shared_ptr<Node> x) {
        auto y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;

        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;

        y->left = x;
        x->parent = y;
    }

    void rightRotate(std::shared_ptr<Node> y) {
        auto x = y->left;
        y->left = x->right;
        if (x->right) x->right->parent = y;

        x->parent = y->parent;
        if (!y->parent) root = x;
        else if (y == y->parent->right) y->parent->right = x;
        else y->parent->left = x;

        x->right = y;
        y->parent = x;
    }

    void insertFixup(std::shared_ptr<Node> z) {
        while (z->parent && z->parent->color) {
            if (z->parent == z->parent->parent->left) {
                auto y = z->parent->parent->right;
                if (y && y->color) { // Case 1: Uncle is red
                    z->parent->color = false;
                    y->color = false;
                    z->parent->parent->color = true;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) { // Case 2: Uncle is black
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = false; // Case 3: Uncle is black
                    z->parent->parent->color = true;
                    rightRotate(z->parent->parent);
                }
            } else {
                auto y = z->parent->parent->left;
                if (y && y->color) {
                    z->parent->color = false;
                    y->color = false;
                    z->parent->parent->color = true;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = false;
                    z->parent->parent->color = true;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = false;
    }

    void inOrderTraversalHelper(std::shared_ptr<Node> node, std::vector<T>& result) const {
        if (node) {
            inOrderTraversalHelper(node->left, result);
            result.push_back(node->value);
            inOrderTraversalHelper(node->right, result);
        }
    }

public:
    void insert(const T& value) {
        auto z = std::make_shared<Node>(value);
        auto y = std::shared_ptr<Node>(nullptr);
        auto x = root;

        while (x) {
            y = x;
            if (z->value < x->value) x = x->left;
            else if (z->value > x->value) x = x->right;
            else return; // No duplicates
        }

        z->parent = y;
        if (!y) root = z;
        else if (z->value < y->value) y->left = z;
        else y->right = z;

        insertFixup(z);
    }

    std::vector<T> inOrderTraversal() const {
        std::vector<T> result;
        inOrderTraversalHelper(root, result);
        return result;
    }
};

// Read the file into a string
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

// Tokenize the text
std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> words;
    std::string word;

    for (char ch : text) {
        if (std::isalnum(ch) || ch == '\'') { // Include apostrophes
            word += std::tolower(ch);
        } else if (!word.empty()) { // Push current word if punctuation or space is found
            words.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) { // Add the last word if any
        words.push_back(word);
    }

    return words;
}

// Parallel tokenization
std::vector<std::string> parallelTokenize(const std::string& text) {
    size_t mid = text.size() / 2;

    // Move `mid` to the next space or punctuation
    while (mid > 0 && std::isalnum(text[mid])) {
        --mid;
    }

    auto future1 = std::async(std::launch::async, tokenize, text.substr(0, mid));
    auto future2 = std::async(std::launch::async, tokenize, text.substr(mid));

    auto words1 = future1.get();
    auto words2 = future2.get();
    words1.insert(words1.end(), words2.begin(), words2.end());
    return words1;
}

// Write sorted words to a file
void writeToFile(const std::string& filePath, const std::vector<std::string>& words) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    for (const auto& word : words) {
        file << word << "\n";
    }
}

// **TEST CASES**
TEST_CASE("Test File Reading") {
    std::ofstream testFile("test_file.txt");
    testFile << "Hello world! Functional programming in C++.";
    testFile.close();

    CHECK(readFile("test_file.txt") == "Hello world! Functional programming in C++.");
    std::filesystem::remove("test_file.txt");
}

TEST_CASE("Test Tokenization") {
    std::string text = "Hello, world! Functional-programming in C++.";
    auto words = tokenize(text);

    std::vector<std::string> expected{"hello", "world", "functional", "programming", "in", "c"};
    REQUIRE(words == expected);
}

TEST_CASE("Test Parallel Tokenization") {
    std::string text = "Hello, world! Functional-programming in C++. Let's test this!";
    auto words = parallelTokenize(text);

    std::vector<std::string> expected{"hello", "world", "functional", "programming", "in", "c", "let's", "test", "this"};

    if (words != expected) {
        std::cerr << "\nParallel Tokenization Failed!\n";
        std::cerr << "Expected (" << expected.size() << "): ";
        for (const auto& w : expected) {
            std::cerr << "\"" << w << "\" ";
        }
        std::cerr << "\nActual (" << words.size() << "): ";
        for (const auto& w : words) {
            std::cerr << "\"" << w << "\" ";
        }
        std::cerr << "\n";
    }
    REQUIRE(words == expected);
}

TEST_CASE("Test Red-Black Tree Insertion") {
    RedBlackTree<std::string> tree;

    tree.insert("functional");
    tree.insert("programming");
    tree.insert("in");
    tree.insert("c");
    tree.insert("functional"); // Duplicate

    auto sorted = tree.inOrderTraversal();
    std::vector<std::string> expected{"c", "functional", "in", "programming"};
    CHECK(sorted == expected);
}

TEST_CASE("Test Writing to File") {
    std::string filePath = "output_test.txt";
    std::vector<std::string> words{"apple", "banana", "cherry"};

    writeToFile(filePath, words);

    std::ifstream file(filePath);
    std::ostringstream content;
    content << file.rdbuf();

    std::string expected = "apple\nbanana\ncherry\n";
    CHECK(content.str() == expected);

    std::filesystem::remove(filePath);
}

int main(int argc, char** argv) {
    doctest::Context context;

    // Run tests
    std::cout << "\nRunning tests...\n";
    int testResult = context.run(); // Run doctest

    if (testResult == 0) {
        std::cout << "\nAll test cases passed!\n";
    } else {
        std::cout << "\nSome test cases failed. Check the details above.\n";
    }

    // Main program logic
    try {
        std::cout << "Enter the path to the file: ";
        std::string filePath;
        std::getline(std::cin, filePath);

        std::string text = readFile(filePath);
        auto words = tokenize(text);

        RedBlackTree<std::string> tree;
        for (const auto& word : words) {
            tree.insert(word);
        }

        auto sortedWords = tree.inOrderTraversal();
        writeToFile("output.txt", sortedWords);

        std::cout << "The sorted list of unique words has been written to output.txt.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return testResult; // Return test results
}