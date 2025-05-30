/*
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
                node->setEval(std::stod(node->getString())); // string to double
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
*/

#include <omp.h>
#include <vector>
#include <random>
#include <unordered_map>
#include <string>
#include <stack>
#include "Tree.h"

int Arg(Node* v) {
    int arg = 0;
    if (v->getLeftChild() && !v->getLeftChild()->hasValue()) ++arg;
    if (v->getRightChild() && !v->getRightChild()->hasValue()) ++arg;
    return arg;
}


void randomized_contract(std::vector<Node*>& nodes, Node* root) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> arg_dist(0, 1);
    std::uniform_int_distribution<int> sex_dist(0, 1);

    // Sex assignment map: node ptr -> 'M' or 'F'
    std::unordered_map<Node*, char> sex_map;

    // Parallel phase
    #pragma omp parallel for
    for (int i = 0; i < nodes.size(); ++i) {
        Node* v = nodes[i];

        if (v == root || v->isDeleted()) continue;

        int arg = Arg(v);  // Arg(v)

        if (arg == 0) {
            // 1) If Arg(v) = 0 then mark P(v) and delete v
            Node* parent = v->getParent();
            if (parent) {
                // Optionally "mark" parent
                #pragma omp critical
                parent->markDeleted(); // Use mark or custom flag if needed
            }
            v->markDeleted();
        }
        else {
            // 2) Arg(v) = 1 → Randomly assign M or F
            char sex = sex_dist(gen) == 0 ? 'M' : 'F';
            sex_map[v] = sex;

            Node* parent = v->getParent();
            if (parent) {
                char parent_sex = sex_map[parent];

                // 3) If Sex(v) = F and Sex(P(v)) = M
                if (sex == 'F' && parent_sex == 'M') {
                    // a) Push on Store_v value P(v)
                    std::stack<std::string> store_v;
                    store_v.push(parent->getString());

                    // b) P(v) ← P(P(v))
                    Node* grandparent = parent->getParent();
                    if (grandparent) {
                        parent->setParent(grandparent);
                    }

                    // c) delete vertex(P(v))
                    parent->markDeleted();
                }
            }
        }
    }
}


// clang++ -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/include -L/opt/homebrew/lib -lomp main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp randomised.cpp -std=c++17 -pthread -o main