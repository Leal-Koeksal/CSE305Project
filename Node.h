#ifndef NODE_H
#define NODE_H

#include <string>

class Node {
    std::string x;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;
public:
    Node(const std::string& x);
    Node(const std::string& x, Node* left, Node* right);

    std::string getString();
    Node* getLeftChild();
    Node* getRightChild();
    Node* getParent();

    bool is_leaf() const;
    bool is_op() const;
};

#endif // NODE_H