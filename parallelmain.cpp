#include "TreeContrParallel.h"
#include "tree_constructor2.h"

#include <chrono>

void print_tree(Node* node, int indent = 0) {
    if (!node) return;
    print_tree(node->getRightChild(), indent + 4);
    std::cout << std::string(indent, ' ') << node->getString() << std::endl;
    print_tree(node->getLeftChild(), indent + 4);
}

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

    int l = static_cast<int>(left);
    int r = static_cast<int>(right);

    if (op == "+") result = (l + r) % 6101 ;
    else if (op == "-") result = ((l - r) % 6101  + 6101 ) % 6101 ;
    else if (op == "*") result = (l * r) % 6101 ;
    else if (op == "/") result = (r != 0 ? (l / r) % 6101  : std::numeric_limits<double>::infinity());

    node->setEval(result);
    return result;
}

int main() {
    int n = 500;
    Tree tree1 = full_tree_constructor(n);

    //debug
    std::cout << "Tree created :) \n";

    
    Tree tree2 = tree1;
    Node* root = tree1.getRoot();

   // print_tree(root);

    auto start_serial = std::chrono::high_resolution_clock::now();
    double result_serial = evaluate_serial(tree2.getRoot());
    auto end_serial = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_serial = end_serial - start_serial;
    std::cout << "[Serial Recursion] Result: " << result_serial << "\n";
    std::cout << "[Serial Recursion] Time: " << elapsed_serial.count() << " seconds\n";

    auto start_time = std::chrono::high_resolution_clock::now();
    ThreadPool pool(THREAD_POOL_SIZE);
    std::cout << "No. threads used: " << THREAD_POOL_SIZE;
    std::cout << "\n Size of batch: " << BATCH_SIZE;

    while (root && !root->is_leaf()) {
        parallelRake(pool, root);
        parallelCompress(pool, root);
    }
    std::cout << "\n[Final Contracted Tree]\n";

    double result_contract;
    std::string rootStr = root->getString();

    if (root->is_function()) {
        result_contract = evaluateFunctionNode(rootStr, 0);  // Evaluate at x = 0
    } 
    else {
        result_contract = std::stod(rootStr);
    }

    auto end_contract = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_contract = end_contract - start_time;
    std::cout << "[Contraction] Result: " << result_contract << "\n";
    std::cout << "[Contraction] Time: " << elapsed_contract.count() << " seconds\n";

    return 0;
}