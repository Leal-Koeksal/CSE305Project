
#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

enum class Sex { UNASSIGNED, M, F };

class Node {
    std::string x;
    // std::vector<Node*> children = std::vector<Node*>();;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;

    // Meta data (for randomised tree evaluation)
    bool deleted = false;
    bool marked = false;
    double eval = 0.0;
    bool is_value_set = false;
    Sex sex = Sex::UNASSIGNED;
public:
    Node(const std::string& x);
    Node(const std::string& x, Node* left, Node* right);
    ~Node() = default;

    std::string getString();
    Node* getLeftChild();
    Node* getRightChild();
    Node* getParent();
    void setParent(Node* parent);
    Sex getSex() const { return sex; }
    void setSex(Sex s) { sex = s; }

    bool isMarked() const { return marked; }
    void mark() { marked = true; }
    void unmark() { marked = false; }

    bool isDeleted() const;
    void markDeleted();

    void setEval(double val);
    double getEval() const;
    bool hasValue() const;

    bool is_leaf() const;
    bool is_op() const;
};

#endif // NODE_H