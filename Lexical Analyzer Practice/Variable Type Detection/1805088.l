%option noyywrap

%{
    #include<iostream>
    #include<bits/stdc++.h>
    using namespace std;

    FILE *logout;
    int line_count=1;

%}

WHITESPACE [ \t\f\r\v]+
NEWLINE \r?\n
CAPITAL_LETTER [A-Z]
SMALL_LETTER [a-z]
DIGIT [0-9]
DIGITS {DIGIT}+
CAPITAL_ALPHABET {CAPITAL_LETTER}+
SMALL_ALPHABET {SMALL_LETTER}+
CAP_ALPHANUMERIC {CAPITAL_LETTER}|{DIGIT}
SMALL_ALPHANUMERIC {SMALL_LETTER}|{DIGIT}
UNDERSCORE _

SNAKE_CASE ^[a-z]({SMALL_ALPHANUMERIC})*({UNDERSCORE}{SMALL_ALPHANUMERIC}+)*{SMALL_ALPHANUMERIC}$

UNDEFINED_CASE ^[a-zA-Z_][a-z0-9_]*(({DIGIT})|([A-Z][a-z0-9_]+))*([A-Z_])?

CAMEL_CASE ^[a-zA-Z][a-z0-9]*(({DIGIT})|([A-Z][a-z0-9]+))*([A-Z])?
%%

{WHITESPACE}+ {}
{NEWLINE} { 
    line_count++;
}

{SNAKE_CASE} {
     fprintf(logout,"Line no %d: Snake Case Variable \"%s\" found\n",line_count, yytext);
}

{CAMEL_CASE} {
     fprintf(logout,"Line no %d: Camel Case Variable \"%s\" found\n",line_count, yytext);
}

{UNDEFINED_CASE} {
    fprintf(logout,"Line no %d: Undefined Case Variable \"%s\" found\n",line_count, yytext);
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

