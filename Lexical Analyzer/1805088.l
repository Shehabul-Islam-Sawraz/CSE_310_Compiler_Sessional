%option noyywrap

%x HANDLE_STRING

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

    //Define  CONSTANTS
    #define CONST_INT_TOKEN "CONST_INT"
    #define CONST_FLOAT_TOKEN "CONST_FLOAT"
    #define CONST_CHAR_TOKEN "CONST_CHAR"
    #define IDENTIFIER_TOKEN "ID"

    //Define Operators and Punctuators
    #define ADDOP_TOKEN "ADDOP"
    #define MULOP_TOKEN "MULOP"
    #define INCOP_TOKEN "INCOP"
    #define RELOP_TOKEN "RELOP"
    #define ASSIGNOP_TOKEN "ASSIGNOP"
    #define LOGICOP_TOKEN "LOGICOP"
    #define BITOP_TOKEN "BITOP"
    #define NOT_TOKEN "NOT"
    #define LPAREN_TOKEN "LPAREN"
    #define RPAREN_TOKEN "RPAREN"
    #define LCURL_TOKEN "LCURL"
    #define RCURL_TOKEN "RCURL"
    #define LTHIRD_TOKEN "LTHIRD"
    #define RTHIRD_TOKEN "RTHIRD"
    #define COMMA_TOKEN "COMMA"
    #define SEMICOLON_TOKEN "SEMICOLON"

    #define STRING_TOKEN "STRING"
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

STRING_START \"
SPECIAL_STRING_CHARACTERS (\\[\\nt\'afrbv0\"])

ADDOP [+-]
MULOP [*/%]
INCOP ("++"|"--")
RELOP ("<"|"<="|">"|">="|"=="|"!=")
ASSIGNOP [=]
LOGICOP ("&&"|"||")
NOT [!]
LPAREN [(]
RPAREN [)]
LCURL [{]
RCURL [}]
LTHIRD ("[")
RTHIRD("]")
COMMA [,]
SEMICOLON [;]

%%
{WHITESPACE}+ {}
{NEWLINE} {line_count++;}

{INTEGER} {addConstInt(CONST_INT_TOKEN);}
{FLOATING_POINT} {addConstFloat(CONST_FLOAT_TOKEN);}
{CHARACTER} {addConstChar(CONST_CHAR_TOKEN);}

{STRING_START} {
    BEGIN HANDLE_STRING;
    str.append(yytext);
    original_str.append(yytext);
    string_line_no = line_count;
}
<HANDLE_STRING>\" {
    str.append(yytext);
    original_str.append(yytext);
    yytext = str;
    addString(STRING_TOKEN, original_str);
    str = "";
    original_str="";
    BEGIN INITIAL;
}
<HANDLE_STRING>((\\){NEWLINE}) {
    line_count++;
    original_str.append(yytext);
}
<HANDLE_STRING>{SPECIAL_STRING_CHARACTERS} {
    handleSpecialStringCharacters();
}

<HANDLE_STRING>. {
    str.append(yytext);
    original_str.append(yytext);
}

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

{ADDOP} {addOperators(ADDOP_TOKEN);}
{MULOP} {addOperators(MULOP_TOKEN);}
{INCOP} {addOperators(INCOP_TOKEN);}
{RELOP} {addOperators(RELOP_TOKEN);}
{ASSIGNOP} {addOperators(ASSIGNOP_TOKEN);}
{LOGICOP} {addOperators(LOGICOP_TOKEN);}
{NOT} {addOperators(NOT_TOKEN);}
{LPAREN} {addOperators(LPAREN_TOKEN);}
{RPAREN} {addOperators(RPAREN_TOKEN);}
{LCURL} {addOperators(LCURL_TOKEN);}
{RCURL} {addOperators(RCURL_TOKEN);}
{LTHIRD} {addOperators(LTHIRD_TOKEN);}
{RTHIRD} {addOperators(RTHIRD_TOKEN);}
{COMMA} {addOperators(COMMA_TOKEN);}
{SEMICOLON} {addOperators(SEMICOLON_TOKEN);}


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

