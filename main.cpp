#include <iostream>
#include <chrono>
#include "Tree.h"

double evaluate_parallel(Node* node);
void randomized_contract(std::vector<Node*>& nodes, Node* root);
Tree tree_constructor(int n);
std::vector<Node*> list_nodes(const Tree& tree);

int main() {
    try {
        Tree tree = tree_constructor(1000000);
        std::vector<Node*> nodes = list_nodes(tree);
        // --- Serial Evaluation Timer ---
        auto start_serial = std::chrono::high_resolution_clock::now();
        double result_serial = tree.evaluate();
        auto end_serial = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_serial = end_serial - start_serial;

        std::cout << "Serial Result: " << result_serial << "\n";
        std::cout << "Serial Time: " << elapsed_serial.count() << " seconds\n";
        // --- Parallel Evaluation Timer ---
        auto start_parallel = std::chrono::high_resolution_clock::now();
        double result_parallel = evaluate_parallel(tree.root);
        auto end_parallel = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_parallel = end_parallel - start_parallel;

        std::cout << "Parallel Result: " << result_parallel << "\n";
        std::cout << "Parallel Time: " << elapsed_parallel.count() << " seconds\n";

        // --- Randomised Parallel Evaluation Timer ---
        auto start_randomised_parallel = std::chrono::high_resolution_clock::now();
        randomized_contract(nodes, tree.root);
        double result_randomised_parallel = tree.root->getEval();
        auto end_randomised_parallel = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_randomised_parallel = end_randomised_parallel - start_randomised_parallel;

        std::cout << "Randomised Parallel Result: " << result_randomised_parallel << "\n";
        std::cout << "Randomised Parallel Time: " << elapsed_randomised_parallel.count() << " seconds\n";

    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred!" << std::endl;
        return 1;
    }

    return 0;
}

/*
int main() {
    // Create leaf nodes
    Node* n3 = new Node("3");
    Node* n5 = new Node("5");
    Node* n2 = new Node("2");
    Node* n1 = new Node("1");

    // Create operator nodes
    Node* plus = new Node("+", n3, n5);
    Node* minus = new Node("-", n2, n1);
    Node* mult = new Node("*", plus, minus);

    // Set parents
    n3->setParent(plus);
    n5->setParent(plus);
    n2->setParent(minus);
    n1->setParent(minus);
    plus->setParent(mult);
    minus->setParent(mult);

    // Collect all nodes into a vector for processing
    std::vector<Node*> nodes = { mult, plus, minus, n3, n5, n2, n1 };

    // Run randomized contraction
    randomized_contract(nodes, mult);

    // Print which nodes are deleted
    std::cout << "Deleted nodes after contraction:\n";
    for (Node* node : nodes) {
        std::cout << node->getString() << ": "
                  << (node->isDeleted() ? "deleted" : "active") << "\n";
    }
    std::cout << "Final result: " << mult->getEval() << std::endl;
    return 0;
}
*/

// g++ main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp -std=c++17 -pthread -o main
// ./main