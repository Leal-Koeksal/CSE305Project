#include "TreeContrParallel.h"

// -- THREAD AUX ---------------------------------------
// -----------------------------------------------------

// version 1
constexpr int MAX_THREADS = 8;
std::atomic<int> thread_count(0);

template <typename Func>
void limited_thread(std::vector<std::thread>& threads, Func task) {
    if (thread_count.load() < MAX_THREADS) {
        thread_count++;
        threads.emplace_back([=] (){ // the syntax for this lamnda function was created using AI.
            task(); 
            thread_count--;
        });
    }
    else {
        // no more threads available, run sequentially
        task();
    }
}

// version 2

size_t THREAD_POOL_SIZE = 1;
size_t BATCH_SIZE = 15;

// -- RAKE ---------------------------------------------
// -----------------------------------------------------

// version 1

// // case 1
// void process_eval_nodes(const std::vector<Node*>& nodes, std::vector<std::thread>& threads) {
//     for (Node* node : nodes) {
//         limited_thread(threads, [node]() {
//             Node* left = node->getLeftChild();
//             Node* right = node->getRightChild();
//             std::string op = node->getString();

//             double l = std::stod(left->getString());
//             double r = std::stof(right->getString());
//             double res = 0.0;

//             if (op == "+") res = l + r;
//             else if (op == "-") res = l - r;
//             else if (op == "*") res = l * r;

//             node->setString(std::to_string(res));
//             node->setEval(res);

//             left->markDeleted();
//             right->markDeleted();
//             node->setLeftChild(nullptr);
//             node->setRightChild(nullptr);
//         });
//     }
// }

// // case 2
// void process_function_nodes(const std::vector<Node*>& nodes, std::vector<std::thread>& threads) {
//     for (Node* node : nodes) {
//         limited_thread(threads, [node]() {
//             Node* left = node->getLeftChild();
//             Node* right = node->getRightChild();
//             std::string op = node->getString();

//             bool left_leaf = left && left->is_leaf();
//             bool right_leaf = right && right->is_leaf();

//             std::string func;

//             if (left_leaf && !right_leaf) {
//                 if (op == "+") func = "1," + left->getString();
//                 if (op == "-") func = "-1," + left->getString();
//                 if (op == "*") func = left->getString() + ",0";

//                 node->setString(func);
//                 node->setEval(0.0);
//                 left->markDeleted();
//                 node->setLeftChild(nullptr);

//             } 
//             else if (!left_leaf && right_leaf) {
//                 if (op == "+") func = "1," + right->getString();
//                 if (op == "-") func = "1,-" + right->getString();
//                 if (op == "*") func = right->getString() + ",0";

//                 node->setString(func);
//                 node->setEval(0.0);
//                 right->markDeleted();
//                 node->setRightChild(nullptr);
//             }
//         });
//     }
// }

// // case 3
// void process_function_eval_nodes(const std::vector<Node*>& nodes, std::vector<std::thread>& threads) {
//     for (Node* node : nodes) {
//         limited_thread(threads, [node]() {
//             double a, b;
//             if (!parseFunctionString(node->getString(), a, b)) return;

//             Node* left = node->getLeftChild();
//             Node* right = node->getRightChild();
//             Node* child = left ? left : right;
//             if (!child || !child->is_leaf()) return;

//             double x = std::stod(child->getString());
//             double val = evaluateFunctionNode(node->getString(), x);
//             node->setString(std::to_string(val));
//             node->setEval(val);

//             child->markDeleted();
//             if (node->getLeftChild() == child)
//                 node->setLeftChild(nullptr);
//             else
//                 node->setRightChild(nullptr);
//         });
//     }
// }

// void parallelRake(Node* root) {
//     if (!root || root->isDeleted()) return;

//     std::vector<Node*> eval_nodes; // case 1
//     std::vector<Node*> function_nodes; // case 2 
//     std::vector<Node*> function_eval_nodes; // case 2

//     collect_rakeable_nodes(root, eval_nodes, function_nodes, function_eval_nodes);

//     std::vector<std::thread> threads;

//     process_function_eval_nodes(function_eval_nodes, threads);
//     process_function_nodes(function_nodes, threads);
//     process_eval_nodes(eval_nodes, threads);

//     for (auto& thread : threads) {
//         thread.join();
//     }
// }

