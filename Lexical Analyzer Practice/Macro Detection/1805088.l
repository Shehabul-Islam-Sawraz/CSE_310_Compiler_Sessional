%option noyywrap

%{

    #include<iostream>
    #include<bits/stdc++.h>
    using namespace std;

    FILE *logout;
    int line_count=1;

%}

ANYTHING_EXCEPT_NEWLINE [^\r\n]
WHITESPACE [ \t\f\r\v]+
LETTER [a-zA-Z]
NEWLINE \r?\n
DIGIT [0-9]
DIGITS {DIGIT}+
INTEGER {DIGITS}

ALPHABET {LETTER}|_
ALPHANUMERIC {ALPHABET}|{DIGIT}
IDENTIFIER {ALPHABET}({ALPHANUMERIC})*

CHAR_CONST ((\\{ANYTHING_EXCEPT_NEWLINE})|[^\\'(\r\n)])
CHARACTER ('{CHAR_CONST}')

FLOATING_POINT ({INTEGER})?(\.?{DIGITS})(E[+-]?{INTEGER})?

STRING_START \"

STRING {STRING_START}{IDENTIFIER}{STRING_START}

DEFINE_START #define

MACRO {DEFINE_START}{WHITESPACE}{IDENTIFIER}{WHITESPACE}({INTEGER}|{STRING}|{FLOATING_POINT}|{CHARACTER})

%%

{WHITESPACE}+ {}
{NEWLINE} {line_count++;}

{MACRO} {
    fprintf(logout,"MACRO %s (MACRO WITH VALUE) DETECTED at line no: %d\n",yytext,line_count);
}

<<EOF>> {
    return 0;
}

%%

int main(int argc, char* argv[]){
    if(argc!=2){
        cout << "Please provide input file name and try again" << endl;
        return 0;
    }
    FILE *fin = fopen(argv[1],"r");
    if(fin==NULL){
        cout << "Can't open specified file!!" << endl;
        return 0;
    }

    logout= fopen("log.txt","w");

    yyin = fin;
    yylex();
    fclose(yyin);
    return 0;
}