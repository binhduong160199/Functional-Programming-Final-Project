#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <future>
#include <cctype>
#include <iterator>

// Simplified Red-Black Tree Implementation
template <typename T>
struct ImmutableRedBlackTree {
    struct Node {
        T value;
        bool color; // true = Red, false = Black
        std::shared_ptr<Node> left, right;

        Node(const T& val, bool color, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr)
            : value(val), color(color), left(left), right(right) {}
    };

    std::shared_ptr<Node> root;

    ImmutableRedBlackTree() : root(nullptr) {}

    ImmutableRedBlackTree(std::shared_ptr<Node> root) : root(root) {}

    std::shared_ptr<Node> balance(std::shared_ptr<Node> node) const {
        if (!node) return nullptr;

        // If left child and left-left grandchild are red
        if (node->left && node->left->color && node->left->left && node->left->left->color) {
            return std::make_shared<Node>(node->left->value, true,
                                          node->left->left,
                                          std::make_shared<Node>(node->value, false, node->left->right, node->right));
        }

        // If left child and left-right grandchild are red
        if (node->left && node->left->color && node->left->right && node->left->right->color) {
            return std::make_shared<Node>(node->left->right->value, true,
                                          std::make_shared<Node>(node->left->value, false, node->left->left, node->left->right->left),
                                          std::make_shared<Node>(node->value, false, node->left->right->right, node->right));
        }

        // If right child and right-right grandchild are red
        if (node->right && node->right->color && node->right->right && node->right->right->color) {
            return std::make_shared<Node>(node->right->value, true,
                                          std::make_shared<Node>(node->value, false, node->left, node->right->left),
                                          node->right->right);
        }

        // If right child and right-left grandchild are red
        if (node->right && node->right->color && node->right->left && node->right->left->color) {
            return std::make_shared<Node>(node->right->left->value, true,
                                          std::make_shared<Node>(node->value, false, node->left, node->right->left->left),
                                          std::make_shared<Node>(node->right->value, false, node->right->left->right, node->right->right));
        }

        return node;
    }

    std::shared_ptr<Node> insert(std::shared_ptr<Node> node, const T& value) const {
        if (!node) return std::make_shared<Node>(value, true); // Insert as red node

        if (value < node->value) {
            return balance(std::make_shared<Node>(node->value, node->color, insert(node->left, value), node->right));
        } else if (value > node->value) {
            return balance(std::make_shared<Node>(node->value, node->color, node->left, insert(node->right, value)));
        }

        return node; // No duplicates allowed
    }

    ImmutableRedBlackTree insert(const T& value) const {
        auto newRoot = insert(root, value);
        return ImmutableRedBlackTree(std::make_shared<Node>(newRoot->value, false, newRoot->left, newRoot->right));
    }

    void inOrderTraversalHelper(const std::shared_ptr<Node>& node, std::vector<T>& result) const {
        if (node) {
            inOrderTraversalHelper(node->left, result);
            result.push_back(node->value);
            inOrderTraversalHelper(node->right, result);
        }
    }

    std::vector<T> getSortedValues() const {
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
    // Convert the entire text to lowercase and replace unwanted characters with spaces
    std::string processed;
    std::transform(text.begin(), text.end(), std::back_inserter(processed), [](unsigned char c) {
        return (std::isalnum(c) || c == '\'') ? std::tolower(c) : ' ';
    });

    // Use a stringstream to split the processed string into words
    std::istringstream iss(processed);
    return {std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
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
    try {
        std::ofstream file(filePath, std::ios::out | std::ios::trunc); // Ensure the file is opened for writing and truncated
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + filePath);
        }

        // Write words to the file
        std::for_each(words.begin(), words.end(), [&file](const std::string& word) {
            file << word << "\n";
        });

    } catch (const std::exception& e) {
        std::cerr << "Error while writing to file: " << e.what() << "\n";
        throw; // Rethrow the exception for the caller to handle
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
    std::string text = "Hello, world! Functional-programming in C. Let's test this!";
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
    ImmutableRedBlackTree<std::string> tree;

    tree = tree.insert("functional");
    tree = tree.insert("programming");
    tree = tree.insert("in");
    tree = tree.insert("c");
    tree = tree.insert("functional"); // Duplicate

    auto sorted = tree.getSortedValues();
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

// Helper function to merge two red-black trees
template <typename T>
ImmutableRedBlackTree<T> mergeTrees(const ImmutableRedBlackTree<T>& tree1, const ImmutableRedBlackTree<T>& tree2) {
    auto sortedValues = tree2.getSortedValues();
    ImmutableRedBlackTree<T> mergedTree = tree1;
    for (const auto& value : sortedValues) {
        mergedTree = mergedTree.insert(value);
    }
    return mergedTree;
}

// Parallel insertion function
template <typename T>
ImmutableRedBlackTree<T> parallelInsert(const std::vector<T>& words) {
    if (words.empty()) {
        return ImmutableRedBlackTree<T>();
    }

    size_t mid = words.size() / 2;

    // Launch asynchronous tasks for each half
    auto futureTree1 = std::async(std::launch::async, [&]() {
        ImmutableRedBlackTree<T> tree;
        for (size_t i = 0; i < mid; ++i) {
            tree = tree.insert(words[i]);
        }
        return tree;
    });

    auto futureTree2 = std::async(std::launch::async, [&]() {
        ImmutableRedBlackTree<T> tree;
        for (size_t i = mid; i < words.size(); ++i) {
            tree = tree.insert(words[i]);
        }
        return tree;
    });

    // Get the results and merge the trees
    ImmutableRedBlackTree<T> tree1 = futureTree1.get();
    ImmutableRedBlackTree<T> tree2 = futureTree2.get();
    return mergeTrees(tree1, tree2);
}

// Updated processFile function to use parallel insertion
void processFile(const std::string& inputPath, const std::string& outputPath) {
    try {
        // Read the file and tokenize
        std::string text = readFile(inputPath);
        std::vector<std::string> words = parallelTokenize(text);

        // Perform parallel insertion into the red-black tree
        ImmutableRedBlackTree<std::string> tree = parallelInsert(words);

        // Retrieve sorted values from the tree
        std::vector<std::string> sortedWords = tree.getSortedValues();

        // Write sorted words to the output file
        writeToFile(outputPath, sortedWords);

        std::cout << "The sorted list of unique words has been written to " << outputPath << ".\n";
    } catch (const std::exception& e) {
        std::cerr << "Error during file processing: " << e.what() << "\n";
        throw;
    }
}

// Main function remains unchanged
int main(int argc, char** argv) {
    doctest::Context context;

    // Run tests
    std::cout << "\nRunning tests...\n";
    int testResult = context.run(); // Run doctest

    if (testResult != 0) {
        std::cerr << "\nSome test cases failed. Check the details above.\n";
        return testResult;
    }
    std::cout << "\nAll test cases passed!\n";

    try {
        // Prompt user for input file path
        std::cout << "Enter the path to the input file: ";
        std::string inputPath;
        std::getline(std::cin, inputPath);

        // Process the file and generate output
        const std::string outputPath = "output.txt";
        processFile(inputPath, outputPath);
    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << "\n";
        return 1;
    }

    return 0;
}