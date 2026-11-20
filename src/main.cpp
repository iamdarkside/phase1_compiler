#include "common.h"
#include <fstream>
#include <sstream>
using namespace std;
int main(int argc, char **argv) {
    if (argc<3 || argc>4) {
        cerr << "Usage: " << argv[0] << " --tokens|--ast|--ansi|--html input [output.html]\n"; return 2;
    }
    string mode=argv[1], file=argv[2];
    if (mode!="--tokens" && mode!="--ast" && mode!="--ansi" && mode!="--html") { cerr << "Unknown mode\n"; return 2; }
    if (argc==4 && mode!="--html") { cerr << "An output file is supported only for --html\n"; return 2; }
    ifstream in(file,ios::binary);
    if (!in) { cerr << "Cannot open " << file << "\n"; return 2; }
    stringstream buffer; buffer << in.rdbuf(); string src=buffer.str();
    auto result=lex(src,file);
    Tree root;
    if (mode!="--tokens") root=parse(result.tokens,file,result.errors,result.bad);
    if (mode=="--tokens") {
        for (auto &t:result.tokens) if (t.kind!="eof") cout << t.file << ":" << t.line << ":" << t.col << " " << t.kind << " " << t.text << "\n";
    } else if (mode=="--ast") show_tree(root);
    else if (mode=="--html" && argc==4) {
        ofstream out(argv[3],ios::binary); if (!out) { cerr << "Cannot open output\n"; return 2; }
        highlight(src,result.tokens,root,result.bad,true,out);
    } else highlight(src,result.tokens,root,result.bad,mode=="--html",cout);
    for (auto &e:result.errors) cerr << e << '\n';
    return result.errors.empty()?0:1;
}
