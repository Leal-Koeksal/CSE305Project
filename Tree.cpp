#include "Tree.h"
#include <limits>
#include <cstdlib>
#include <iostream>
#include <cmath>

constexpr int LARGE_PRIME = 6101;
// double evaluate_parallel(Node* node);

Tree::Tree(Node* root) : root(root) {}

Tree::~Tree() {
    delete_subtree(root);
}

Node* Tree::getRoot() const { return root; }

void Tree::delete_subtree(Node* node) {
    if (!node) return;
    if (node->getLeftChild()) delete_subtree(node->getLeftChild());
    if (node->getRightChild()) delete_subtree(node->getRightChild());
    delete node;
}


double Tree::evaluate(Node* node) const {
    if (!node) node = root;

    if (node->is_leaf() && node->is_op()) {
        throw std::runtime_error("Invalid tree: a leaf node cannot be an operator.");
    }

    if (node->is_leaf()) return std::stod(node->getString());

    double left = evaluate(node->getLeftChild());
    double right = evaluate(node->getRightChild());

    int l = static_cast<int>(left);
    int r = static_cast<int>(right);

    std::string op = node->getString();
  
    if (op == "+") return std::fmod(l + r, static_cast<double>(LARGE_PRIME));
    if (op == "-") return std::fmod(l - r, static_cast<double>(LARGE_PRIME));
    if (op == "*") return std::fmod(l * r, static_cast<double>(LARGE_PRIME));
    if (op == "/") return r != 0 ? std::fmod(l / r, static_cast<double>(LARGE_PRIME)) : std::numeric_limits<double>::infinity();

    return 0;
}