#include <random>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <stack>
#include "Tree.h"

// This code was inspired from stack overflow:
// https://stackoverflow.com/questions/44576857/randomly-pick-from-a-vector-in-c
std::string get_random_operator() {
    static const std::vector<std::string> ops = {"+", "-", "*", "/"};
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, ops.size() - 1);
    return ops[dist(rng)];
}

std::string get_random_number() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<> dist(0.1, 1); // Avoid zero for division (easier)
    return std::to_string(dist(rng));
}

Tree tree_constructor(int n) {
    if (n < 1) throw std::invalid_argument("Tree must have at least one operator");
    
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<Node*> nodes;
    for (int i = 0; i < n + 1; ++i) {
        std::string value = get_random_number();
        nodes.push_back(new Node(value));
    }

    while (nodes.size() > 1) {
        int i = std::uniform_int_distribution<>(0, nodes.size() - 1)(gen);
        Node* left = nodes[i];
        nodes.erase(nodes.begin() + i);

        int j = std::uniform_int_distribution<>(0, nodes.size() - 1)(gen);
        Node* right = nodes[j];
        nodes.erase(nodes.begin() + j);

        std::string op = get_random_operator();
        Node* parent = new Node(op, left, right);

        nodes.push_back(parent);
    }

    Node* root = nodes.front();
    return Tree(root);
}

std::vector<Node*> list_nodes(Tree& tree) {
    Node* root = tree.getRoot();
    std::vector<Node*> result;

    if (!root) return result;

    // Use a stack for iterative traversal (or you could use recursion)
    std::stack<Node*> stack;
    stack.push(root);

    while (!stack.empty()) {
        Node* current = stack.top();
        stack.pop();

        result.push_back(current);

        // Push right first so left is processed first
        if (current->getRightChild()) {
            stack.push(current->getRightChild());
        }

        if (current->getLeftChild()) {
            stack.push(current->getLeftChild());
        }
    }

    return result;
}
