#ifndef TREE_H
#define TREE_H

#include "Node.h"

class Tree {
public:
    Node* root;

    Tree(Node* root = nullptr);
    ~Tree();

    Node* getRoot() const;
    void delete_subtree(Node* node);
    double evaluate(Node* node = nullptr) const;
};

#endif // TREE_H
