#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <vector>
#include <future>
#include <numeric>
#include "redBlackTree.h"

// Function declarations
// Reads the content of a file and returns it as a single string
std::string readFile(const std::string& filePath);

// Trims leading and trailing apostrophes from a word
std::string trimApostrophes(const std::string& word);

// Tokenizes the input text into a vector of words, removing punctuation and converting to lowercase
std::vector<std::string> tokenize(const std::string& text);

// Performs parallel tokenization by splitting the input text into two parts
// and processing them in separate threads, then merging the results
std::vector<std::string> parallelTokenize(const std::string& text);

// Writes a vector of words to the specified file, with each word on a new line
void writeToFile(const std::string& filePath, const std::vector<std::string>& words);

// Processes a file by reading its content, tokenizing the text, inserting words into a tree,
// and writing the sorted output to a file. Supports parallel processing for optimization.
void processFileWithTiming(const std::string& inputPath, const std::string& outputPath, bool useParallel = false);

// Utility class to measure execution time for processes
class Timer {
public:
    // Constructor: Starts the timer when an object of Timer is created
    Timer();

    // Stops the timer and prints the duration with a custom process name
    void stop(const std::string& processName);

private:
    std::chrono::high_resolution_clock::time_point start_time; // Start time of the timer
};

// Parallel insertion of elements into a persistent Red-Black Tree
// Splits the input vector into two halves, inserts them in parallel, and merges the resulting trees
template <typename T>
RBTree<T> parallelInsert(const std::vector<T>& words) {
    if (words.empty()) {
        // If the input vector is empty, return an empty Red-Black Tree
        return RBTree<T>();
    }

    // Split the input vector into two halves
    size_t mid = words.size() / 2;

    // Asynchronously insert the first half into a tree
    auto futureTree1 = std::async(std::launch::async, [&]() {
        return std::accumulate(
            words.begin(), words.begin() + mid, RBTree<T>(),
            [](const RBTree<T>& tree, const T& word) {
                return tree.insert(word); // Insert each word into the tree
            }
        );
    });

    // Asynchronously insert the second half into a tree
    auto futureTree2 = std::async(std::launch::async, [&]() {
        return std::accumulate(
            words.begin() + mid, words.end(), RBTree<T>(),
            [](const RBTree<T>& tree, const T& word) {
                return tree.insert(word); // Insert each word into the tree
            }
        );
    });

    // Merge the two resulting trees into one
    return mergeTrees(futureTree1.get(), futureTree2.get());
}

// Merges two Red-Black Trees by inserting all values from the second tree into the first tree
template <typename T>
RBTree<T> mergeTrees(const RBTree<T>& tree1, const RBTree<T>& tree2) {
    // Retrieve the sorted values from the second tree
    auto sortedValues = tree2.getSortedValues();

    // Insert each value into the first tree to merge the two trees
    return std::accumulate(
        sortedValues.begin(), sortedValues.end(), tree1,
        [](const RBTree<T>& accumulatedTree, const T& value) {
            return accumulatedTree.insert(value); // Insert the value into the accumulated tree
        }
    );
}

#endif // HEADER_H