// version 2 - thread pool

void process_eval_nodes(const std::vector<Node*>& nodes, ThreadPool& pool) {
    const size_t BATCH = BATCH_SIZE;
    for (size_t i = 0; i < nodes.size(); i += BATCH) {
        size_t end = std::min(i + BATCH, nodes.size());
        pool.enqueue([=]() {
            for (size_t j = i; j < end; ++j) {
                Node* node = nodes[j];
                Node* left = node->getLeftChild();
                Node* right = node->getRightChild();
                std::string op = node->getString();
                double l = std::stod(left->getString());
                double r = std::stod(right->getString());

                int il = static_cast<int>(l);
                int ir = static_cast<int>(r);
                double res = (op == "+") ? (il + ir) % 6101 :
                (op == "-") ? ((il - ir) % 6101 + 6101) % 6101 :
                (il * ir) % 6101;

                node->setString(std::to_string(res));
                node->setEval(res);
                left->markDeleted();
                right->markDeleted();
                node->setLeftChild(nullptr);
                node->setRightChild(nullptr);
            }
        });
    }
}

void process_function_nodes(const std::vector<Node*>& nodes, ThreadPool& pool) {
    const size_t BATCH = BATCH_SIZE;
    for (size_t i = 0; i < nodes.size(); i += BATCH) {
        size_t end = std::min(i + BATCH, nodes.size());
        pool.enqueue([=]() {
            for (size_t j = i; j < end; ++j) {
                Node* node = nodes[j];
                if (!node || node->isDeleted()) continue;

                Node* left = node->getLeftChild();
                Node* right = node->getRightChild();
                std::string op = node->getString();
                bool left_leaf = left && left->is_leaf();
                bool right_leaf = right && right->is_leaf();
                std::string func;

                if (left_leaf && !right_leaf) {
                    int val = static_cast<int>(std::stod(left->getString()));
                    if (op == "+") {
                        func = "1," + std::to_string((val % 6101 + 6101) % 6101);
                    } else if (op == "-") {
                        int mod_val = ((-val % 6101) + 6101) % 6101;
                        func = "1," + std::to_string(mod_val);
                    } else if (op == "*") {
                        int coeff = (val % 6101 + 6101) % 6101;
                        func = std::to_string(coeff) + ",0";
                    }
                    node->setString(func);
                    node->setEval(0.0);
                    left->markDeleted();
                    node->setLeftChild(nullptr);
                }

                else if (!left_leaf && right_leaf) {
                    int val = static_cast<int>(std::stod(right->getString()));
                    if (op == "+") {
                        func = "1," + std::to_string((val % 6101 + 6101) % 6101);
                    } else if (op == "-") {
                        int mod_val = ((-val % 6101) + 6101) % 6101;
                        func = "1," + std::to_string(mod_val);
                    } else if (op == "*") {
                        int coeff = (val % 6101 + 6101) % 6101;
                        func = std::to_string(coeff) + ",0";
                    }
                    node->setString(func);
                    node->setEval(0.0);
                    right->markDeleted();
                    node->setRightChild(nullptr);
                }
            }
        });
    }
}


void process_function_eval_nodes(const std::vector<Node*>& nodes, ThreadPool& pool) {
    const size_t BATCH = BATCH_SIZE;
    for (size_t i = 0; i < nodes.size(); i += BATCH) {
        size_t end = std::min(i + BATCH, nodes.size());
        pool.enqueue([=]() {
            for (size_t j = i; j < end; ++j) {
                Node* node = nodes[j];
                if (!node || node->isDeleted()) continue;

                Node* left = node->getLeftChild();
                Node* right = node->getRightChild();
                Node* child = (left && left->is_leaf()) ? left : (right && right->is_leaf()) ? right : nullptr;
                if (!child) continue;

                double x = std::stod(child->getString());
                double val = evaluateFunctionNode(node->getString(), x);

                // Apply modulo and set result
                int ival = static_cast<int>(val) % 6101;
                if (ival < 0) ival += 6101;

                node->setString(std::to_string(ival));
                node->setEval(ival);
                child->markDeleted();

                if (child == left) {
                    node->setLeftChild(nullptr);
                } else {
                    node->setRightChild(nullptr);
                }
            }
        });
    }
}


