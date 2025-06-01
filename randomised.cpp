#include <omp.h>
#include <vector>
#include <random>
#include <unordered_map>
#include <string>
#include <stack>
#include <atomic>
#include <cmath>
#include "Tree.h"
#include <random>
#include <algorithm>
#include <iterator>
#include <thread>

std::vector<Node*> list_nodes(Tree& tree);

// Arg(v) gives the number of children of v according to resource 2
int Arg(Node* v) {
    int arg = 0;
    if (v->getLeftChild() && !v->getLeftChild()->hasValue()) ++arg;
    if (v->getRightChild() && !v->getRightChild()->hasValue()) ++arg;
    return arg;
}

// Costly function: to be called once
int count_active_nodes(const std::vector<Node*>& nodes) {
    int count = 0;
    for (Node* node : nodes) {
        if (node && !node->isDeleted()) {
            ++count;
        }
    }
    return count;
}

void dynamic_tree_contraction(std::vector<Node*>& nodes, Node* root, std::atomic<int>& active_node_count) {
    const int num_threads = std::thread::hardware_concurrency(); // Number of concurrent threads supported
    // Found at link: https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency.html
    std::vector<std::thread> threads;

    auto worker = [&](int start, int end) { // https://www.geeksforgeeks.org/lambda-expression-in-c/
        for (int i = start; i < end; ++i) {
            Node* v = nodes[i];
            if (!v || v->isDeleted()) continue;

            int num_children = 0;
            Node* l = v->getLeftChild();
            Node* r = v->getRightChild();
            if (l && !l->isDeleted()) ++num_children;
            if (r && !r->isDeleted()) ++num_children;

            Node* parent = v->getParent();
            if (!parent || parent->isDeleted()) continue;

            int num_siblings = 0;
            if (parent->getLeftChild() && !parent->getLeftChild()->isDeleted()) ++num_siblings;
            if (parent->getRightChild() && !parent->getRightChild()->isDeleted()) ++num_siblings;

            if (num_children == 0) {
                if (parent) parent->mark();

                if (active_node_count > 1) {
                    v->markDeleted();
                    --active_node_count;
                }
            } else if (num_children == 1 && num_siblings == 1) {
                Node* grandparent = parent->getParent();
                if (!grandparent || grandparent->isDeleted()) continue;

                if (grandparent->getLeftChild() == parent)
                    grandparent->setLeftChild(v);
                else if (grandparent->getRightChild() == parent)
                    grandparent->setRightChild(v);

                v->setParent(grandparent);
            }
        }
    };

    int chunk_size = nodes.size() / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start = t * chunk_size;
        int end = (t == num_threads - 1) ? nodes.size() : (t + 1) * chunk_size;
        threads.emplace_back(worker, start, end);
    }

    for (auto& thread : threads) thread.join();
}

void randomized_contract(std::vector<Node*>& nodes, Node* root, std::atomic<int>& active_node_count) {
    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    auto worker = [&](int start, int end) {
        std::mt19937 rng(std::random_device{}());
        for (int i = start; i < end; ++i) {
            Node* v = nodes[i];
            if (!v || v->isDeleted()) continue;

            int num_children = 0;
            Node* l = v->getLeftChild();
            Node* r = v->getRightChild();
            if (l && !l->isDeleted()) ++num_children;
            if (r && !r->isDeleted()) ++num_children;

            if (num_children == 0) {
                Node* parent = v->getParent();
                if (parent) parent->mark();

                if (active_node_count > 1) {
                    v->markDeleted();
                    --active_node_count;
                }
            } else if (num_children == 1) {
                v->setSex((rng() % 2 == 0) ? Sex::F : Sex::M);

                Node* parent = v->getParent();
                if (!parent || parent->isDeleted()) continue;

                if (parent->getSex() == Sex::M && v->getSex() == Sex::F) {
                    Node* grandparent = parent->getParent();
                    if (!grandparent || grandparent->isDeleted()) continue;

                    if (grandparent->getLeftChild() == parent)
                        grandparent->setLeftChild(v);
                    else if (grandparent->getRightChild() == parent)
                        grandparent->setRightChild(v);

                    v->setParent(grandparent);

                    if (active_node_count > 1) {
                        parent->markDeleted();
                        --active_node_count;
                    }
                }
            }
        }
    };

    int chunk_size = nodes.size() / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start = t * chunk_size;
        int end = (t == num_threads - 1) ? nodes.size() : (t + 1) * chunk_size;
        threads.emplace_back(worker, start, end);
    }

    for (auto& thread : threads) thread.join();
}



void randomized_tree_evaluation(std::vector<Node*>& nodes, Node* root) {
    int n = nodes.size();
    int p = n * std::log(std::log(n)) / std::log(n);
    int k = 1;
    const int c = 2;

    // Phase 1: initial dynamic contractions with atomic counter
    std::atomic<int> active_node_count(count_active_nodes(nodes));

    while (k <= c * std::log(std::log(n))) {
        if (active_node_count <= 1) break;  // Early exit if tree is reduced
        dynamic_tree_contraction(nodes, root, active_node_count);
        k++;
    }

    // Phase 2: randomized contractions
    while (active_node_count > 1) {
        randomized_contract(nodes, root, active_node_count);
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

    // Step (a) — Build x[i] sizes
    while (x[i] >= nodes.size() / std::log(nodes.size())) {
        x.push_back(std::ceil(alpha * x[i]));
        ++i;
    }

    // Step (b) — Prepare RNG
    std::random_device rd;
    std::mt19937 gen(rd());

    // Step (c) — Iteratively contract + sample
    while (k < i) {
        // Contract current nodes
        std::atomic<int> active_node_count(count_active_nodes(nodes));
        randomized_contract(nodes, tree->root, active_node_count);

        // Filter out deleted nodes (in-place)
        nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
                    [](Node* n) { return !n || n->isDeleted(); }),
                    nodes.end());

        // Sample up to x[k+1] nodes randomly from the survivors
        std::vector<Node*> sampled;
        sampled.reserve(x[k + 1]);

        std::vector<int> indices(nodes.size());
        std::iota(indices.begin(), indices.end(), 0);               // Fill indices 0..n-1
        std::shuffle(indices.begin(), indices.end(), gen);          // Shuffle them

        for (int j = 0; j < x[k + 1] && j < (int)nodes.size(); ++j) {
            sampled.push_back(nodes[indices[j]]);                   // Pick randomly
        }

        nodes = std::move(sampled);  // Update the working set
        ++k;
    }

    // Step (d) — Final contraction to 1 node
    std::atomic<int> active_node_count(count_active_nodes(nodes));
    while (active_node_count > 1) {
        dynamic_tree_contraction(nodes, tree->root, active_node_count);
        nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
                    [](Node* n) { return !n || n->isDeleted(); }),
                    nodes.end());
    }
}

// clang++ -std=c++17 -Xpreprocessor -fopenmp -I/opt/homebrew/include -L/opt/homebrew/lib -lomp main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp randomised.cpp -std=c++17 -pthread -o main