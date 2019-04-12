/*
    Ukkonen's algorithm
    String Algorithms Exam 3 Project

    Zachary Dobbs - zld6fd - 12452097
    4/11/19

    Referencing Dan Gunsfield's Algorithms on Strings, Trees, and Sequences

    Also this: 
    https://stackoverflow.com/questions/9452701/ukkonens-suffix-tree-algorithm-in-plain-english
*/

// includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <map> 
#include <string>
#include <stdio.h>
#include <stdlib.h>

// Node of the actual suffix tree
class Node {
    public:
        Node(Node*, int, int, int*); 
        void addChild(char, Node*); 
        void removeChild(char); 
        void splitEdge(char, char, int, int);  

        int id; 
        Node* parent; 
        Node* link;     
        int b_i;
        int* e_i;
        std::map<char, Node*> children; 
};

// Node constructor 
Node::Node(Node* parent, int id, int b_i, int* e_i) {
    this->parent = parent; 
    this->id = id; 
    this->b_i = b_i; 
    this->e_i = e_i; 
    this->link = NULL;
}

// Add a child to the node's children map
void Node::addChild(char key, Node* node) {
    this->children.insert( std::pair<char, Node*>(key, node) ); 
}

// Removes a child from the node's children map
void Node::removeChild(char key) {
    std::map<char, Node*>::iterator it; 
    it = children.find(key); 
    if (it != children.end()) children.erase(it); 
}

// Splits the edge 
void Node::splitEdge(char x, char y, int c, int new_id) {
    Node* internal = new Node(parent, new_id, b_i, new int(c)); 
    parent->removeChild(x);
    parent->addChild(x, internal); 

    this->parent = internal; 
    this->b_i = c + 1; 
    internal->addChild(y, this);  
}

// Suffix is the current string that is being added to the suffix tree
class Suffix {
    public:
        Suffix(Node*, int); 
        Node* walkUp(int&, int&); 

        Node* node; 
        bool new_node;
        int c; 
};

// Suffix constructor
Suffix::Suffix(Node* node, int c) {
    this->node = node; 
    this->c = c; 
    this->new_node = false; 
}

// Walks up from the end location to find the nearest node above i 
Node* Suffix::walkUp(int& begin, int& end) {
    // if the character index is the end of the node's edge, we are sitting on a node 
    if (c == *(node->e_i) && node->link != NULL) {
        begin = *(node->e_i);
        end   = *(node->e_i)-1;  
        return node;
    }
    // otherwise, we need to get the node above the edge 
    else {
        begin = node->b_i;
        end   = c; 
        return node->parent;
    }
}

// The actual suffix tree that will be constructed by the algorithm 
class SuffixTree {
    public:
        SuffixTree(std::string); 
        char getChar(int); 
        void buildTree(); 
        void singlePhaseAlgorithm(int); 
        int singleExtensionAlgorithm(int, int); 
        Suffix* walkDown(Node*, int, int); 
        bool rule2Check(Suffix*, int);
        void rule2(Suffix*, int, int); 

        Node* root; 
        Node* last_extension; 
        Suffix* last_suffix; 
        std::string text; 
        int internal_nodes; 
        int total_nodes; 
        int* e; 
};

// SuffixTree constructor 
SuffixTree::SuffixTree(std::string text) {
    e = new int(0); 
    this->root = new Node(NULL, 0, 1, e); 
    this->text = text; 
    this->internal_nodes = 0; 
}

// Gets the character for the given index
char SuffixTree::getChar(int i) {
    // the indices are 1-indexed to follow Gunsfield's source
    return text[i - 1];
}

// Builds the suffix tree 
void SuffixTree::buildTree() {
    int m = text.length(); 

    // build the first implicit tree I(1)
    (*e)++; 
    last_extension = new Node(root, 1, 1, e); 
    root->addChild(getChar(last_extension->b_i), last_extension); 
    
    for (int i = 1; i < m; i++) {
        singlePhaseAlgorithm(i);
    }
}

// Single Phase Algorithm as specified by Gunsfield 
void SuffixTree::singlePhaseAlgorithm(int i) {

    last_suffix = new Suffix(last_extension, *e); 

    // perform Rule 1 on all nodes by incrementing the current ene
    (*e)++;  

    // j becomes j(i) and references the last node that was extended in the previous phase 
    for (int j = (last_extension->id + 1); j <= i+1; j++) {
        // if rule 3 gets used then we end this phase automatically 
        if (singleExtensionAlgorithm(j, i) == 3) 
            break; 
    }
}

