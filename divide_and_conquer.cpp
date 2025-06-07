#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include <atomic>
#include "Tree.h"
#include <iostream>

constexpr int LARGE_PRIME = 6101;

static std::atomic<int> active_threads{0};

double evaluate(Node* node) {
    if (!node) return 0.0;
    if (node->hasValue()) 
        return node->getEval();

    if (node->is_leaf() && !node->is_op()) {
        double val = std::stod(node->getString());
        node->setEval(val);
        return val;
    }

    // Otherwise, evaluate left and right completely sequentially:
    double left  = evaluate(node->getLeftChild());
    double right = evaluate(node->getRightChild());
    double result = 0.0;
    std::string op = node->getString();

    if      (op == "+") result = std::fmod(left + right, static_cast<double>(LARGE_PRIME));
    else if (op == "-") result = std::fmod(left - right, static_cast<double>(LARGE_PRIME));
    else if (op == "*") result = std::fmod(left * right, static_cast<double>(LARGE_PRIME));
    else if (op == "/") result = (right != 0.0 ? std::fmod(left / right, static_cast<double>(LARGE_PRIME)) : std::numeric_limits<double>::infinity());
    else throw std::runtime_error("Unknown operator in evaluate(): " + op);

    node->setEval(result);
    return result;
}

// Recursive helper that attempts to spawn a new thread if there is capacity.
void evaluate_parallel(
    Node* node,
    std::shared_ptr<std::promise<double>> result_promise,
    int MAX_THREADS
) {
    try {
        if (!node) 
            throw std::runtime_error("Node is null");

        // If it's a leaf, it must be numeric (not an operator).
        if (node->is_leaf()) {
            if (node->is_op()) 
                throw std::runtime_error("Invalid leaf node with operator");
            double val = std::stod(node->getString());
            result_promise->set_value(val);
            return;
        }

        // Non‐leaf: operator, evaluate left and right.
        double right_result = 0.0;
        std::future<double> left_future;

        int old_count = active_threads.load(std::memory_order_relaxed);
        bool do_spawn = false;
        while (old_count < MAX_THREADS) {
            if (active_threads.compare_exchange_weak(old_count, old_count + 1,
                                                     std::memory_order_acquire,
                                                     std::memory_order_relaxed)) {
                do_spawn = true;
                break;
            }
        }

        if (do_spawn) {
            auto left_promise = std::make_shared<std::promise<double>>();
            left_future = left_promise->get_future();
            Node* left_child = node->getLeftChild();

            /*
            std::thread left_thread([left_child, left_promise]() {
                // Evaluate the left subtree recursively in this new thread:
                evaluate_parallel(left_child, left_promise, MAX_THREADS);

                // Once this thread's work is done, decrement the global counter:
                active_threads.fetch_sub(1, std::memory_order_release);
            });
            */

            std::thread left_thread([left_child, left_promise, MAX_THREADS]() {
                evaluate_parallel(left_child, left_promise, MAX_THREADS);
            
                active_threads.fetch_sub(1, std::memory_order_release);
            });

            right_result = evaluate(node->getRightChild());

            left_thread.join();
        }
        else {
            // We are at or above MAX_THREADS, so do not spawn. Just evaluate both sides sequentially:
            double left_val  = evaluate(node->getLeftChild());
            right_result     = evaluate(node->getRightChild());

            std::string op = node->getString();
            double result = 0.0;
            if      (op == "+") result = std::fmod(left_val + right_result, static_cast<double>(LARGE_PRIME));
            else if (op == "-") result = std::fmod(left_val - right_result, static_cast<double>(LARGE_PRIME));
            else if (op == "*") result = std::fmod(left_val * right_result, static_cast<double>(LARGE_PRIME));
            else if (op == "/") result = (right_result != 0.0 ? std::fmod(left_val / right_result, static_cast<double>(LARGE_PRIME))
                                                               : std::numeric_limits<double>::infinity());
            else throw std::runtime_error("Unknown operator: " + op);

            result_promise->set_value(result);
            return;
        }

        double left_result = left_future.get();

        // Combine left_result and right_result with the current node’s operator:
        std::string op = node->getString();
        double final_res = 0.0;
        if (op == "+") final_res = std::fmod(left_result + right_result, static_cast<double>(LARGE_PRIME));
        else if (op == "-") final_res = std::fmod(left_result - right_result, static_cast<double>(LARGE_PRIME));
        else if (op == "*") final_res = std::fmod(left_result * right_result, static_cast<double>(LARGE_PRIME));
        else if (op == "/") final_res = (right_result != 0.0 
                                         ? std::fmod(left_result / right_result, static_cast<double>(LARGE_PRIME))
                                         : std::numeric_limits<double>::infinity());
        else throw std::runtime_error("Unknown operator: " + op);

        result_promise->set_value(final_res);
    }
    catch (...) {
        result_promise->set_exception(std::current_exception());
    }
}

double evaluate_parallel(Node* node, int MAX_THREADS) {
    auto result_promise = std::make_shared<std::promise<double>>();
    std::future<double> result_future = result_promise->get_future();
    evaluate_parallel(node, result_promise, MAX_THREADS);
    return result_future.get();
}
