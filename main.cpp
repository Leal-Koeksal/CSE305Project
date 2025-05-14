#include <iostream>
#include "Tree.h"

int main() {
    Node* leftleft = new Node("10");
    Node* leftright = new Node("8");
    Node* left = new Node("*", leftleft, leftright);
    Node* right = new Node("3");
    Node* root = new Node("+", left, right);
    Tree tree(root);

    std::cout << "Result: " << tree.evaluate() << std::endl;

    return 0;
}