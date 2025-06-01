
#include "Node.h"

Node::Node(const std::string& x) : x(x) {}

Node::Node(const std::string& x, Node* left, Node* right) : x(x), left(left), right(right) {}

std::string Node::getString() { return x; }
Node* Node::getLeftChild() { return left; }
Node* Node::getRightChild() { return right; }
Node* Node::getParent() { return parent; }

bool Node::isDeleted() const { return deleted; }
void Node::markDeleted() {
    deleted = true;
    left = nullptr;
    right = nullptr;
} 

void Node::setEval(double val) {
    eval = val;
    is_value_set = true;
}
double Node::getEval() const { return eval; }
bool Node::hasValue() const { return is_value_set; }


bool Node::is_leaf() const {return (!left || left->isDeleted()) && (!right || right->isDeleted());}

bool Node::is_op() const {
    return x == "+" || x == "-" || x == "*" || x == "/";
}


// for tree contraction
// bool Node::is_unary_chain_node() const {
//     return !isDeleted() && ((left && !right) || (!left && right));
// }

// for test tree
void Node::setLeftChild(Node* child) {
    left = child;
    if (child) {
        child->setParent(this);
    }
}

void Node::setRightChild(Node* child) {
    right = child;
    if (child) {
        child->setParent(this);
    }
}

void Node::setParent(Node* p) {
    parent = p;
}

void Node::setString(const std::string& val) {
    x = val;
}

bool Node::eval_function(double x) {
    // Matches strings like "a,b"
    static std::regex func_pattern(R"(^(-?\d*\.?\d+),(-?\d*\.?\d+)$)");
    std::smatch matches;

    std::string s = this->getString();
    if (std::regex_match(s, matches, func_pattern)) {
        double a = std::stod(matches[1]);
        double b = std::stod(matches[2]);
        double val = a * x + b;

        this->setString(std::to_string(val));
        this->setEval(val);

        // Delete children if any
        if (this->getLeftChild()) {
            this->getLeftChild()->markDeleted();
            this->setLeftChild(nullptr);
        }
        if (this->getRightChild()) {
            this->getRightChild()->markDeleted();
            this->setRightChild(nullptr);
        }

        return true; 
    }

    return false; 
}

bool Node::is_function() {
    static std::regex func_pattern(R"(^(-?\d*\.?\d+),(-?\d*\.?\d+)$)");
    std::smatch matches;
    std::string str = this->getString(); // store in a local lvalue
    return std::regex_match(str, matches, func_pattern);
}

