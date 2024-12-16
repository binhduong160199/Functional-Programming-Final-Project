#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include <vector>
#include <cassert>
#include <memory> // For std::shared_ptr

// Colors for Red-Black Tree
enum class Color { R, B }; // Enum to define node colors: R = Red, B = Black

// Persistent Red-Black Tree
template<typename T>
class RBTree {
    // Node structure representing a single element in the tree
    struct Node {
        Node(Color c,
             std::shared_ptr<const Node> const &lft, // Left child
             T val, // Value stored in the node
             std::shared_ptr<const Node> const &rgt) // Right child
            : _c(c), _lft(lft), _val(val), _rgt(rgt) {
        }

        Color _c; // Color of the node
        std::shared_ptr<const Node> _lft; // Pointer to the left child
        T _val; // Value stored in this node
        std::shared_ptr<const Node> _rgt; // Pointer to the right child
    };

    // Constructor to initialize an RBTree with a given root node
    explicit RBTree(std::shared_ptr<const Node> const &node)
        : _root(node) {
    }

public:
    // Default constructor to create an empty tree
    RBTree() : _root(nullptr) {
    }

    // Constructor to create a tree with a root node
    RBTree(Color c, RBTree const &lft, T val, RBTree const &rgt)
        : _root(std::make_shared<const Node>(c, lft._root, val, rgt._root)) {
    }

    // Check if the tree is empty
    bool isEmpty() const { return !_root; }

    // Get the value of the root node
    T root() const {
        assert(!isEmpty()); // Ensure the tree is not empty
        return _root->_val;
    }

    // Get the color of the root node
    Color rootColor() const {
        assert(!isEmpty()); // Ensure the tree is not empty
        return _root->_c;
    }

    // Get the left subtree
    RBTree left() const {
        assert(!isEmpty()); // Ensure the tree is not empty
        return RBTree(_root->_lft);
    }

    // Get the right subtree
    RBTree right() const {
        assert(!isEmpty()); // Ensure the tree is not empty
        return RBTree(_root->_rgt);
    }

    // Insert a value into the tree and return the updated tree
    RBTree insert(T x) const {
        RBTree t = ins(x); // Perform insertion
        // Ensure the root of the tree is always black
        return RBTree(Color::B, t.left(), t.root(), t.right());
    }

    // Retrieve all values in the tree in sorted order
    std::vector<T> getSortedValues() const {
        std::vector<T> result;
        getSortedValuesHelper(_root, result); // Perform an in-order traversal
        return result;
    }

private:
    std::shared_ptr<const Node> _root; // Root node of the tree

    // Helper function to recursively insert a value into the tree
    RBTree ins(T x) const {
        if (isEmpty()) // Base case: Create a new red node for the value
            return RBTree(Color::R, RBTree(), x, RBTree());

        T y = root(); // Current root value
        Color c = rootColor(); // Current root color

        // Recursively insert into the left or right subtree
        if (x < y)
            return balance(c, left().ins(x), y, right()); // Insert into the left subtree
        else if (y < x)
            return balance(c, left(), y, right().ins(x)); // Insert into the right subtree
        else
            return *this; // Value already exists, no duplicates allowed
    }

    // Helper function to balance the tree to maintain Red-Black Tree properties
    static RBTree balance(Color c, RBTree const &lft, T x, RBTree const &rgt) {
        // Case 1: Check and resolve violations for doubled left red nodes
        // This occurs when the left child and its left child are both red.
        // Fix: Rotate right and recolor the nodes to restore balance.
        if (c == Color::B && lft.doubledLeft()) {
            return RBTree(
                Color::R, // Recolor the new root as red
                lft.left().paint(Color::B), // Recolor the left child's left subtree as black
                lft.root(), // Promote the left child to root
                RBTree(Color::B, lft.right(), x, rgt) // Balance the right subtree
            );
        }
        // Case 2: Check and resolve violations for left-right red nodes (zig-zag pattern)
        // This occurs when the left child is red and its right child is also red.
        // Fix: Rotate the left child leftward, reducing the problem to Case 1.
        else if (c == Color::B && lft.doubledRight()) {
            return RBTree(
                Color::R, // Recolor the new root as red
                RBTree(Color::B, lft.left(), lft.root(), lft.right().left()), // Rotate left child leftward
                lft.right().root(), // Promote the left child's right child as new root
                RBTree(Color::B, lft.right().right(), x, rgt) // Balance the right subtree
            );
        }
        // Case 3: Check and resolve violations for right-left red nodes (zig-zag pattern)
        // This occurs when the right child is red and its left child is also red.
        // Fix: Rotate the right child rightward, reducing the problem to Case 4.
        else if (c == Color::B && rgt.doubledLeft()) {
            return RBTree(
                Color::R, // Recolor the new root as red
                RBTree(Color::B, lft, x, rgt.left().left()), // Balance the left subtree
                rgt.left().root(), // Promote the right child's left child as new root
                RBTree(Color::B, rgt.left().right(), rgt.root(), rgt.right()) // Rotate right subtree
            );
        }
        // Case 4: Check and resolve violations for doubled right red nodes
        // This occurs when the right child and its right child are both red.
        // Fix: Rotate left and recolor the nodes to restore balance.
        else if (c == Color::B && rgt.doubledRight()) {
            return RBTree(
                Color::R, // Recolor the new root as red
                RBTree(Color::B, lft, x, rgt.left()), // Balance the left subtree
                rgt.root(), // Promote the right child to root
                rgt.right().paint(Color::B) // Recolor the right child's right subtree as black
            );
        }
        // Default Case: If no violations are found, return the tree as-is
        else {
            return RBTree(c, lft, x, rgt); // No balancing is needed
        }
    }

    // Check if the left subtree has doubled red nodes
    bool doubledLeft() const {
        return !isEmpty() && rootColor() == Color::R && !left().isEmpty() && left().rootColor() == Color::R;
    }

    // Check if the right subtree has doubled red nodes
    bool doubledRight() const {
        return !isEmpty() && rootColor() == Color::R && !right().isEmpty() && right().rootColor() == Color::R;
    }

    // Paint the root of the tree with a new color
    RBTree paint(Color c) const {
        assert(!isEmpty()); // Ensure the tree is not empty
        return RBTree(c, left(), root(), right());
    }

    // Recursive helper function for in-order traversal to collect sorted values
    static void getSortedValuesHelper(std::shared_ptr<const Node> const &node, std::vector<T> &result) {
        if (!node) return; // Base case: empty node
        getSortedValuesHelper(node->_lft, result); // Traverse left subtree
        result.push_back(node->_val); // Add root value
        getSortedValuesHelper(node->_rgt, result); // Traverse right subtree
    }
};

#endif // RED_BLACK_TREE_H
