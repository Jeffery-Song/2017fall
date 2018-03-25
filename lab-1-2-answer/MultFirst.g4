grammar MultFirst;


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
    exp Multiply exp # Mult
    | exp Plus  exp # Plus
    | LeftParen exp RightParen # Brac
    | Identifier # Id
    | Number # Num
;