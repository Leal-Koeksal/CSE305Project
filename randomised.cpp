#include <omp.h>
#include <vector>
#include <random>
#include <unordered_map>
#include <string>
#include <stack>
#include "Tree.h"


std::vector<Node*> list_nodes(Tree& tree);

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

std::vector<int> generate_random_permutation(int n) {
    std::vector<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(perm.begin(), perm.end(), g);
    return perm;
}

void discard_zeros(std::vector<Node*>& pointers, int x_max) {
    std::vector<Node*> filtered;
    for (Node* ptr : pointers) {
        if (ptr != nullptr) {
            filtered.push_back(ptr);
            if (filtered.size() == x_max) break;
        }
    }
    pointers = filtered;
}

void optimal_randomised_tree_evaluation_algorithm(std::vector<Node*>& nodes, Tree* tree) {
    std::vector<int> x;
    x.push_back(nodes.size());

    double alpha = 31.0 / 32.0;
    int k = 0, i = 0;
    std::vector<Tree*> T;
    T.push_back(tree);

    // Step (a)
    while (x[i] >= nodes.size() / log(nodes.size())) {
        x.push_back(ceil(alpha * x[i]));
        i++;
    }

    // Step (b): Generate permutations π₁, ..., πᵢ
    std::vector<std::vector<int>> permutations(i);
    #pragma omp parallel for
    for (int j = 0; j < i; ++j) {
        permutations[j] = generate_random_permutation(x[j]);
    }

    // Step (c)
    while (k < i) {
        std::vector<Node*> nodesT_k = list_nodes(*T[k]);
        randomized_contract(nodesT_k, T[k]->root);
    
        std::vector<Node*> permuted;
        for (int idx : permutations[k]) {
            if (idx < nodesT_k.size()) {
                permuted.push_back(nodesT_k[idx]);
            }
        }
    
        discard_zeros(permuted, x[k + 1]);
        nodes = permuted;
    
        Tree* new_tree = new Tree{T[k]->root};  // or updated root
        T.push_back(new_tree);
    
        k++;
    }

    // Step (d): Final contraction until |T| == 1
    while (count_active_nodes(nodes) > 1) {
        dynamic_tree_contraction(nodes, tree->root);
    }

    for (size_t j = 1; j < T.size(); ++j) {
        T[j]->root = nullptr;  // prevent destructor from double-freeing
        delete T[j];
    }
}

/*
void optimal_randomised_tree_evaluation_algorithm(std::vector<Node*>& nodes, Tree* tree) {
    std::vector<int> x = std::vector<int>();
    x.push_back(nodes.size());
    double alpha = 31/32;
    int k = 0;
    int i = 0;
    std::vector<Tree*> T = std::vector<Tree*>();
    T.push_back(tree);
    while (x[i] >= nodes.size() / (log(nodes.size()))) {
        x.push_back(ceil(alpha*x[i]));
        i ++;
    }
    #pragma omp parallel for
    for (int j = 0; j < i; ++j) {
        // generate permutations pi_1, ..., pi_i of sizes x[0], ..., x[i] respectively
    }
    while (k < i) {
        T.push_back(randomized_contract(T[k])); // using p processors
        // permute the pointers of T[k+1] using pi_{k+1}
        // apply the discard zeros to the list of pointers T[k+1] returning at most x[k+1]
        k++;
    }
    while (count_active_nodes(nodes) > 1) {
        dynamic_tree_contraction(nodes, tree->root); // using a distinct processor at each vertex
    }
}
*/

// clang++ -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/include -L/opt/homebrew/lib -lomp main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp randomised.cpp -std=c++17 -pthread -o main