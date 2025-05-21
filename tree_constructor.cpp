#include <random>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
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
    std::uniform_int_distribution<> dist(1, 100); // Avoid zero for division (easier)
    return std::to_string(dist(rng));
}

Tree tree_constructor(int n) {
    if (n < 1) throw std::invalid_argument("Tree must have at least one operator");
    
    std::random_device rd;
    std::mt19937 gen(rd());

    // Step 1: Create n+1 leaf nodes (operands)
    std::vector<Node*> nodes;
    for (int i = 0; i < n + 1; ++i) {
        std::string value = get_random_number();
        nodes.push_back(new Node(value));
    }

    // Step 2: Randomly combine nodes into binary operator nodes
    while (nodes.size() > 1) {
        // Pick two random nodes to combine
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

    // Final node is the root
    Node* root = nodes.front();
    return Tree(root);
}