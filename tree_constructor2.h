
#include <random>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <stack>
#include <stdexcept>
#include <functional>
#include "Tree.h"

std::string get_random_operator();
std::string get_random_number();
Tree random_tree_constructor(int);
Tree full_tree_constructor(int);
Tree most_unbalanced_tree_constructor(int);
std::vector<Node*> list_nodes(Tree&);

