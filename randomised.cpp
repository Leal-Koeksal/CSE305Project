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

int count_active_nodes(const std::vector<Node*>& nodes) {
    int count = 0;
    for (Node* node : nodes) {
        if (node && !node->isDeleted()) {
            ++count;
        }
    }
    return count;
}

void dynamic_tree_contraction(std::vector<Node*>& nodes, Node* root) {
    #pragma omp parallel for
    for (int i = 0; i < (int)nodes.size(); ++i) {
        Node* v = nodes[i];
        if (!v || v->isDeleted()) continue;
        int num_children = 0;
        if (v->getLeftChild() && !v->getLeftChild()->isDeleted()) ++num_children;
        if (v->getRightChild() && !v->getRightChild()->isDeleted()) ++num_children;

        Node* parent = v->getParent();
        if (!parent || parent->isDeleted()) continue;
        int num_siblings = 0;
        if (parent->getLeftChild() && !parent->getLeftChild()->isDeleted()) ++num_siblings;
        if (parent->getRightChild() && !parent->getRightChild()->isDeleted()) ++num_siblings;

        if (num_children == 0) {
            Node* parent = v->getParent();
                if (parent) parent->mark();

                // Delete if more than 1 node remains
                if (count_active_nodes(nodes) > 1) {
                    v->markDeleted();
                }
        }
        else if (num_children == 1 && num_siblings == 1) {
            Node* grandparent = parent->getParent();
            if (!grandparent || grandparent->isDeleted()) continue;
            if (grandparent->getLeftChild() == parent) {
                grandparent->setLeftChild(v);
            } else if (grandparent->getRightChild() == parent) {
                grandparent->setRightChild(v);
            }

            v->setParent(grandparent);
        }
    }
}

void randomized_contract(std::vector<Node*>& nodes, Node* root) {
    #pragma omp parallel for
    for (int i = 0; i < (int)nodes.size(); ++i) {
        Node* v = nodes[i];
        if (!v || v->isDeleted()) continue;

        int num_children = 0;
        if (v->getLeftChild() && !v->getLeftChild()->isDeleted()) ++num_children;
        if (v->getRightChild() && !v->getRightChild()->isDeleted()) ++num_children;

        if (num_children == 0) {
            Node* parent = v->getParent();
            if (parent) parent->mark();

            // Delete if more than 1 node remains
            if (count_active_nodes(nodes) > 1) {
                v->markDeleted();
            }
        }
        else if (num_children == 1) {
            v->setSex((rand() % 2 == 0) ? Sex::F : Sex::M);

            Node* parent = v->getParent();
            if (!parent || parent->isDeleted()) continue;

            if (parent->getSex() == Sex::M && v->getSex() == Sex::F) {
                Node* grandparent = parent->getParent();
                if (!grandparent || grandparent->isDeleted()) continue;


                //Store 
                // push_back but useless it seems 
                // Update grandparent to point to v instead of parent
                if (grandparent->getLeftChild() == parent) {
                    grandparent->setLeftChild(v);
                } else if (grandparent->getRightChild() == parent) {
                    grandparent->setRightChild(v);
                }

                v->setParent(grandparent);

                // Delete if more than 1 node remains
                if (count_active_nodes(nodes) > 1) {
                    parent->markDeleted();
                }
            }
        }
    }

    #pragma omp barrier
}

void randomized_tree_evaluation(std::vector<Node*>& nodes, Node* root) {
    int n = nodes.size();
    int p = n * std::log(std::log(n)) / std::log(n);
    int k = 1;
    const int c = 2;  // Or some small constant
    
    while (k <= c * std::log(std::log(n))) {
        dynamic_tree_contraction(nodes, root);  // using p processors

        // Prefix sum logic or processor assignment if needed
        k++;
    }

    while (count_active_nodes(nodes) > 1) {
        randomized_contract(nodes, root);
    }
}

/*
void optimal_randomised_tree_evaluation_algorithm(std::vector<Node*>& nodes, Node* root) {
    int x_1 = nodes.size();
    double alpha = 31/32;
    int k = 1;
    int i = 1;
    int 
}
*/

// clang++ -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/include -L/opt/homebrew/lib -lomp main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp randomised.cpp -std=c++17 -pthread -o main