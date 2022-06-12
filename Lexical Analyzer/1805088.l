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

    #define CONST_INT_TOKEN "CONST_INT"
    #define CONST_FLOAT_TOKEN "CONST_FLOAT"
    #define CONST_CHAR_TOKEN "CONST_CHAR"
    #define IDENTIFIER_TOKEN "ID"
%}

ANYTHING_EXCEPT_NEWLINE [^\r\n]

WHITESPACE [ \t\f\r\v]+
LETTER [a-zA-Z]
NEWLINE [\r?\n]
DIGIT [0-9]
DIGITS {DIGIT}+
INTEGER {DIGITS}
ALPHABET {LETTER}|_
ALPHANUMERIC {ALPHABET}|{DIGIT}
IDENTIFIER {ALPHABET}({ALPHANUMERIC})*
FLOATING_POINT ({INTEGER})?(\.?{DIGITS})(E[+-]?{INTEGER})?

CHAR_CONST ((\\{ANYTHING_EXCEPT_NEWLINE})|[^\\'(\r\n)])
CHARACTER ('{CHAR_CONST}')

%%
{WHITESPACE}+ {}
{NEWLINE} {line_count++;}

{INTEGER} {addConstInt(CONST_INT_TOKEN);}
{FLOATING_POINT} {addConstFloat(CONST_FLOAT_TOKEN);}
{CHARACTER} {addConstChar(CONST_CHAR_TOKEN);}


"if" 		|
"else" 		|
"switch" 	|
"case" 		|
"for" 		|
"do" 		|
"while" 	|
"int" 		|
"float" 	|
"char" 		|
"double" 	|
"void" 		|
"default" 	|
"break" 	|
"return"	|
"continue" 	{addKeywords();}

{IDENTIFIER} {addIdentifier(IDENTIFIER_TOKEN);}

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

    logout= fopen("log.txt","w");
	tokenout= fopen("token.txt","w");

    yyin = fin;
    yylex();
    fclose(yyin);
    return 0;
}

