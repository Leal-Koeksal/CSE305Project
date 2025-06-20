#include "TreeContraction.h"

// -- HELPER FUNCTIONS ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
bool safeParseDouble(const std::string& s, double& val) {
    try {
        val = std::stod(s);
        return true;
    } catch (...) {
        std::cerr << "Failed to parse string as double: " << s << std::endl;
        return false;
    }
}

bool parseFunctionString(const std::string& s, double& a, double& b) {
    static std::regex func_pattern(R"(^(-?\d*\.?\d+),(-?\d*\.?\d+)$)");
    std::smatch matches;
    if (std::regex_match(s, matches, func_pattern)) {
        a = std::stod(matches[1]);
        b = std::stod(matches[2]);
        return true;
    }
    return false;
}

double evaluateFunctionNode(const std::string& func_str, double x) {
    double da, db;
    if (parseFunctionString(func_str, da, db)) {
        int a = static_cast<int>(da);
        int b = static_cast<int>(db);
        int ix = static_cast<int>(x);
        int result = (a * ix + b) % 6101;
        if (result < 0) result += 6101;
        return result;
    }
    return 0.0;
}

void collect_rakeable_nodes(Node* node, 
                           std::vector<Node*>& eval_nodes, 
                           std::vector<Node*>& function_nodes,
                           std::vector<Node*>& function_eval_nodes) {

    if (!node || node->isDeleted()) return;

    Node* left = node->getLeftChild();
    Node* right = node->getRightChild();

    if (!node->is_op() && !node->is_function()) return;

    if (!left && !right) return; // no children

    // both children exist/ not deleted
    if (left && right && !left->isDeleted() && !right->isDeleted()) {
        bool left_leaf = left->is_leaf();
        bool right_leaf = right->is_leaf();

        // case 1: two leaf children
        if (left_leaf && right_leaf) {
            eval_nodes.push_back(node);
            return;
        }

        // case 2: one leaf child
        else if ((left_leaf && !right_leaf) || (!left_leaf && right_leaf)) {
            function_nodes.push_back(node);
            if (!left_leaf) collect_rakeable_nodes(left, eval_nodes, function_nodes, function_eval_nodes);
            if (!right_leaf) collect_rakeable_nodes(right, eval_nodes, function_nodes, function_eval_nodes);
            return;
        }
    }
    // case 3
    if (node->is_function()) {
    // Handle left child
        if (left && !left->isDeleted() && left->is_leaf() &&
            (!right || right->isDeleted())) {
            function_eval_nodes.push_back(node);
            return;
        }

        // Handle right child
        if (right && !right->isDeleted() && right->is_leaf() &&
            (!left || left->isDeleted())) {
            function_eval_nodes.push_back(node);
            return;
        }
    }

    if (left && !left->isDeleted()) collect_rakeable_nodes(left, eval_nodes, function_nodes, function_eval_nodes);
    if (right && !right->isDeleted()) collect_rakeable_nodes(right, eval_nodes, function_nodes, function_eval_nodes);
}

void composeFunctions(Node* first, Node* second) {
    double a1, b1, a2, b2;
    if (!parseFunctionString(first->getString(), a1, b1)) return;
    if (!parseFunctionString(second->getString(), a2, b2)) return;

    int ia1 = static_cast<int>(a1);
    int ib1 = static_cast<int>(b1);
    int ia2 = static_cast<int>(a2);
    int ib2 = static_cast<int>(b2);

    int a = (ia1 * ia2) % 6101;
    int b = (ia1 * ib2 + ib1) % 6101;

    // Ensure positive modulo if needed
    if (b < 0) b += 6101;

    first->setString(std::to_string(a) + "," + std::to_string(b));
    first->setEval(0.0);

    // remove second node from the chain
    Node* grandChild = second->getLeftChild() ? second->getLeftChild() : second->getRightChild();

    if (first->getLeftChild() == second) {
        first->setLeftChild(grandChild);
    } else if (first->getRightChild() == second) {
        first->setRightChild(grandChild);
    }

    if (grandChild) grandChild->setParent(first);
    second->markDeleted();
}


