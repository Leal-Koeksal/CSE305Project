#include "Node.h"

Node::Node(const std::string& x) : x(x) {}

Node::Node(const std::string& x, Node* left, Node* right) : x(x), left(left), right(right) {}

std::string Node::getString() { return x; }
Node* Node::getLeftChild() { return left; }
Node* Node::getRightChild() { return right; }
Node* Node::getParent() { return parent; }

bool Node::is_leaf() const { return left == nullptr && right == nullptr; }

bool Node::is_op() const {
    return x == "+" || x == "-" || x == "*" || x == "/";
}