void parallelRake(ThreadPool& pool, Node* root) {
    if (!root || root->isDeleted()) return;

    std::vector<Node*> eval_nodes, function_nodes, function_eval_nodes;
    collect_rakeable_nodes(root, eval_nodes, function_nodes, function_eval_nodes);

    // ThreadPool pool(THREAD_POOL_SIZE);
    process_function_eval_nodes(function_eval_nodes, pool);
    process_function_nodes(function_nodes, pool);
    process_eval_nodes(eval_nodes, pool);

    pool.wait();
    //std::cout << "[parallelRake] End" << std::endl;
}



// -- COMPRESS ------------------------------------------
// ------------------------------------------------------

bool isFunctionChainRoot(Node* node) {
    if (!node || !node->is_function() || node->is_leaf()) return false;

    Node* child = node->getLeftChild() ? node->getLeftChild() : node->getRightChild();
    if (!child || !child->is_function()) return false;

    // this means node is mid-chain, and not chain root
    if (node->getParent() && node->getParent()->is_function()) return false;

    return true;
}

void collectUnaryFuncChains(Node* root, std::vector<std::vector<Node*>>& chains) {
    if (!root || root->isDeleted()) return;

    if (isFunctionChainRoot(root)) {
        std::vector<Node*> chain;
        Node* current = root;

        while(current && current->is_function() && !current->is_leaf()) {
            chain.push_back(current);
            // root is function chain root, hence it only has one child
            Node* next = current->getLeftChild() ? current->getLeftChild() : current->getRightChild();
            if (!next || !next->is_function()) break;
            current = next;
        }

        if (chain.size() > 1) chains.push_back(chain);

    }
    else {
        collectUnaryFuncChains(root->getLeftChild(), chains);
        collectUnaryFuncChains(root->getRightChild(), chains);
    }
}

// this compresses the while chain into one in one go
// the compression is preformed each round pairwise 
// each computation is preformed by a thread
// void  parallelComposeChain(std::vector<Node*>& chain) {
//     while (chain.size() > 1) {
//         std::vector<Node*> next_round;
//         std::vector<std::thread> threads;

//         for (size_t i = 0; i < chain.size() - 2; i += 2) {
//             Node* f1 = chain[i];
//             Node* f2 = chain[i+1];

//             limited_thread(threads, [f1, f2]() {
//                 composeFunctions(f1, f2);
//             });
//             next_round.push_back(f1);
//         }

//         if (chain.size() % 2 == 1) next_round.push_back(chain.back());

//         for (auto& thread : threads) {
//             thread.join();
//         }

//         chain = std::move(next_round); // `more efficient than copying elements, found this https://stackoverflow.com/questions/3413470/what-is-stdmove-and-when-should-it-be-used 
//     }
// }

// void parallelCompress(Node* root) {
//     if (!root || root->isDeleted()) return;

//     std::vector<std::vector<Node*>> chains;
//     collectUnaryFuncChains(root, chains);

//     for (auto& chain: chains) {
//         parallelComposeChain(chain);
//     }
// }

// verison 2: thread pool

void parallelComposeChain(std::vector<Node*>& chain, ThreadPool& pool) {
    int round = 0;
    while (chain.size() > 1) {
        std::vector<Node*> next_round;

        for (size_t i = 0; i < chain.size() - 1; i += 2) {
            Node* f1 = chain[i];
            Node* f2 = chain[i + 1];

            // std::cout << "[Compose] " << f1->getString() << " ° " << f2->getString() << "\n";

            pool.enqueue([f1, f2]() {
                composeFunctions(f1, f2);
            });

            next_round.push_back(f1);
        }

        if (chain.size() % 2 == 1) {
            next_round.push_back(chain.back());
        }

        pool.wait(); // Ensure this round finishes before next
        chain = std::move(next_round);
    }
}

void parallelCompress(ThreadPool& pool, Node* root) {
    //std::cout << "[parallelCompress] Start" << std::endl;
    if (!root || root->isDeleted()) return;

    std::vector<std::vector<Node*>> chains;
    collectUnaryFuncChains(root, chains);

    // ThreadPool pool(8);
    for (auto& chain : chains) {
        parallelComposeChain(chain, pool);
    }
    pool.wait();
    //std::cout << "[parallelCompress] End" << std::endl;
}


