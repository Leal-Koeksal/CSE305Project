#include "Node.h"

Node::Node(const std::string& x) : x(x) {}

Node::Node(const std::string& x, Node* left, Node* right) : x(x), left(left), right(right) {}

std::string Node::getString() { return x; }
Node* Node::getLeftChild() { return left; }
Node* Node::getRightChild() { return right; }
Node* Node::getParent() { return parent; }
void Node::setParent(Node* prt) {
    parent = prt;
}

bool Node::isDeleted() const { return deleted; }
void Node::markDeleted() { deleted = true; }

void Node::setEval(double val) {
    eval = val;
    is_value_set = true;
}
double Node::getEval() const { return eval; }
bool Node::hasValue() const { return is_value_set; }


bool Node::is_leaf() const { return left == nullptr && right == nullptr; }

bool Node::is_op() const {
    return x == "+" || x == "-" || x == "*" || x == "/";
}