// --- RAKE --------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

void rake(Node* root) {
    if (!root || root->isDeleted()) return;

    std::vector<Node*> eval_nodes;
    std::vector<Node*> function_nodes; // make into function
    std::vector<Node*> function_eval_nodes;

    collect_rakeable_nodes(root, eval_nodes, function_nodes, function_eval_nodes); // collect rakeable ops

    //case 3: evaluate function at leaf child 
    for (Node* node : function_eval_nodes) {
        double a, b;
        if (!parseFunctionString(node->getString(), a, b)) continue;

        Node* left = node->getLeftChild();
        Node* right = node->getRightChild();

        Node* child = left ? left : right;
        if (!child || !child->is_leaf()) continue;

        double x = std::stod(child->getString());
        double val = evaluateFunctionNode(node->getString(), x);

        node->setString(std::to_string(val));
        node->setEval(val);

        child->markDeleted();
        if (left)
            node->setLeftChild(nullptr);
        else
            node->setRightChild(nullptr);
    }

    // Case 2: Function transformation
    for (Node* node : function_nodes) {
        Node* left = node->getLeftChild();
        Node* right = node->getRightChild();
        std::string op = node->getString();

        bool left_leaf = left->is_leaf();
        bool right_leaf = right->is_leaf();

        std::string func;

        if (left_leaf && !right_leaf) {
            if (op == "+") func = "1," + left->getString(); // 5 + x --> 1,5
            if (op == "-") func = "-1," + left->getString(); // 5 - x --> -1,5
            if (op == "*") func = left->getString() + ",0"; // 5 * x --> 5,0
            node->setString(func);
            node->setEval(0.0);
            left->markDeleted();
            node->setLeftChild(nullptr);

        }
        else if (!left_leaf && right_leaf) {
            if (op == "+") func = "1," + right->getString(); // x + 5 --> 1,5
            double val;
            if (safeParseDouble(right->getString(), val)) {
                func = "1," + std::to_string(-val);  // avoids --5
            }
            if (op == "*") func = right->getString() + ",0"; // x * 5 --> 5,0
            node->setString(func);
            node->setEval(0.0);
            right->markDeleted();
            node->setRightChild(nullptr);
        }
    }

    // Case 1: Evaluate both-leaf nodes
    for (Node* node : eval_nodes) {
        
        Node* left = node->getLeftChild();
        Node* right = node->getRightChild();
        std::string op = node->getString();

        double l = std::stod(left->getString());
        double r = std::stod(right->getString());
        double res = 0.0;

        if (op == "+") res = static_cast<int>(l + r) % 6101;
        else if (op == "-") res = ((static_cast<int>(l - r) % 6101 + 6101) % 6101);
        else if (op == "*") res = (static_cast<int>(l * r) % 6101);

        node->setString(std::to_string(res));
        node->setEval(res);
        left->markDeleted();
        right->markDeleted();
        node->setLeftChild(nullptr);
        node->setRightChild(nullptr);
    }
}

// --- COMPRESS ----------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

void compress(Node* root) {
    if (!root || root->isDeleted()) return;

    // Post-order traversal
    compress(root->getLeftChild());
    compress(root->getRightChild());

    // If root is function node with exactly one child which is also function node, compose
    double a, b;
    if (!root->is_leaf() &&
        parseFunctionString(root->getString(), a, b)) {

        Node* child = nullptr;
        if (root->getLeftChild() && !root->getRightChild())
            child = root->getLeftChild();
        else if (!root->getLeftChild() && root->getRightChild())
            child = root->getRightChild();

        if (child && parseFunctionString(child->getString(), a, b)) {
            // Compose root and child
            composeFunctions(root, child);

            // Try compress again at root, in case there is a longer chain
            compress(root);
        }
    }
}



