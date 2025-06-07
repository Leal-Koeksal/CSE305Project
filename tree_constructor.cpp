#include <random>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <stack>
#include <stdexcept>
#include <functional>
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
    std::uniform_real_distribution<> dist(0.1, 1000); // Avoid zero for division (easier)
    return std::to_string(dist(rng));
}

Tree full_tree_constructor(int n) {
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

Tree random_tree_constructor(int n) {
    if (n < 1) {
        throw std::invalid_argument("Tree must have height at least 1");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist01(0.0, 1.0);
    std::function<Node*(int)> build = [&](int depth) -> Node* {
        if (depth == n) {
            return new Node(get_random_number());
        }
        double p = 0.8 * (1.0 - double(depth - 1) / double(n - 1));
        if (dist01(gen) < p) {
            Node* left  = build(depth + 1);
            Node* right = build(depth + 1);
            std::string op = get_random_operator();
            return new Node(op, left, right);
        } else {
            return new Node(get_random_number());
        }
    };

    Node* root = build(1);
    return Tree(root);
}

Tree most_unbalanced_tree_constructor(int height) {
    if (height < 1) throw std::invalid_argument("Tree must have at least height 1");
    std::random_device rd;
    std::mt19937 gen(rd());

    if (height == 1) {
        Node* leaf = new Node(get_random_number());
        return Tree(leaf);
    }
    Node* curr = new Node(get_random_number()); 

    for (int level = 2; level <= height; ++level) {
        Node* rightLeaf = new Node(get_random_number());
        std::string op = get_random_operator();
        Node* parent = new Node(op, curr, rightLeaf);
        curr = parent;
    }

    return Tree(curr);
}

std::vector<Node*> list_nodes(Tree& tree) {
    Node* root = tree.getRoot();
    std::vector<Node*> result;

    if (!root) return result;

    // Use a stack for iterative traversal
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


