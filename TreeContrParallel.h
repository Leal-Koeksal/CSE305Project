#include "TreeContraction.h"
#include "ThreadPool.h"

#include <thread>
#include <atomic>
#include <mutex>

extern std::atomic<int> thread_count;
extern const int MAX_THREADS;

#pragma once
extern size_t THREAD_POOL_SIZE;
extern size_t BATCH_SIZE;

template <typename Func>
void limited_thread(std::vector<std::thread>&, Func);

// RAKE
void proccess_eval_nodes(const std::vector<Node*>&, std::vector<std::thread>&);
void process_function_nodes(const std::vector<Node*>&, std::vector<std::thread>&);
void process_function_eval_nodes(const std::vector<Node*>&, std::vector<std::thread>&);
void parallelRake(ThreadPool& pool, Node* root);

// COMPRESS
bool isFunctionChainRoot(Node*);
void collectUnaryFuncChains(Node*, std::vector<std::vector<Node*>>&);
void parallelComposeChain(std::vector<Node*>&);
void parallelCompress(ThreadPool&, Node*);


