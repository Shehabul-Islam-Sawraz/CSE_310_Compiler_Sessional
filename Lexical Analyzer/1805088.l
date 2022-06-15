%option noyywrap

%x HANDLE_STRING

%{
    #include"LexicalAnalyzer.h"

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
    #define COMMENT_TOKEN "COMMENT"
%}

ANYTHING_EXCEPT_NEWLINE [^\r\n]

WHITESPACE [ \t\f\r\v]+
LETTER [a-zA-Z]
NEWLINE \r?\n
DIGIT [0-9]
DIGITS {DIGIT}+
INTEGER {DIGITS}

MUL_DEC_POINTS (\.)*
TOO_MANY_DECIMAL {DIGIT}*((\.)+{DIGITS})+(E{MUL_DEC_POINTS}[+-]?{MUL_DEC_POINTS}{DIGITS})?{MUL_DEC_POINTS}{DIGIT}*

ILL_NUM ({DIGITS}|{FLOATING_POINT})[E]*{FLOATING_POINT}*

ALPHABET {LETTER}|_
ALPHANUMERIC {ALPHABET}|{DIGIT}
IDENTIFIER {ALPHABET}({ALPHANUMERIC})*
NON_IDENTIFIER {DIGITS}({ALPHANUMERIC})*

FLOATING_POINT ({INTEGER})?(\.?{DIGITS})(E[+-]?{INTEGER})?

CHAR_CONST ((\\{ANYTHING_EXCEPT_NEWLINE})|[^\\'(\r\n)])
CHARACTER ('{CHAR_CONST}')
EMPTY_CHARACTER ('')
MULTI_CHARACTER ('{CHAR_CONST}{2,}')
UNFINISHED_CHARACTER '({CHAR_CONST}|\\)*

ANYTHING ({ANYTHING_EXCEPT_NEWLINE}|{NEWLINE})

STRING_START \"
SPECIAL_STRING_CHARACTERS (\\[\\nt\'afrbv0\"])

MULTILINE_COMMENT_START "/*"
MULTILINE_COMMENT_END "*/"
INCOMPLETE_MULTILINE_COMMENT {MULTILINE_COMMENT_START}([^\*]|([\*]+[^*/]))*
MULTILINE_COMMENT {INCOMPLETE_MULTILINE_COMMENT}[\*]*{MULTILINE_COMMENT_END}

SINGLELINE_COMMENT_START "//"
SLASH [\\]
SINGLELINE_COMMENT {SINGLELINE_COMMENT_START}({ANYTHING_EXCEPT_NEWLINE}*({SLASH}({NEWLINE}))?{ANYTHING_EXCEPT_NEWLINE}*)*

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
    fprintf(tokenout, " <%s, %s> ", "STRING", str.data());
    fprintf(logout,"Line no %d: TOKEN <%s> Lexeme %s found --> < %s, %s >\n",string_line_no, "STRING", original_str.data(), "STRING", str.data());
    str = "";
    original_str="";
    BEGIN INITIAL;
}
<HANDLE_STRING><<EOF>> {
    fprintf(logout,"Error at ine no %d: %s  %s\n",string_line_no+1, "Unfinished String", str.data());
    error_count++;
    str = "";
    original_str="";
    BEGIN INITIAL;
    return 0;
}
<HANDLE_STRING>{NEWLINE} {
    fprintf(logout,"Error at ine no %d: %s  %s\n",string_line_no, "Unfinished String", str.data());
    error_count++;
    line_count++;
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

{SINGLELINE_COMMENT} {addComment(COMMENT_TOKEN);}
{MULTILINE_COMMENT} {addComment(COMMENT_TOKEN);}
{INCOMPLETE_MULTILINE_COMMENT} {
    printErrorLog("Unfinished comment");
    clearNewline();
}

{TOO_MANY_DECIMAL} {printErrorLog("Too Many Decimal Points");}
{ILL_NUM} {printErrorLog("Ill Formed Number");}
{MULTI_CHARACTER} {printErrorLog("Multi Character Constant Error");}
{UNFINISHED_CHARACTER} {printErrorLog("Unfinished Character");}
{EMPTY_CHARACTER} {printErrorLog("Empty Character Constant Error");}


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
{NON_IDENTIFIER} {printErrorLog("Invalid Suffix on numeric constant or invalid prefix on identifier");}

{ANYTHING} {printErrorLog("Unrecognized Character");}

<<EOF>> {
    line_count--;
    symbolTable->printAllScope();
    fprintf(logout,"\nTotal Lines: %d\nTotal Errors: %d\n",line_count, error_count);
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