// Single Extension Algorithm as specified by Gunsfield
int SuffixTree::singleExtensionAlgorithm(int j, int i) {
    // find first node above the last suffix 
    int begin, end; 
    Node* v = last_suffix->walkUp(begin, end); 

    // walk down s(v) or root 
    Suffix* suffix = (v == root ? 
                    walkDown(root, j, i)  :
                    walkDown(v->link, begin, end)
    );

    // we should now be at the end of the path. need to use extension rules to add i+1
    int rule; 
    // indices are 1-indexed so calling text[i] really gets text[i+1]
    if (rule2Check(suffix, i+1)) {
        rule2(suffix, i+1, j);
        rule = 2; 
    }
    else {
        rule = 3; 
    }

    // check if we need to create a suffix link 
    if (last_suffix->new_node) {
        last_suffix->node->link = suffix->node; 
    }

    last_suffix = suffix; 
    return rule; 
}

// Walks down the node looking for suffix text[j..i]
Suffix* SuffixTree::walkDown(Node* v, int begin, int end) {
    int c = *(v->e_i);
    std::map<char, Node*>::iterator it; 

    while (begin <= end) {
        it = v->children.find(getChar(begin));
        if (it == v->children.end()) {
            // some error occurred here
            std::cout << "Error: Could not find " << getChar(begin) << " in node" << v->id << std::endl;
            exit(1);  
        } 
        else {
            v = it->second; 
            // length of current edge 
            int g_p = *(v->e_i) - v->b_i + 1; 
            if (g_p < end - begin + 1) 
                // at the end of the edge 
                c = *(v->e_i);
            else 
                // the string extends to another edge
                c = v->b_i + (end - begin);
            begin += g_p; 
        }
    } 

    Suffix* suffix = new Suffix(v, c); 
    return suffix; 
}

// Checks if rule 2 can be applied 
bool SuffixTree::rule2Check(Suffix* suffix, int i) {
    // does the suffix end at a node or on an edge
    bool node_ending = (suffix->c == *(suffix->node->e_i)); 
    char x = getChar(i); 
    if (node_ending) {
        // the node shouldn't be a leaf 
        if (suffix->node->children.empty()) return false; 
        // see if i+1 is already in the tree
        std::map<char, Node*>::iterator it; 
        it = suffix->node->children.find(getChar(i));
        if (it == suffix->node->children.end())
            // i+1 edge is not there 
            return true; 
        else 
            // i+1 edge is already there
            return false; 
    }
    else {
        if (getChar(suffix->c + 1) == x && (x != '$' || suffix->c + 1 == i)) 
            return false;
        else 
            return true;  
    }
}

// Performs the rule 2 operation
void SuffixTree::rule2(Suffix* suffix, int i, int j) {
    // does the suffix end within an edge?
    if (suffix->c != *(suffix->node->e_i)) {
        // yes, split the edge
        suffix->node->splitEdge(getChar(suffix->node->b_i), getChar(suffix->c + 1), suffix->c, --internal_nodes);
        suffix->node = suffix->node->parent; 
        suffix->new_node = true; 
    }
    Node* leaf = new Node(suffix->node, j, i, e);
    suffix->node->addChild(getChar(leaf->b_i), leaf); 
    last_extension = leaf; 
}

int main(int argc, char* argv[]) {
    // should take in a text file to build the suffix tree from 
    if (argc != 2) {
        std::cout << "Incorrect arguments. Expected `ukkonen file`" << std::endl;
        return 1; 
    }
    else {
        std::ifstream inFile;
        inFile.open(argv[1]);
        if (!inFile.good()) {
            std::cout << "Invalid file" << std::endl;
            return 1; 
        }
        std::stringstream buffer; 
        buffer << inFile.rdbuf();
        SuffixTree* s = new SuffixTree(buffer.str()); 
        // Add the unique character and an endline character 
        s->text += '$';
        s->text += '\0';
        s->buildTree();
        std::cout << "Total internal nodes: " << s->internal_nodes * (-1) << std::endl; 
    }
    return 0; 
}