/*
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
*/

#include <thread>
#include <future>
#include <stdexcept>
#include <limits>
#include <string>
#include <atomic>
#include "Tree.h"
#include <iostream>

// Maximum number of threads we ever want to have running at once:
constexpr int MAX_THREADS = 4;  
// (You can tweak this, or use std::thread::hardware_concurrency() if you like.)

// Global atomic counter of how many threads are currently evaluating subtrees:
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

    if      (op == "+") result = left + right;
    else if (op == "-") result = left - right;
    else if (op == "*") result = left * right;
    else if (op == "/") result = (right != 0.0 ? left / right : std::numeric_limits<double>::infinity());
    else throw std::runtime_error("Unknown operator in evaluate(): " + op);

    node->setEval(result);
    return result;
}

// Recursive helper that attempts to spawn a new thread if there is capacity.
// If not, it falls back to plain sequential evaluation of both children.
void evaluate_parallel(
    Node* node,
    std::shared_ptr<std::promise<double>> result_promise
) {
    try {
        if (!node) 
            throw std::runtime_error("Node is null");

        // If it's a leaf, it must be a numeric leaf (not an operator).
        if (node->is_leaf()) {
            if (node->is_op()) 
                throw std::runtime_error("Invalid leaf node with operator");
            double val = std::stod(node->getString());
            result_promise->set_value(val);
            return;
        }

        // Non‐leaf: we have an operator, so we need to evaluate left and right.
        double right_result = 0.0;
        std::future<double> left_future;

        // Check if we can spawn a new thread for the left subtree:
        int old_count = active_threads.load(std::memory_order_relaxed);
        bool do_spawn = false;
        while (old_count < MAX_THREADS) {
            // try to claim one “slot”
            if (active_threads.compare_exchange_weak(old_count, old_count + 1,
                                                     std::memory_order_acquire,
                                                     std::memory_order_relaxed)) {
                do_spawn = true;
                break;
            }
            // else old_count was updated to a larger value; loop until no capacity or succeed
        }

        if (do_spawn) {
            // We will evaluate left in a newly spawned thread.
            auto left_promise = std::make_shared<std::promise<double>>();
            left_future = left_promise->get_future();

            // We must capture node->getLeftChild() into a local variable (to avoid capturing `node` by reference
            // if node might go out of scope). Likewise capture left_promise by value.
            Node* left_child = node->getLeftChild();

            std::thread left_thread([left_child, left_promise]() {
                // Evaluate the left subtree recursively in this new thread:
                evaluate_parallel(left_child, left_promise);

                // Once this thread's work is done, decrement the global counter:
                active_threads.fetch_sub(1, std::memory_order_release);
            });

            // Meanwhile, in this thread, we evaluate the right subtree (always sequentially):
            right_result = evaluate(node->getRightChild());

            // Wait for the left‐thread to finish:
            left_thread.join();
        }
        else {
            // We are at or above MAX_THREADS, so do not spawn. Just evaluate both sides sequentially:
            double left_val  = evaluate(node->getLeftChild());
            right_result     = evaluate(node->getRightChild());

            std::string op = node->getString();
            double result = 0.0;
            if      (op == "+") result = left_val + right_result;
            else if (op == "-") result = left_val - right_result;
            else if (op == "*") result = left_val * right_result;
            else if (op == "/") result = (right_result != 0.0 ? left_val / right_result
                                                               : std::numeric_limits<double>::infinity());
            else throw std::runtime_error("Unknown operator: " + op);

            result_promise->set_value(result);
            return;
        }

        // If we did spawn, then at this point `left_future` holds the left‐subtree’s result:
        double left_result = left_future.get();

        // Now combine left_result and right_result with the current node’s operator:
        std::string op = node->getString();
        double final_res = 0.0;
        if (op == "+") final_res = left_result + right_result;
        else if (op == "-") final_res = left_result - right_result;
        else if (op == "*") final_res = left_result * right_result;
        else if (op == "/") final_res = (right_result != 0.0 
                                         ? left_result / right_result 
                                         : std::numeric_limits<double>::infinity());
        else throw std::runtime_error("Unknown operator: " + op);

        result_promise->set_value(final_res);
    }
    catch (...) {
        result_promise->set_exception(std::current_exception());
    }
}

// Public entry‐point: creates the promise/future pair and invokes the helper on the “main thread.”
double evaluate_parallel(Node* node) {
    auto result_promise = std::make_shared<std::promise<double>>();
    std::future<double> result_future = result_promise->get_future();

    // We could run the very top call in a new thread, but there is no benefit:
    // Just call the helper directly; it will spawn when needed.
    evaluate_parallel(node, result_promise);

    // Either got a value or an exception:
    return result_future.get();
}
