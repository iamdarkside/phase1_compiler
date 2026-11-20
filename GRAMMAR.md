 Lexical rules and grammar

 Tokens

```text
identifier = [A-Za-z_][A-Za-z_0-9]*
integer    = [0-9]+
boolean    = true | false
string     = " ( [^"\\\r\n] | \\ [^\r\n] )* "
char       = ' ( [^'\\\r\n] | \\ [^\r\n] ) '
line-comment  = // [^\r\n]*
block-comment = /\* ([^*] | \*+ [^*/])* \*+ /
whitespace = [ \t\r\n]+
operator   = <= | >= | == | != | && | || | ++ | --
           | + | - | * | / | % | ^ | < | > | = | !
delimiter  = ( | ) | { | } | [ | ] | , | : | ;
```

Keywords:

`array boolean char else for function if integer print return string void while`

`true` and `false` are boolean tokens. The whole identifier is read first and then compared with the keywords. For example, `return` is a keyword, but `returnValue` is an identifier.

To apply the longest match rule, two-character operators like `<=` are checked before single-character operators. Comments are also checked before the `/` operator. The minus sign is separate from the number.

A block comment continues until the first `*/`. An unclosed string or character literal, or a block comment without `*/`, is an error. An unknown character becomes an `invalid` token, and analysis continues.

 EBNF grammar


```ebnf
program      = { declaration }, EOF ;
declaration  = identifier, ":", type, [ "=", initializer ], ";"
             | identifier, ":", function-type, "=", block ;
type         = "integer" | "boolean" | "char" | "string" | "void"
             | "array", "[", [ expression ], "]", type
             | function-type ;
function-type = "function", type, "(", [ parameter-list ], ")" ;
parameter-list = parameter, { ",", parameter } ;
parameter    = identifier, ":", type ;
initializer  = expression | "{", [ initializer, { ",", initializer } ], "}" ;
block        = "{", { statement }, "}" ;
statement    = declaration | block
             | "if", "(", expression, ")", statement, [ "else", statement ]
             | "for", "(", [ expression ], ";", [ expression ], ";", [ expression ], ")", statement
             | "while", "(", expression, ")", statement
             | "return", [ expression ], ";"
             | "print", [ expression, { ",", expression } ], ";"
             | [ expression ], ";" ;
expression   = assignment ;
assignment   = logical-or, [ "=", assignment ] ;
logical-or   = logical-and, { "||", logical-and } ;
logical-and  = comparison, { "&&", comparison } ;
comparison   = additive, { ( "<" | "<=" | ">" | ">=" | "==" | "!=" ), additive } ;
additive     = multiply, { ( "+" | "-" ), multiply } ;
multiply     = power, { ( "*" | "/" | "%" ), power } ;
power        = unary, [ "^", power ] ;
unary        = ( "-" | "!" ), unary | postfix ;
postfix      = primary, { "(", [ arguments ], ")" | "[", expression, "]" | "++" | "--" } ;
arguments    = expression, { ",", expression } ;
primary      = identifier | integer | boolean | string | char | "(", expression, ")" ;
```



The parser uses recursive descent and handles expressions according to operator precedence. The expression grammar has no left recursion. The `=` and `^` operators are right-associative, and the other binary operators are left-associative.

Inside a block, the parser looks for the two tokens `identifier :` to recognize a declaration. A declaration starts with an `identifier`, and a type starts with `array`, `function`, or a basic type. The optional part of `return` ends with `;`, and the optional parts of `for` end with `;` or `)`, so they are distinct from the start of an expression. Each `else` belongs to the nearest `if`, so the raw if/else grammar is not considered LL(1). An expression starts with an identifier, a literal, `(`, `-`, or `!`; expression endings include `;`, `,`, `)`, and `]`. On an error, the parser moves to `;`, `}`, or the beginning of the next statement or declaration and continues.
