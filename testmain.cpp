#include "TreeContraction.h"
#include <iostream>

// simple expression tree: ((2 + 3) * (4 - 1)) * (7 + 8) => evaluate to 15^2
Node* build_test_tree() {
    // Leaves
    Node* n2 = new Node("2");
    Node* n3 = new Node("3");
    Node* n4 = new Node("4");
    Node* n1 = new Node("1");
    Node* n7 = new Node("7");
    Node* n8 = new Node("8");

    // Subtrees
    Node* plus1 = new Node("+", n2, n3);
    n2->setParent(plus1);
    n3->setParent(plus1);

    Node* minus1 = new Node("-", n4, n1);
    n4->setParent(minus1);
    n1->setParent(minus1);

    Node* mult1 = new Node("*", plus1, minus1);
    plus1->setParent(mult1);
    minus1->setParent(mult1);

    Node* plus2 = new Node("+", n7, n8);
    n7->setParent(plus2);
    n8->setParent(plus2);

    // Root
    Node* root = new Node("*", mult1, plus2);
    mult1->setParent(root);
    plus2->setParent(root);

    return root;
}

// ((12 + 2) - 6) * 11
Node* build_test_tree2(){
    Node* n2 = new Node("2");
    Node* n6 = new Node("6");
    Node* n11 = new Node("11");
    Node* n12 = new Node("12");

    Node* plus = new Node("+", n12, n2);
    n12->setParent(plus);
    n2->setParent(plus);

    Node* minus = new Node("-", plus, n6);
    plus->setParent(minus);
    n6->setParent(minus);

    Node* root = new Node("*", minus, n11);
    minus->setParent(root);
    n11->setParent(root);

    //debug 
    std::cout << "sanity check, is the plus a leaf?? " << plus->is_leaf() << std::endl;

    return root;
}

Node* build_test_tree3(){
    Node* n1 = new Node("1");
    Node* n2 = new Node("2");
    Node* n3 = new Node("3");
    Node* n4 = new Node("4");
    Node* n5 = new Node("5");

    Node* plus = new Node("+", n1, n3);
    n1->setParent(plus);
    n3->setParent(plus);

    Node* mult = new Node("*", plus, n2);
    plus->setParent(mult);
    n2->setParent(mult);

    Node* minus = new Node("-", mult, n5);
    mult->setParent(minus);
    n5->setParent(minus);

    Node* root = new Node("*", minus, n4);
    minus->setParent(root);
    n4->setParent(root);

    return root;
}

Node* build_test_tree4(){
    Node* n1 = new Node("1");
    Node* n2 = new Node("2");
    Node* n3 = new Node("3");
    Node* n4 = new Node("4");
    Node* n5 = new Node("5");

    Node* plus = new Node("+", n1, n3);
    n1->setParent(plus);
    n3->setParent(plus);

    Node* mult = new Node("*", plus, n2);
    plus->setParent(mult);
    n2->setParent(mult);

    Node* minus = new Node("-", mult, n5);
    mult->setParent(minus);
    n5->setParent(minus);

    Node* root = new Node("*", minus, n4);
    minus->setParent(root);
    n4->setParent(root);

    return root;
}

void print_tree(Node* node, int indent = 0) {
    if (!node) return;
    print_tree(node->getRightChild(), indent + 4);
    std::cout << std::string(indent, ' ') << node->getString() << std::endl;
    print_tree(node->getLeftChild(), indent + 4);
}

int main() {
    Node* root = build_test_tree3();
    Tree tree(root);

    print_tree(tree.getRoot());

    std::cout << "Before contraction: " << tree.evaluate() << std::endl;

    rake(root);

    compress(root);

    //debug

    //rake(root);

    std::cout << "After contraction :)" << std::endl;

    print_tree(tree.get_root());


    return 0;
}
