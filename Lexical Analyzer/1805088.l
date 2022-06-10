%option noyywrap

%{
    #include"LexicalAnalyzer.h"

    //Define KEYWORDS
    #define IF_TOKEN "IF"
    #define ELSE_TOKEN "ELSE"
    #define SWITCH_TOKEN "SWITCH"
    #define CASE_TOKEN "CASE"
    #define FOR_TOKEN "FOR"
    #define DO_TOKEN "D0"
    #define WHILE_TOKEN "WHILE"
    #define INT_TOKEN "INT"
    #define FLOAT_TOKEN "FLOAT"
    #define CHAR_TOKEN "CHAR"
    #define DOUBLE_TOKEN "DOUBLE"
    #define VOID_TOKEN "VOID"
    #define DEFAULT_TOKEN "DEFAULT"
    #define BREAK_TOKEN "BREAK"
    #define RETURN_TOKEN "RETURN"
    #define CONTINUE_TOKEN "CONTINUE"
%}

WHITESPACE [ \t\f\r\v]+
LETTER [a-zA-Z]
NEWLINE [\r?\n]
DIGIT [0-9]

%%
{WHITESPACE}+ {}
{NEWLINE} {line_count++;}
{DIGIT}+ {cout << "Got a number" << endl;}
{LETTER}+ {cout << "Got a word" << endl;}
<<EOF>> {
    cout << "\nTotal line number: " << line_count << "\nTotal word number: " << word_count << endl;
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

    yyin = fin;
    yylex();
    fclose(yyin);
    return 0;
}

