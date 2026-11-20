#ifndef COMMON_H
#define COMMON_H
#include <string>
#include <vector>
#include <memory>
#include <iostream>

struct Token {
    std::string kind, text, file;
    int line, col;
    size_t start, end;
};
struct Node {
    std::string kind, value, type;
    int line, col;
    size_t token;
    std::vector<std::shared_ptr<Node>> children;
};
using Tree = std::shared_ptr<Node>;
struct Result {
    std::vector<Token> tokens;
    std::vector<std::string> errors;
    std::vector<size_t> bad;
};
Result lex(const std::string &src, const std::string &file);
Tree parse(const std::vector<Token> &tokens, const std::string &file, std::vector<std::string> &errors, std::vector<size_t> &bad);
void show_tree(const Tree &node, int depth = 0);
void highlight(const std::string &src, const std::vector<Token> &tokens, const Tree &root, const std::vector<size_t> &bad, bool html, std::ostream &out);
#endif
