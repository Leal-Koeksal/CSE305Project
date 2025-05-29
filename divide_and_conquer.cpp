/*
#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include "Tree.h"  // Ensure Node definition is available

void evaluate_parallel(Node* node, std::shared_ptr<std::promise<double>> result_promise) {
    try {
        if (!node) throw std::runtime_error("Node is null");

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
*/

/*
#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include "Tree.h"

void evaluate_parallel(Node* node, std::shared_ptr<std::promise<double>> result_promise) {
    try {
        if (!node) throw std::runtime_error("Node is null");

        if (node->is_leaf()) {
            if (node->is_op()) throw std::runtime_error("Invalid leaf node with operator");
            result_promise->set_value(std::stod(node->getString()));
            return;
        }

        auto left_promise = std::make_shared<std::promise<double>>();
        std::future<double> left_future = left_promise->get_future();

        // Parallel left subtree
        std::thread left_thread([=]() {
            evaluate_parallel(node->getLeftChild(), left_promise);
        });

        // Right subtree in current thread
        auto right_promise = std::make_shared<std::promise<double>>();
        evaluate_parallel(node->getRightChild(), right_promise);
        double right = right_promise->get_future().get();

        left_thread.join();
        double left = left_future.get();

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
*/

#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include "Tree.h"

constexpr int MAX_PARALLEL_DEPTH = 2;

double evaluate(Node* node, int depth = 0);

void evaluate_parallel(Node* node, std::shared_ptr<std::promise<double>> result_promise, int depth = 0) {
    try {
        if (!node) throw std::runtime_error("Node is null");

        if (node->is_leaf()) {
            if (node->is_op()) throw std::runtime_error("Invalid leaf node with operator");
            result_promise->set_value(std::stod(node->getString()));
            return;
        }

        std::future<double> left_future, right_future;

        if (depth < MAX_PARALLEL_DEPTH) {
            // Parallel evaluation
            auto left_promise = std::make_shared<std::promise<double>>();
            auto right_promise = std::make_shared<std::promise<double>>();
            left_future = left_promise->get_future();
            right_future = right_promise->get_future();

            std::thread left_thread([=]() {
                evaluate_parallel(node->getLeftChild(), left_promise, depth + 1);
            });

            std::thread right_thread([=]() {
                evaluate_parallel(node->getRightChild(), right_promise, depth + 1);
            });

            left_thread.join();
            right_thread.join();
        } else {
            // Sequential evaluation
            double left = evaluate(node->getLeftChild(), depth + 1);
            double right = evaluate(node->getRightChild(), depth + 1);

            std::string op = node->getString();
            if (op == "+") result_promise->set_value(left + right);
            else if (op == "-") result_promise->set_value(left - right);
            else if (op == "*") result_promise->set_value(left * right);
            else if (op == "/") result_promise->set_value(right != 0 ? left / right : std::numeric_limits<double>::infinity());
            else throw std::runtime_error("Unknown operator: " + op);
            return;
        }

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

// Helper function for sequential evaluation (used below threshold)
double evaluate(Node* node, int depth) {
    if (!node) throw std::runtime_error("Node is null");

    if (node->is_leaf()) {
        if (node->is_op()) throw std::runtime_error("Invalid leaf node with operator");
        return std::stod(node->getString());
    }

    double left = evaluate(node->getLeftChild(), depth + 1);
    double right = evaluate(node->getRightChild(), depth + 1);

    std::string op = node->getString();
    if (op == "+") return left + right;
    else if (op == "-") return left - right;
    else if (op == "*") return left * right;
    else if (op == "/") return right != 0 ? left / right : std::numeric_limits<double>::infinity();
    else throw std::runtime_error("Unknown operator: " + op);
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
