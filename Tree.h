#ifndef TREE_H
#define TREE_H

#include "Node.h"

#include <vector>

class Tree {
public:
    Node* root;

    Tree(Node* root = nullptr);
    ~Tree();

    void delete_subtree(Node* node);
    double evaluate(Node* node = nullptr) const;
    Node* get_root();

    // helper function for rake
    void replace(Node*, Node*); // replace parent of node with replacement and delete node and sibling

    std::vector<Node*> find_max_chain(); // gives list of longest chains (only top nodes)
};

#endif // TREE_H
