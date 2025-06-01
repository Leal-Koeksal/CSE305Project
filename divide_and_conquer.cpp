
#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include "Tree.h"
#include <iostream>

constexpr int MAX_PARALLEL_DEPTH = 2;


double evaluate(Node* node) {
    if (!node) return 0;
    if (node->hasValue()) return node->getEval();
    if (node->is_leaf() && !node->is_op()) {
        double val = std::stod(node->getString());
        node->setEval(val);
        return val;
    }
    double left = evaluate(node->getLeftChild());
    double right = evaluate(node->getRightChild());
    double result = 0;

    std::string op = node->getString();
    if (op == "+") result = left + right;
    else if (op == "-") result = left - right;
    else if (op == "*") result = left * right;
    else if (op == "/") result = (right != 0 ? left / right : std::numeric_limits<double>::infinity());

    node->setEval(result);
    return result;
}

void evaluate_parallel(Node* node, std::shared_ptr<std::promise<double>> result_promise, int depth = 0) {
    try {
        if (!node) throw std::runtime_error("Node is null");

        if (node->is_leaf()) {
            if (node->is_op()) throw std::runtime_error("Invalid leaf node with operator");
            result_promise->set_value(std::stod(node->getString()));
            return;
        }

        double right_result;
        std::future<double> left_future;

        if (depth < MAX_PARALLEL_DEPTH) {
            // Parallelize only the left subtree
            auto left_promise = std::make_shared<std::promise<double>>();
            left_future = left_promise->get_future();

            std::thread left_thread([=]() {
                evaluate_parallel(node->getLeftChild(), left_promise, depth + 1);
            });

            // Evaluate right subtree in this thread
            right_result = evaluate(node->getRightChild());

            left_thread.join();

        } else {
            // Evaluate both subtrees sequentially
            double left = evaluate(node->getLeftChild());
            double right = evaluate(node->getRightChild());
            std::string op = node->getString();
            if (op == "+") result_promise->set_value(left + right);
            else if (op == "-") result_promise->set_value(left - right);
            else if (op == "*") result_promise->set_value(left * right);
            else if (op == "/") result_promise->set_value(right != 0 ? left / right : std::numeric_limits<double>::infinity());
            else throw std::runtime_error("Unknown operator: " + op);
            return;
        }
        // Merge left and right results
        double left_result = left_future.get();
        std::string op = node->getString();

        if (op == "+") result_promise->set_value(left_result + right_result);
        else if (op == "-") result_promise->set_value(left_result - right_result);
        else if (op == "*") result_promise->set_value(left_result * right_result);
        else if (op == "/") result_promise->set_value(right_result != 0 ? left_result / right_result : std::numeric_limits<double>::infinity());
        else throw std::runtime_error("Unknown operator: " + op);
    }
    catch (...) {
        result_promise->set_exception(std::current_exception());
    }
}

double evaluate_parallel(Node* node) {
    auto result_promise = std::make_shared<std::promise<double>>();
    std::future<double> result_future = result_promise->get_future();
    std::thread main_thread([=]() {
        evaluate_parallel(node, result_promise, 0);
    });
    main_thread.join();
    return result_future.get();
}