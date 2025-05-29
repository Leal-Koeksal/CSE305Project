#include <omp.h>
#include <vector>
#include <random>
#include "Tree.h"

void collect_nodes(Node* node, std::vector<Node*>& nodes) {
    if (!node) return;
    nodes.push_back(node);
    collect_nodes(node->getLeftChild(), nodes);
    collect_nodes(node->getRightChild(), nodes);
}

double randomised_evaluate_parallel(Node* root) {
    if (!root) return 0.0;

    std::vector<Node*> active_nodes;
    collect_nodes(root, active_nodes);

    bool done = false;
    while (!done) {
        bool progress = false;

        #pragma omp parallel for
        for (int i = 0; i < active_nodes.size(); ++i) {
            Node* node = active_nodes[i];
            if (node->isDeleted() || node->hasValue()) continue;

            if (node->is_leaf() && !node->is_op()) {
                node->setEval(std::stod(node->getString()));
                node->markDeleted();
                #pragma omp critical
                progress = true;
            }

            if (node->getLeftChild() && node->getRightChild()) {
                Node* left = node->getLeftChild();
                Node* right = node->getRightChild();
                if (left->hasValue() && right->hasValue()) {
                    std::string op = node->getString();
                    double val = 0;
                    if (op == "+") val = left->getEval() + right->getEval();
                    else if (op == "-") val = left->getEval() - right->getEval();
                    else if (op == "*") val = left->getEval() * right->getEval();
                    else if (op == "/") val = right->getEval() != 0 ? left->getEval() / right->getEval()
                                                                    : std::numeric_limits<double>::infinity();
                    node->setEval(val);
                    node->markDeleted();
                    #pragma omp critical
                    progress = true;
                }
            }
        }

        std::vector<Node*> remaining;
        for (Node* node : active_nodes) {
            if (!node->isDeleted()) remaining.push_back(node);
        }

        if (remaining.size() == 1 && remaining[0] == root && root->hasValue()) {
            done = true;
        } else if (!progress) {
            // break to avoid infinite loop
            done = true;
        }

        active_nodes.swap(remaining);
    }

    return root->getEval();
}

// clang++ -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/include -L/opt/homebrew/lib -lomp main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp randomised.cpp -std=c++17 -pthread -o main