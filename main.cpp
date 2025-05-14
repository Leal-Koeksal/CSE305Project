#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <limits>

class Node {
public:
    std::string x;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;

    Node (const std::string& x) : x(x) {}

    bool is_leaf() const {
        if (left == nullptr && right == nullptr) return true;
        return false;
    }

    bool is_op() const {
        if (x == "+" || 
            x == "-" || 
            x == "*" || 
            x == "/") return true;
        return false; 
    }

};

class Tree {
public:
    Node* root;

    Tree(Node* root = nullptr) : root(root) {}

    void delete_subtree(Node* node) {
        if (!node) return;
        delete(node->left);
        delete(node->right);
        delete node;
    }

    ~Tree() {delete_subtree(root);}

    double evaluate(Node* node = nullptr) const {
        if (!node) node = root;
        if (node->is_leaf()) return std::stod(node->x); // std::stod converts string to double

        double left_x = evaluate(node->left);
        double right_x = evaluate(node->right);

        if (node->x == "+") return left_x + right_x;
        if (node->x == "-") return left_x - right_x;
        if (node->x == "*") return left_x * right_x;
        if (node->x == "/") return right_x != 0 ? left_x / right_x : std::numeric_limits<double>::infinity();
    }
};
