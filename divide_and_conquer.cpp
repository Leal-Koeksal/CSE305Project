/*
#include "Tree.h"
#include <future>
#include <stdexcept>
#include <limits>

// Recursive parallel evaluator using divide-and-conquer
double evaluate_parallel(Node* node) {
    if (!node) throw std::runtime_error("Null node encountered");

    if (node->is_leaf()) {
        if (node->is_op()) {
            throw std::runtime_error("Invalid leaf node with operator");
        }
        return std::stod(node->getString());
    }

    // Launch left and right evaluations in parallel
    auto left_future = std::async(std::launch::async, evaluate_parallel, node->getLeftChild());
    auto right_future = std::async(std::launch::async, evaluate_parallel, node->getRightChild());

    double left = left_future.get();
    double right = right_future.get();
    std::string op = node->getString();

    if (op == "+") return left + right;p
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") return right != 0 ? left / right : std::numeric_limits<double>::infinity();

    throw std::runtime_error("Unknown operator: " + op);
}
*/

/*
#include "Tree.h"
#include <thread>
#include <stdexcept>
#include <limits>
#include <future>

// Internal helper for threaded evaluation
void evaluate_parallel(Node* node, std::promise<double> result_promise) {
    try {
        if (!node) throw std::runtime_error("Null node encountered");

        if (node->is_leaf()) {
            if (node->is_op()) throw std::runtime_error("Invalid leaf node with operator");
            result_promise.set_value(std::stod(node->getString()));
            return;
        }

        // Prepare promises and futures for left and right
        std::promise<double> left_promise;
        std::promise<double> right_promise;
        std::future<double> left_future = left_promise.get_future();
        std::future<double> right_future = right_promise.get_future();

        // Spawn threads for left and right subtree
        std::thread left_thread(evaluate_parallel, node->getLeftChild(), std::move(left_promise));
        std::thread right_thread(evaluate_parallel, node->getRightChild(), std::move(right_promise));

        // Wait for results
        double left = left_future.get();
        double right = right_future.get();

        // Join threads
        left_thread.join();
        right_thread.join();

        // Combine result
        std::string op = node->getString();
        if (op == "+") result_promise.set_value(left + right);
        else if (op == "-") result_promise.set_value(left - right);
        else if (op == "*") result_promise.set_value(left * right);
        else if (op == "/") result_promise.set_value(right != 0 ? left / right : std::numeric_limits<double>::infinity());
        else throw std::runtime_error("Unknown operator: " + op);
    }
    catch (...) {
        result_promise.set_exception(std::current_exception());
    }
}

// Public function to evaluate using threads
double evaluate_parallel(Node* node) {
    std::promise<double> result_promise;
    std::future<double> result_future = result_promise.get_future();
    std::thread main_thread(evaluate_parallel, node, std::move(result_promise));
    main_thread.join();
    return result_future.get();
}
*/
#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include "Tree.h"  // Ensure Node definition is available

void evaluate_parallel(Node* node, std::shared_ptr<std::promise<double>> result_promise) {
    try {
        if (!node) throw std::runtime_error("Null node encountered");

        if (node->is_leaf()) {
            if (node->is_op()) throw std::runtime_error("Invalid leaf node with operator");
            result_promise->set_value(std::stod(node->getString()));
            return;
        }

        auto left_promise = std::make_shared<std::promise<double>>();
        auto right_promise = std::make_shared<std::promise<double>>();
        std::future<double> left_future = left_promise->get_future();
        std::future<double> right_future = right_promise->get_future();

        std::thread left_thread([=]() {
            evaluate_parallel(node->getLeftChild(), left_promise);
        });
        
        std::thread right_thread([=]() {
            evaluate_parallel(node->getRightChild(), right_promise);
        });

        left_thread.join();
        right_thread.join();

        double left = left_future.get();
        double right = right_future.get();

        std::string op = node->getString();
        if (op == "+") result_promise->set_value(left + right);
        else if (op == "-") result_promise->set_value(left - right);
        else if (op == "*") result_promise->set_value(left * right);
        else if (op == "/") result_promise->set_value(right != 0 ? left / right : std::numeric_limits<double>::infinity());
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
        evaluate_parallel(node, result_promise);
    });
    main_thread.join();
    return result_future.get();
}