#include <iostream>
#include <chrono>
#include "Tree.h"

double evaluate_parallel(Node* node);
double randomised_evaluate_parallel(Node* root);
Tree tree_constructor(int n);

int main() {
    try {
        /*
        Node* leftleft = new Node("10");
        Node* leftright = new Node("8");
        Node* left = new Node("*", leftleft, leftright);
        Node* right = new Node("3");
        Node* root = new Node("+", left, right);
        Tree tree(root);
        */

        Tree tree = tree_constructor(10000);

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
        double result_randomised_parallel = randomised_evaluate_parallel(tree.root);
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
    /*
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    */

    return 0;
}

// g++ main.cpp Tree.cpp Node.cpp tree_constructor.cpp divide_and_conquer.cpp -std=c++17 -pthread -o main
// ./main