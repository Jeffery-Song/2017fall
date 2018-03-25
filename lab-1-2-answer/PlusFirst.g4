grammar PlusFirst;


// lexer

LeftParen: '(' ;
RightParen: ')' ;

Plus: '+' ;
Multiply: '*' ;

Number: [0-9]+ | '0x' [0-9a-fA-F]+ ;
Identifier: [_a-zA-Z][_0-9a-zA-Z]* ;

WhiteSpace: [ \t\r\n]+ -> skip;


// parser

exp:
    exp Plus exp # Plus
    | exp Multiply exp # Mult
    | LeftParen exp RightParen # Brac
    | Identifier # Id
    | Number # Num
;