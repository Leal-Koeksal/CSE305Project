#include "Tree.h"

#include <vector>
#include <limits>
#include <unordered_set>
#include <iostream>
#include <string>

// rake
void collect_rakeable_nodes(Node*, std::vector<Node*>&, std::vector<Node*>&, std::vector<Node*>&);
double evaluateFunctionNode(const std::string&, double);
void rake(Node*);

// compress
bool parseFunctionString(const std::string&, double&, double&); 
void composeFunctions(Node*, Node*);
void compress(Node*);

void contractTree(Node*);



