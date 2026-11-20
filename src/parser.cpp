#include "common.h"
#include <stdexcept>
using namespace std;

class Parser {
    const vector<Token> &all;
    vector<size_t> ids;
    size_t pos = 0;
    string file;
    vector<string> &errors;
    vector<size_t> &bad;
    const Token &cur() const { return all[ids[pos]]; }
    const Token &peek(size_t n) const { return all[ids[min(pos+n, ids.size()-1)]]; }
    bool at(const string &s) const { return cur().text==s; }
    bool take(const string &s) { if (!at(s)) return false; pos++; return true; }
    [[noreturn]] void fail(const string &message) {
        if (cur().kind!="eof") bad.push_back(ids[pos]);
        errors.push_back(file + ":" + to_string(cur().line) + ":" + to_string(cur().col) + ": " + message + (cur().kind=="eof" ? " at end of file" : " near '" + cur().text + "'"));
        throw runtime_error("parse error");
    }
    void need(const string &s) { if (!take(s)) fail("expected '" + s + "'"); }
    Tree node(const string &kind, size_t n, const string &value = "") {
        auto t = ids[n];
        return make_shared<Node>(Node{kind, value, "", all[t].line, all[t].col, t, {}});
    }
    Tree name() {
        if (cur().kind!="identifier") fail("expected identifier");
        auto n=node("Identifier",pos,cur().text); pos++; return n;
    }
    void recover(size_t before, bool inBlock) {
        if (pos==before && cur().kind!="eof" && !(inBlock && at("}"))) pos++;
        while (cur().kind!="eof") {
            if (at(";")) { pos++; return; }
            if (at("}")) { if (!inBlock) pos++; return; }
            if (at("if") || at("return") || at("for") || at("while") || at("print") || (cur().kind=="identifier" && peek(1).text==":")) return;
            pos++;
        }
    }
    Tree type() {
        size_t start=pos;
        if (take("array")) {
            auto n=node("ArrayType",start); need("[");
            if (!at("]")) n->children.push_back(expr());
            need("]"); n->children.push_back(type()); return n;
        }
        if (take("function")) {
            auto n=node("FunctionType",start); n->children.push_back(type()); need("(");
            if (!at(")")) do {
                size_t p=pos; auto a=node("Parameter",p); a->children.push_back(name()); need(":"); a->children.push_back(type()); n->children.push_back(a);
            } while (take(","));
            need(")"); return n;
        }
        if (at("integer") || at("boolean") || at("char") || at("string") || at("void")) {
            auto n=node("Type",pos,cur().text); pos++; return n;
        }
        fail("expected type");
    }
    Tree decl() {
        size_t start=pos; auto id=name(); need(":"); auto t=type();
        bool fn=t->kind=="FunctionType";
        auto n=node(fn ? "FuncDecl" : "VarDecl",start); n->children={id,t};
        if (take("=")) {
            if (fn && at("{")) n->children.push_back(block());
            else n->children.push_back(initializer());
            if (fn && n->children.back()->kind=="Block") return n;
        }
        need(";"); return n;
    }
    Tree initializer() {
        if (!at("{")) return expr();
        size_t start=pos; need("{"); auto n=node("ArrayLiteral",start);
        if (!at("}")) do { n->children.push_back(initializer()); } while (take(","));
        need("}"); return n;
    }
    Tree block() {
        size_t start=pos; need("{"); auto n=node("Block",start);
        while (!at("}") && cur().kind!="eof") {
            size_t old=pos;
            try { n->children.push_back(stmt()); } catch (const runtime_error &) { recover(old,true); }
        }
        need("}"); return n;
    }
    Tree stmt() {
        size_t start=pos;
        if (at("{")) return block();
        if (cur().kind=="identifier" && peek(1).text==":") return decl();
        if (take("if")) {
            auto n=node("IfStmt",start); need("("); n->children.push_back(expr()); need(")"); n->children.push_back(stmt());
            if (take("else")) n->children.push_back(stmt());
            return n;
        }
        if (take("for")) {
            auto n=node("ForStmt",start); need("(");
            if (!at(";")) n->children.push_back(expr()); else n->children.push_back(node("Empty",pos)); need(";");
            if (!at(";")) n->children.push_back(expr()); else n->children.push_back(node("Empty",pos)); need(";");
            if (!at(")")) n->children.push_back(expr()); else n->children.push_back(node("Empty",pos)); need(")");
            n->children.push_back(stmt()); return n;
        }
        if (take("while")) { auto n=node("WhileStmt",start); need("("); n->children.push_back(expr()); need(")"); n->children.push_back(stmt()); return n; }
        if (take("return")) { auto n=node("ReturnStmt",start); if (!at(";")) n->children.push_back(expr()); need(";"); return n; }
        if (take("print")) {
            auto n=node("PrintStmt",start); if (!at(";")) do { n->children.push_back(expr()); } while (take(",")); need(";"); return n;
        }
        auto n=node("ExprStmt",start); if (!at(";")) n->children.push_back(expr()); need(";"); return n;
    }
    int priority(const string &s) {
        if (s=="=") return 1;
        if (s=="||") return 2;
        if (s=="&&") return 3;
        if (s=="==" || s=="!=" || s=="<" || s=="<=" || s==">" || s==">=") return 4;
        if (s=="+" || s=="-") return 5;
        if (s=="*" || s=="/" || s=="%") return 6;
        if (s=="^") return 7;
        return 0;
    }
    Tree expr(int minPriority=1) {
        size_t start=pos; Tree left;
        if (at("-") || at("!")) {
            string op=cur().text; pos++; left=node("UnaryExpr",start,op); left->children.push_back(expr(8));
        } else if (take("(")) { left=expr(); need(")"); }
        else if (cur().kind=="identifier") left=name();
        else if (cur().kind=="integer" || cur().kind=="string" || cur().kind=="char" || cur().kind=="boolean") {
            left=node("Literal",pos,cur().text); pos++;
        } else fail("expected expression");
        while (true) {
            if (take("(")) {
                auto n=node("CallExpr",start); n->children.push_back(left);
                if (!at(")")) do { n->children.push_back(expr()); } while (take(","));
                need(")"); left=n;
            } else if (take("[")) {
                auto n=node("IndexExpr",start); n->children={left,expr()}; need("]"); left=n;
            } else if (at("++") || at("--")) {
                string op=cur().text; pos++; auto n=node("PostfixExpr",start,op); n->children.push_back(left); left=n;
            } else break;
        }
        while (true) {
            string op=cur().text; int p=priority(op); if (p<minPriority || p==0) break;
            pos++; auto n=node(op=="=" ? "AssignExpr" : "BinaryExpr",start,op); n->children={left,expr(p+(op=="=" || op=="^" ? 0 : 1))}; left=n;
        }
        return left;
    }
public:
    Parser(const vector<Token> &t, const string &f, vector<string> &e, vector<size_t> &b):all(t),file(f),errors(e),bad(b) {
        for (size_t i=0;i<t.size();i++) if (t[i].kind!="comment" && t[i].kind!="invalid") ids.push_back(i);
    }
    Tree run() {
        auto root=node("Program",0);
        while (cur().kind!="eof") {
            size_t old=pos;
            try { root->children.push_back(decl()); } catch (const runtime_error &) { recover(old,false); }
        }
        return root;
    }
};
Tree parse(const vector<Token> &tokens, const string &file, vector<string> &errors, vector<size_t> &bad) { return Parser(tokens,file,errors,bad).run(); }
void show_tree(const Tree &n, int depth) {
    if (!n) return;
    cout << string(depth*2,' ') << n->kind;
    if (!n->value.empty()) cout << " " << n->value;
    cout << " (" << n->line << ":" << n->col << ")\n";
    for (auto &child:n->children) show_tree(child,depth+1);
}
