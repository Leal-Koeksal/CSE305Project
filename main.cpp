#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <limits>

class Node {
    std::string x;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;
public:
    Node (const std::string& x) : x(x) {}
    Node (const std::string& x, Node* left, Node* right) : x(x), left(left), right(right) {}

    std::string getString() {return x;}
    Node* getLeftChild() {return left;}
    Node* getRightChild() {return right;}
    Node* getParent() {return parent;}

    bool is_leaf() const {
        // Checks if node is a leaf
        if (left == nullptr && right == nullptr) return true;
        return false;
    }

    bool is_op() const {
        // Checks if node string is an operator
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
        if (node->getLeftChild() != nullptr) {
            delete(node->getLeftChild());
        }
        if (node->getRightChild() != nullptr) {
            delete(node->getRightChild());
        }
        if (node != nullptr) {
            delete node;
        }
    }

    ~Tree() {delete_subtree(root);}

    double evaluate(Node* node = nullptr) const {
        if (!node) node = root;
        if (node->is_leaf()) return std::stod(node->getString()); // std::stod converts string to double

        double left_x = evaluate(node->getLeftChild());
        double right_x = evaluate(node->getRightChild());

        if (node->getString() == "+") return left_x + right_x;
        if (node->getString() == "-") return left_x - right_x;
        if (node->getString() == "*") return left_x * right_x;
        if (node->getString() == "/") return right_x != 0 ? left_x / right_x : std::numeric_limits<double>::infinity();
    }
};

int main() {
    Node* left = new Node("2");
    Node* right = new Node("3");
    Node* root = new Node("+", left, right);
    Tree tree(root);
    std::cout << "Result: " << tree.evaluate(root) << std::endl;
    return 0;
}