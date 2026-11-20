#include "common.h"
#include <cctype>
#include <set>
#include <sstream>
using namespace std;

Result lex(const string &src, const string &file) {
    Result result;
    size_t i = 0;
    int line = 1, col = 1;
    const set<string> words = {"array", "boolean", "char", "else", "for", "function", "if", "integer", "print", "return", "string", "void", "while"};
    auto move = [&]() {
        if (src[i] == '\r') { line++; col = 1; }
        else if (src[i] == '\n') {
            if (i == 0 || src[i-1] != '\r') line++;
            col = 1;
        } else col++;
        i++;
    };
    auto add = [&](string kind, size_t start, int row, int column) {
        result.tokens.push_back({kind, src.substr(start, i-start), file, row, column, start, i});
    };
    auto error = [&](int row, int column, const string &msg) {
        result.errors.push_back(file + ":" + to_string(row) + ":" + to_string(column) + ": " + msg);
    };
    while (i < src.size()) {
        size_t start = i;
        int row = line, column = col;
        unsigned char c = src[i];
        if (c==' ' || c=='\t' || c=='\r' || c=='\n') { move(); continue; }
        if (isalpha(c) || c == '_') {
            move();
            while (i < src.size() && (isalnum((unsigned char)src[i]) || src[i]=='_')) move();
            string s = src.substr(start, i-start);
            add(s == "true" || s == "false" ? "boolean" : words.count(s) ? "keyword" : "identifier", start, row, column);
        } else if (isdigit(c)) {
            while (i < src.size() && isdigit((unsigned char)src[i])) move();
            add("integer", start, row, column);
        } else if (c=='/' && i+1<src.size() && src[i+1]=='/') {
            move(); move(); while (i<src.size() && src[i]!='\n' && src[i]!='\r') move();
            add("comment", start, row, column);
        } else if (c=='/' && i+1<src.size() && src[i+1]=='*') {
            move(); move(); bool closed = false;
            while (i<src.size()) {
                if (src[i]=='*' && i+1<src.size() && src[i+1]=='/') { move(); move(); closed=true; break; }
                move();
            }
            add(closed ? "comment" : "invalid", start, row, column);
            if (!closed) error(row, column, "unterminated block comment");
        } else if (c=='"' || c=='\'') {
            char quote = c; move(); bool closed=false; int count=0;
            while (i<src.size() && src[i]!='\n' && src[i]!='\r') {
                if (src[i]==quote) { move(); closed=true; break; }
                if (src[i]=='\\') { move(); if (i>=src.size() || src[i]=='\n' || src[i]=='\r') break; }
                move(); count++;
            }
            bool ok = closed && (quote=='"' || count==1);
            add(ok ? (quote=='"' ? "string" : "char") : "invalid", start, row, column);
            if (!ok) error(row, column, closed ? "character literal must contain one character" : "unterminated literal");
        } else {
            string two = src.substr(i, 2);
            if (two=="<=" || two==">=" || two=="==" || two=="!=" || two=="&&" || two=="||" || two=="++" || two=="--") {
                move(); move(); add("operator", start, row, column);
            } else if (string("+-*/%^<>=!").find(c)!=string::npos) {
                move(); add("operator", start, row, column);
            } else if (string("(){}[],:;").find(c)!=string::npos) {
                move(); add("delimiter", start, row, column);
            } else {
                move(); add("invalid", start, row, column); error(row, column, "unknown character");
            }
        }
    }
    result.tokens.push_back({"eof", "", file, line, col, src.size(), src.size()});
    return result;
}
