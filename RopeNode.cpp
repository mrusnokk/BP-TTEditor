#include <string>
#include <iostream>

struct RopeNode {
    std::string text;
    int weight;

    RopeNode* left;
    RopeNode* right;

    RopeNode(std::string& val)
    {
        text = val;
        weight = text.length();

        left = nullptr;
        right = nullptr;

    }
};