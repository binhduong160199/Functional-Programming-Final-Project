#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <vector>
#include <future>
#include <numeric>
#include "redBlackTree.h"

// Function declarations
std::string readFile(const std::string& filePath);
std::string trimApostrophes(const std::string& word);
std::vector<std::string> tokenize(const std::string& text);
std::vector<std::string> parallelTokenize(const std::string& text);
void writeToFile(const std::string& filePath, const std::vector<std::string>& words);
void processFileWithTiming(const std::string& inputPath, const std::string& outputPath, bool useParallel = false);

// Utility to measure time
class Timer {
public:
    Timer();
    void stop(const std::string& processName);

private:
    std::chrono::high_resolution_clock::time_point start_time;
};

// parallel insert and merge
template <typename T>
RBTree<T> parallelInsert(const std::vector<T>& words) {
    if (words.empty()) {
        return RBTree<T>();
    }

    size_t mid = words.size() / 2;
    auto futureTree1 = std::async(std::launch::async, [&]() {
        return std::accumulate(
            words.begin(), words.begin() + mid, RBTree<T>(),
            [](const RBTree<T>& tree, const T& word) {
                return tree.insert(word);
            }
        );
    });

    auto futureTree2 = std::async(std::launch::async, [&]() {
        return std::accumulate(
            words.begin() + mid, words.end(), RBTree<T>(),
            [](const RBTree<T>& tree, const T& word) {
                return tree.insert(word);
            }
        );
    });

    return mergeTrees(futureTree1.get(), futureTree2.get());
}

template <typename T>
RBTree<T> mergeTrees(const RBTree<T>& tree1, const RBTree<T>& tree2) {
    auto sortedValues = tree2.getSortedValues();
    return std::accumulate(
        sortedValues.begin(), sortedValues.end(), tree1,
        [](const RBTree<T>& accumulatedTree, const T& value) {
            return accumulatedTree.insert(value);
        }
    );
}

#endif // HEADER_H