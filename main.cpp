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
#include <numeric>

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
        if (!root) return {};

        std::vector<T> result;

        // Define a helper function for in-order traversal
        std::function<void(const std::shared_ptr<Node>&)> traverse =
            [&](const std::shared_ptr<Node>& node) {
                if (!node) return;
                traverse(node->left);
                result.push_back(node->value);
                traverse(node->right);
        };

        traverse(root);
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

// Trim leading and trailing apostrophes from a word
std::string trimApostrophes(const std::string& word) {
    size_t start = word.find_first_not_of("'");
    size_t end = word.find_last_not_of("'");
    return (start == std::string::npos) ? "" : word.substr(start, end - start + 1);
}

// Tokenize the text
std::vector<std::string> tokenize(const std::string& text) {
    // Step 1: Convert the text to lowercase and replace unwanted characters
    auto preprocess = [](const std::string& input) {
        return std::accumulate(input.begin(), input.end(), std::string(), [](std::string acc, unsigned char c) {
            acc.push_back((std::isalnum(c) || c == '\'') ? std::tolower(c) : ' ');
            return acc;
        });
    };

    // Step 2: Split the processed text into words
    auto splitIntoWords = [](const std::string& processed) {
        std::istringstream iss(processed);
        return std::vector<std::string>{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
    };

    // Step 3: Clean up leading and trailing apostrophes
    auto cleanTokens = [](const std::vector<std::string>& tokens) {
        return std::accumulate(tokens.begin(), tokens.end(), std::vector<std::string>(), [](std::vector<std::string> acc, const std::string& token) {
            std::string cleaned = trimApostrophes(token);
            if (!cleaned.empty()) acc.push_back(cleaned);
            return acc;
        });
    };

    // Compose the steps
    return cleanTokens(splitIntoWords(preprocess(text)));
}

// Parallel tokenization
std::vector<std::string> parallelTokenize(const std::string& text) {
    // Helper function to adjust the midpoint
    auto adjustMid = [](size_t mid, const std::string& txt) {
        while (mid > 0 && std::isalnum(txt[mid])) --mid;
        return mid;
    };

    size_t mid = adjustMid(text.size() / 2, text);

    // Launch parallel tasks for tokenizing the text
    auto future1 = std::async(std::launch::async, tokenize, text.substr(0, mid));
    auto future2 = std::async(std::launch::async, tokenize, text.substr(mid));

    auto words1 = future1.get();
    auto words2 = future2.get();

    // Use std::accumulate to combine the vectors without requiring mutable references
    return std::accumulate(
        words2.begin(), words2.end(), std::move(words1),
        [](std::vector<std::string> acc, const std::string& word) {
            acc.push_back(word);
            return acc;
        }
    );
}

// Write sorted words to a file
void writeToFile(const std::string& filePath, const std::vector<std::string>& words) {
    try {
        // Open the file for writing
        std::ofstream file(filePath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::ios_base::failure("Failed to open file: " + filePath);
        }

        // Combine all words into a single string, separating them with newlines, and add a trailing newline
        const std::string content = std::accumulate(
            words.begin(), words.end(), std::string(),
            [](const std::string& acc, const std::string& word) {
                return acc + (acc.empty() ? "" : "\n") + word;
            }
        ) + "\n"; // Add a final newline for consistency

        // Write the combined string to the file
        file << content;

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
    // Define the input text for testing
    std::string text = "Hello, world! Functional-programming in C. Let's test this!";

    // Call the function to tokenize the input text in parallel
    auto words = parallelTokenize(text);

    // Define the expected output after tokenization
    std::vector<std::string> expected{"hello", "world", "functional", "programming", "in", "c", "let's", "test", "this"};

    // Compare the result using functional programming style
    bool match = std::equal(words.begin(), words.end(), expected.begin());

    // If the result doesn't match, print debugging information
    if (!match) {
        std::cerr << "\nParallel Tokenization Failed!\n"
                  << "Expected: " << std::accumulate(expected.begin(), expected.end(), std::string(),
                                                    [](const std::string& acc, const std::string& word) {
                                                        return acc + (acc.empty() ? "" : " ") + word;
                                                    })
                  << "\nActual:   " << std::accumulate(words.begin(), words.end(), std::string(),
                                                       [](const std::string& acc, const std::string& word) {
                                                           return acc + (acc.empty() ? "" : " ") + word;
                                                       })
                  << "\n";
    }

    // Use a functional approach for the assertion
    REQUIRE(match);
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

    // Use std::accumulate to merge the values from tree2 into tree1
    return std::accumulate(
        sortedValues.begin(), sortedValues.end(), tree1,
        [](const ImmutableRedBlackTree<T>& accumulatedTree, const T& value) {
            return accumulatedTree.insert(value);
        }
    );
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
        // Insert the first half of the words using std::accumulate
        return std::accumulate(
            words.begin(), words.begin() + mid, ImmutableRedBlackTree<T>(),
            [](const ImmutableRedBlackTree<T>& tree, const T& word) {
                return tree.insert(word);
            }
        );
    });

    auto futureTree2 = std::async(std::launch::async, [&]() {
        // Insert the second half of the words using std::accumulate
        return std::accumulate(
            words.begin() + mid, words.end(), ImmutableRedBlackTree<T>(),
            [](const ImmutableRedBlackTree<T>& tree, const T& word) {
                return tree.insert(word);
            }
        );
    });

    // Merge the results from the two futures
    return mergeTrees(futureTree1.get(), futureTree2.get());
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