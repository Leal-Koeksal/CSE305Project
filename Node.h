
#ifndef NODE_H
#define NODE_H

#include <string>
#include <regex>

class Node {
    std::string x;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;

    // Meta data (for randomised tree evaluation)
    bool deleted = false;
    bool marked = false;
    double eval = 0.0;
    bool is_value_set = false;

public:
    Node(const std::string& x);
    Node(const std::string& x, Node* left, Node* right);

    std::string getString();
    Node* getLeftChild();
    Node* getRightChild();
    Node* getParent();

    bool isDeleted() const;
    void markDeleted();

    void setEval(double val);
    double getEval() const;
    bool hasValue() const;

    bool is_leaf() const;
    bool is_op() const;

    // helper functions for tree contraction
    bool is_function();
    // bool is_unary_chain_node() const; // true is node has exactly one child
    
    // for example tree I am making
    void setLeftChild(Node*);
    void setRightChild(Node*);
    void setParent(Node*);
    void setString(const std::string& val);

    // linear function: a*x + b -> a,b
    bool eval_function(double); // function is the node

};

#endif // NODE_H
