
This program reads B-Minor code, creates tokens and an AST, and displays the code in color. The project is written in C++.

 Run

```sh
make
./bminor --tokens tests/valid.bminor
./bminor --ast tests/valid.bminor
./bminor --ansi tests/valid.bminor
./bminor --html tests/valid.bminor result.html
make test
```

 Files

- `src/lexer.cpp`: Creates tokens
- `src/parser.cpp`: Parses tokens and creates the AST
- `src/highlight.cpp`: Highlights code using the AST
- `GRAMMAR.md`: Token rules and grammar
- `tests`: Tests and sample inputs

