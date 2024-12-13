#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include <vector>
#include <cassert>

// Colors for Red-Black Tree
enum class Color { R, B };

// Persistent Red-Black Tree
template <typename T>
class RBTree {
    struct Node {
        Node(Color c,
             std::shared_ptr<const Node> const& lft,
             T val,
             std::shared_ptr<const Node> const& rgt)
            : _c(c), _lft(lft), _val(val), _rgt(rgt)
        {}

        Color _c;
        std::shared_ptr<const Node> _lft;
        T _val;
        std::shared_ptr<const Node> _rgt;
    };

    explicit RBTree(std::shared_ptr<const Node> const& node)
        : _root(node) {}

public:
    RBTree() : _root(nullptr) {}

    RBTree(Color c, RBTree const& lft, T val, RBTree const& rgt)
        : _root(std::make_shared<const Node>(c, lft._root, val, rgt._root)) {}

    bool isEmpty() const { return !_root; }

    T root() const {
        assert(!isEmpty());
        return _root->_val;
    }

    Color rootColor() const {
        assert(!isEmpty());
        return _root->_c;
    }

    RBTree left() const {
        assert(!isEmpty());
        return RBTree(_root->_lft);
    }

    RBTree right() const {
        assert(!isEmpty());
        return RBTree(_root->_rgt);
    }

    RBTree insert(T x) const {
        RBTree t = ins(x);
        return RBTree(Color::B, t.left(), t.root(), t.right());
    }

    std::vector<T> getSortedValues() const {
        std::vector<T> result;
        getSortedValuesHelper(_root, result);
        return result;
    }

private:
    std::shared_ptr<const Node> _root;

    RBTree ins(T x) const {
        if (isEmpty())
            return RBTree(Color::R, RBTree(), x, RBTree());
        T y = root();
        Color c = rootColor();
        if (x < y)
            return balance(c, left().ins(x), y, right());
        else if (y < x)
            return balance(c, left(), y, right().ins(x));
        else
            return *this; // no duplicates
    }

    static RBTree balance(Color c, RBTree const& lft, T x, RBTree const& rgt) {
        if (c == Color::B && lft.doubledLeft())
            return RBTree(Color::R,
                          lft.left().paint(Color::B),
                          lft.root(),
                          RBTree(Color::B, lft.right(), x, rgt));
        else if (c == Color::B && lft.doubledRight())
            return RBTree(Color::R,
                          RBTree(Color::B, lft.left(), lft.root(), lft.right().left()),
                          lft.right().root(),
                          RBTree(Color::B, lft.right().right(), x, rgt));
        else if (c == Color::B && rgt.doubledLeft())
            return RBTree(Color::R,
                          RBTree(Color::B, lft, x, rgt.left().left()),
                          rgt.left().root(),
                          RBTree(Color::B, rgt.left().right(), rgt.root(), rgt.right()));
        else if (c == Color::B && rgt.doubledRight())
            return RBTree(Color::R,
                          RBTree(Color::B, lft, x, rgt.left()),
                          rgt.root(),
                          rgt.right().paint(Color::B));
        else
            return RBTree(c, lft, x, rgt);
    }

    bool doubledLeft() const {
        return !isEmpty() && rootColor() == Color::R && !left().isEmpty() && left().rootColor() == Color::R;
    }

    bool doubledRight() const {
        return !isEmpty() && rootColor() == Color::R && !right().isEmpty() && right().rootColor() == Color::R;
    }

    RBTree paint(Color c) const {
        assert(!isEmpty());
        return RBTree(c, left(), root(), right());
    }

    static void getSortedValuesHelper(std::shared_ptr<const Node> const& node, std::vector<T>& result) {
        if (!node) return;
        getSortedValuesHelper(node->_lft, result);
        result.push_back(node->_val);
        getSortedValuesHelper(node->_rgt, result);
    }
};

#endif // RED_BLACK_TREE_H