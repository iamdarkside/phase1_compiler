#include "common.h"
#include <set>
#include <unordered_set>
using namespace std;

static void functions(const Tree &n, unordered_set<size_t> &ids) {
    if (!n) return;
    if ((n->kind=="FuncDecl" || n->kind=="CallExpr") && !n->children.empty() && n->children[0]->kind=="Identifier") ids.insert(n->children[0]->token);
    for (const auto &child:n->children) functions(child,ids);
}
static string escape(const string &s) {
    string r;
    for (char c:s) {
        if (c=='&') r+="&amp;";
        else if (c=='<') r+="&lt;";
        else if (c=='>') r+="&gt;";
        else if (c=='\"') r+="&quot;";
        else r+=c;
    }
    return r;
}
void highlight(const string &src, const vector<Token> &tokens, const Tree &root, const vector<size_t> &bad, bool html, ostream &out) {
    unordered_set<size_t> ids; functions(root,ids);
    unordered_set<size_t> mistakes(bad.begin(),bad.end());
    const set<string> types={"integer","boolean","char","string","void","array"};
    if (html) out << "<!doctype html><html><head><meta charset=\"utf-8\"><title>B-Minor</title><style>body{background:#1e1e1e;color:#eee}pre{white-space:pre;tab-size:4}.kw{color:#569cd6;font-weight:bold}.type{color:#4ec9b0}.fn{color:#dcdcaa}.num{color:#e6a564}.str{color:#b5cea8}.op{color:#ccc}.comment{color:#888;font-style:italic}.invalid{color:#f44;text-decoration:underline}</style></head><body><pre>";
    size_t last=0;
    for (size_t i=0;i<tokens.size();i++) {
        const auto &t=tokens[i]; if (t.kind=="eof") break;
        string gap=src.substr(last,t.start-last); out << (html?escape(gap):gap);
        string cl, color;
        if (mistakes.count(i) || t.kind=="invalid") { cl="invalid"; color="\033[4;31m"; }
        else if (ids.count(i)) { cl="fn"; color="\033[33m"; }
        else if (t.kind=="keyword" && types.count(t.text)) { cl="type"; color="\033[36m"; }
        else if (t.kind=="keyword") { cl="kw"; color="\033[1;34m"; }
        else if (t.kind=="integer" || t.kind=="boolean") { cl="num"; color="\033[38;5;208m"; }
        else if (t.kind=="string" || t.kind=="char") { cl="str"; color="\033[32m"; }
        else if (t.kind=="operator") { cl="op"; color="\033[37m"; }
        else if (t.kind=="comment") { cl="comment"; color="\033[2;3m"; }
        if (html) {
            if (!cl.empty()) out << "<span class=\"" << cl << "\">";
            out << escape(t.text);
            if (!cl.empty()) out << "</span>";
        } else {
            if (!color.empty()) out << color;
            out << t.text;
            if (!color.empty()) out << "\033[0m";
        }
        last=t.end;
    }
    string rest=src.substr(last); out << (html?escape(rest):rest);
    if (html) out << "</pre></body></html>\n";
}
