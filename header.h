#ifndef HEADER_H
#define HEADER_H
#include <string>
#include <vector>
#include <future>
#include <functional>
#include <numeric>

// Immutable Red-Black Tree Declaration
template <typename T>
struct ImmutableRedBlackTree {
    struct Node {
        T value;
        bool color; // true = Red, false = Black
        std::shared_ptr<Node> left, right;

        Node(const T& val, bool color, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr)
            : value(val), color(color), left(std::move(left)), right(std::move(right)) {}
    };

    std::shared_ptr<Node> root;

    ImmutableRedBlackTree() : root(nullptr) {}
    ImmutableRedBlackTree(std::shared_ptr<Node> root) : root(std::move(root)) {}

    ImmutableRedBlackTree insert(const T& value) const {
        auto newRoot = insert(root, value);
        return ImmutableRedBlackTree(std::make_shared<Node>(newRoot->value, false, newRoot->left, newRoot->right));
    }

    std::vector<T> getSortedValues() const {
        if (!root) return {};
        std::vector<T> result;
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

private:
    std::shared_ptr<Node> balance(std::shared_ptr<Node> node) const {
        if (!node) return nullptr;

        if (node->left && node->left->color && node->left->left && node->left->left->color) {
            return std::make_shared<Node>(node->left->value, true, node->left->left,
                                          std::make_shared<Node>(node->value, false, node->left->right, node->right));
        }
        if (node->left && node->left->color && node->left->right && node->left->right->color) {
            return std::make_shared<Node>(node->left->right->value, true,
                                          std::make_shared<Node>(node->left->value, false, node->left->left, node->left->right->left),
                                          std::make_shared<Node>(node->value, false, node->left->right->right, node->right));
        }
        if (node->right && node->right->color && node->right->right && node->right->right->color) {
            return std::make_shared<Node>(node->right->value, true,
                                          std::make_shared<Node>(node->value, false, node->left, node->right->left),
                                          node->right->right);
        }
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
};

// Function declarations
std::string readFile(const std::string& filePath);
std::string trimApostrophes(const std::string& word);
std::vector<std::string> tokenize(const std::string& text);
std::vector<std::string> parallelTokenize(const std::string& text);
void writeToFile(const std::string& filePath, const std::vector<std::string>& words);

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

void processFile(const std::string& inputPath, const std::string& outputPath);

#endif // HEADER_H