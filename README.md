**CSE305 Project: Parallel expression evaluation**

This project provides implementations of various algorithms for evaluating expression trees both serially and in parallel:

* **Serial Evaluation**: A straightforward recursive evaluation of the tree.
* **Fixed-Thread Parallel Evaluation**: Spawns up to a configurable maximum number of threads to evaluate subtrees concurrently.
* **Randomized Contraction Evaluation**: Repeatedly contracts random nodes in parallel until one node remains.
* **Optimal Randomized Evaluation**: A refined, theoretically optimal randomized contraction strategy.

Each algorithm can be benchmarked on different tree shapes:

* **Full Tree**: A full binary tree tree with a specified number of leaves.
* **Most-Unbalanced Tree**: A left skewed binary tree of a given height, where every left child is an operator and every right child is a number.
* **Random Tree**: A binary tree whose shape is randomly generated up to a specified height.

---

## Building the Project

1. **Clone the repository**

   ```bash
   git clone <repository_url>
   cd <project_root>
   ```

2. **Compile**

   Use the provided `clang++` command at the bottom of `main.cpp`. For example:

   ```bash
   clang++ -std=c++17 -Xpreprocessor -fopenmp \
     -I/opt/homebrew/include -L/opt/homebrew/lib -lomp \
     main.cpp Tree.cpp Node.cpp tree_constructor.cpp \
     divide_and_conquer.cpp randomised.cpp \
     -pthread -o tree_eval
   ```
   
3. **Run**

   ```bash
   ./tree_eval
   ```

---

## Usage

In `main.cpp`, you can select which tree to construct by uncommenting one of the following lines:

```cpp
// Full (complete) tree with N leaves:
// Tree tree = full_tree_constructor(N);

// Most-unbalanced tree of height H:
// Tree tree = most_unbalanced_tree_constructor(H);

// Random tree up to height H:
Tree tree = random_tree_constructor(H);
```

* For **`full_tree_constructor(n)`**, `n` is the number of leaf nodes.
* For **`most_unbalanced_tree_constructor(height)`**, `height` is the tree height (i.e., number of internal nodes along the longest path).
* For **`random_tree_constructor(height)`**, `height` is the maximum depth; the constructor randomly decides branching.

After constructing the tree, the program will automatically:

1. Evaluate the tree **serially** and print the result and elapsed time.
2. Evaluate **parallel** (fixed number of threads) and print the result and elapsed time.
3. Perform **randomized contraction** and print the result and elapsed time.
4. Perform **optimal randomized** evaluation and print the result and elapsed time.

Example output:

```
Tree is constructed.
Serial Result: 0.117324
Serial Time: 1.542e-06 seconds
Parallel Result: 0.117324
Parallel Time: 6.5167e-05 seconds
Randomised Parallel Result: 0.117324
Randomised Parallel Time: 0.00014675 seconds
Optimal Randomised Result: 0.117324
Optimal Randomised Time: 1.2041e-05 seconds
```

---

## File Structure

* `main.cpp` — Driver program and timing harness.
* `Tree.h` / `Tree.cpp` — Tree data structure, constructors, and serial evaluation.
* `Node.cpp` / `Node.h` — Representation of individual nodes.
* `tree_constructor.cpp` — Implementations of the three tree constructors.
* `divide_and_conquer.cpp` — Fixed-thread parallel evaluation logic.
* `randomised.cpp` — Randomized contraction and optimal randomized algorithms.
