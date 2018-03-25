lexer grammar C1Lexer;

tokens {
    Comma,
    SemiColon,
    Assign,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    LeftParen,
    RightParen,
    If,
    Else,
    While,
    Const,
    Equal,
    NonEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,

    Int,
    Void,

    Identifier,
    Number
}

Comma: ',' ;
SemiColon: ';' ;
Assign: '=' ;

LeftBracket: '[' ;
RightBracket: ']' ;

LeftBrace: '{' ;
RightBrace: '}' ;

LeftParen: '(' ;
RightParen: ')' ;

If: 'if' ;
Else: 'else' ;
While: 'while' ;
Const: 'const' ;

Equal: '==' ;
NonEqual: '!=' ;
Less: '<' ;
Greater: '>' ;
LessEqual: '<=' ;
GreaterEqual: '>=' ;

Plus: '+' ;
Minus: '-' ;
Multiply: '*' ;
Divide: '/' ;
Modulo: '%' ;

Int: 'int' ;
Void: 'void' ;

Identifier: [_a-zA-Z][_0-9a-zA-Z]* ;
Number: [0-9]+ | '0x' [0-9a-fA-F]+ ;

Comment: '//' (.*? ~[\\])?? '\r'? '\n' -> skip;
Comment_Block: '/*' .*? '*/' -> skip;

WhiteSpace: [ \t\r\n]+ -> skip;