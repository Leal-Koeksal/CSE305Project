#include "Tree.h"
#include <limits>
#include <cstdlib>
#include <iostream>

double evaluate_parallel(Node* node);

Tree::Tree(Node* root) : root(root) {}

Tree::~Tree() {
    delete_subtree(root);
}

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

    std::string op = node->getString();
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") return right != 0 ? left / right : std::numeric_limits<double>::infinity();

    return 0;
}

Node* Tree::get_root() {return root;}

// doesnt work generally, only for the rake step
// called on the leaf -> delete leaf, parent and sibling, leaf->parent = replacement
// leaf is always left child
void Tree::replace(Node* leaf, Node* replacement) {
    Node* parent = leaf->getParent();
    Node* sibling = parent->getRightChild();

    if (!parent || parent->isDeleted()) {
        root = replacement;
        replacement->setParent(nullptr);
        leaf->markDeleted();
        sibling->markDeleted();
        return;
    }

    Node* x = parent->getParent();

    if (x) {
        if (x->getLeftChild() == parent) x->setLeftChild(replacement);
        else if (x->getRightChild() == parent) x->setRightChild(replacement);
        replacement->setParent(x);
    }
    else {
        root = replacement;
        replacement->setParent(nullptr);
    }

    leaf->markDeleted();
    sibling->markDeleted();
    parent->markDeleted();
}
