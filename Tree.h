#ifndef TREE_H
#define TREE_H

#include "Node.h"

class Tree {
public:
    Node* root;

    Tree(Node* root = nullptr);
    ~Tree();

    void delete_subtree(Node* node);
    double evaluate(Node* node = nullptr) const;
};

#endif // TREE_H
