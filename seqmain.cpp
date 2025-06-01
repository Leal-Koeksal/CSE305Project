#include "TreeContraction.h"
#include "tree_constructor.cpp"

#include <chrono>

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

void print_tree(Node* node, int indent = 0) {
    if (!node) return;
    print_tree(node->getRightChild(), indent + 4);
    std::cout << std::string(indent, ' ') << node->getString() << std::endl;
    print_tree(node->getLeftChild(), indent + 4);
}

int main() {

    Tree tree1 = full_tree_constructor(8);
    Tree tree2 = tree1; // clone for fair comparison

    std::cout << "Tree constructed.\n";

    //print_tree(tree1.getRoot());

    // --- Tree Contraction (Sequential Rake + Compress) ---
    auto start_contract = std::chrono::high_resolution_clock::now();
    Node* root_contract = tree1.getRoot();

    int i = 0;

    while (!(root_contract->is_leaf())) {
            std::cout << "iteration: " << i << "\n" << std::endl;

            rake(root_contract);
            compress(root_contract);

            //debug 
            std::cout << "root_contract" << std::endl;
            ++i;
    }

    std::cout << "Final root string: " << root_contract->getString() << std::endl;

    double result_contract = std::stod(root_contract->getString());
    auto end_contract = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_contract = end_contract - start_contract;

    std::cout << "[Contraction] Result: " << result_contract << "\n";
    std::cout << "[Contraction] Time: " << elapsed_contract.count() << " seconds\n";


    // --- Serial Recursive Evaluation ---
    auto start_serial = std::chrono::high_resolution_clock::now();
    double result_serial = evaluate_serial(tree2.getRoot());
    auto end_serial = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_serial = end_serial - start_serial;
    std::cout << "[Serial Recursion] Result: " << result_serial << "\n";
    std::cout << "[Serial Recursion] Time: " << elapsed_serial.count() << " seconds\n";

    return 0;
}