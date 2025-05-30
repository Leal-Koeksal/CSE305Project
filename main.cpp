#include <iostream>
#include <chrono>
#include "Tree.h"

Tree tree_constructor(int n);
std::vector<Node*> list_nodes(Tree& tree);
double evaluate_parallel(Node* node);
void randomized_contract(std::vector<Node*>& nodes, Node* root); //randomized_tree_evaluation(std::vector<Node*>& nodes, Node* root);
void optimal_randomised_tree_evaluation_algorithm(std::vector<Node*>& nodes, Tree* tree);

double evaluate_serial(Node* node) {
    if (!node) return 0;
    if (node->hasValue()) return node->getEval();
    if (node->is_leaf() && !node->is_op()) {
        double val = std::stod(node->getString());
        node->setEval(val);
        return val;
    }

    double left = evaluate_serial(node->getLeftChild());
    double right = evaluate_serial(node->getRightChild());
    double result = 0;

    std::string op = node->getString();
    if (op == "+") result = left + right;
    else if (op == "-") result = left - right;
    else if (op == "*") result = left * right;
    else if (op == "/") result = (right != 0 ? left / right : std::numeric_limits<double>::infinity());

    node->setEval(result);
    return result;
}


int main() {
    try {
        Tree tree = tree_constructor(100000);
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

        auto start = std::chrono::high_resolution_clock::now();

        // 1. Call randomized contraction
        randomized_contract(nodes, tree.root);

        // 2. Find the final surviving node and evaluate
        for (Node* node : nodes) {
            if (node && !node->isDeleted()) {
                evaluate_serial(node);  // ⬅️ This must be called here
                std::cout << "Randomised Parallel Result: " << node->getEval() << std::endl;
                break;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_taken = end - start;
        std::cout << "Randomised Parallel Time: " << time_taken.count() << " seconds\n";

        // --- Optimal Randomised Tree Evaluation Timer ---
        auto start_optimal = std::chrono::high_resolution_clock::now();
        // Make a fresh copy of the nodes vector (since nodes may be modified in-place)
        std::vector<Node*> nodes_opt = list_nodes(tree);  // or however you get the full node list
        // Call the optimal randomized tree evaluation algorithm
        optimal_randomised_tree_evaluation_algorithm(nodes_opt, &tree);
        // Find and evaluate the final surviving node
        for (Node* node : nodes_opt) {
            if (node && !node->isDeleted()) {
                evaluate_serial(node);
                std::cout << "Optimal Randomised Result: " << node->getEval() << std::endl;
                break;
            }
        }
        auto end_optimal = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_optimal = end_optimal - start_optimal;
        std::cout << "Optimal Randomised Time: " << time_optimal.count() << " seconds\n";


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