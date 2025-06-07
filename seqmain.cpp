#include "TreeContraction.h"
#include "tree_constructor2.cpp"

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

    int l = static_cast<int>(left);
    int r = static_cast<int>(right);


    if (op == "+") result = (l + r) % 6101;
    else if (op == "-") result = ((l - r) % 6101 + 6101) % 6101;
    else if (op == "*") result = (l * r) % 6101;
    else if (op == "/") result = (r != 0 ? (l / r) % 6101 : std::numeric_limits<double>::infinity());

    node->setEval(result);
    return result;
}

void print_tree(Node* node, int indent = 0) {
    if (!node) return;
    print_tree(node->getRightChild(), indent + 4);
    std::cout << std::string(indent, ' ') << node->getString() << std::endl;
    print_tree(node->getLeftChild(), indent + 4);
}

int count_nodes(Node* root) {
    if (!root || root->isDeleted()) return 0;
    return 1 + count_nodes(root->getLeftChild()) + count_nodes(root->getRightChild());
}

int main() {
    int i = 10000;
    Tree tree1 = full_tree_constructor(i);
    Tree tree2 = tree1; // clone for fair comparison

    std::cout << "Tree constructed! It has " << i <<" nodes \n";

    // --- Tree Contraction (Sequential Rake + Compress) ---
    auto start_contract = std::chrono::high_resolution_clock::now();
    Node* root_contract = tree1.getRoot();

    int iteration = 0;
    int last_count = -1;
    while (!(root_contract->is_leaf())) {
        rake(root_contract);
        compress(root_contract);

        int active_nodes = count_nodes(root_contract); // You'll need to implement this
        //std::cout << "Iteration " << iteration++ << ", Active nodes: " << active_nodes << "\n";

        if (active_nodes == last_count) {
            std::cerr << "[Error] No progress in contraction; possible infinite loop.\n";
            break;
        }
        last_count = active_nodes;
    }

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