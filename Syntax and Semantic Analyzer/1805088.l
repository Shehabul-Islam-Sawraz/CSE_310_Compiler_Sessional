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
    #define DECOP_TOKEN "DECOP"
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
INCOP ("++")
DECOP ("--")
RELOP ("<"|"<="|">"|">="|"=="|"!=")
BITOP ("&"|"|"|"^"|"<<"|">>")
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

{INTEGER} {addConstInt(CONST_INT_TOKEN); return CONST_INT;}
{FLOATING_POINT} {addConstFloat(CONST_FLOAT_TOKEN); return CONST_FLOAT;}
{CHARACTER} {addConstChar(CONST_CHAR_TOKEN); return CONST_CHAR;}

{STRING_START} {
    BEGIN HANDLE_STRING;
    str.append(yytext);
    original_str.append(yytext);
    string_line_no = line_count;
}
<HANDLE_STRING>\" {
    str.append(yytext);
    original_str.append(yytext);
    str = "";
    original_str="";
    addString(STRING_TOKEN, original_str);
    BEGIN INITIAL;
    return STRING;
}
<HANDLE_STRING><<EOF>> {
    fprintf(errorout,"LEXICAL Error at ine no %d: %s  < %s > \n",string_line_no+1, "Unfinished String", str.data());
    error_count++;
    str = "";
    original_str="";
    BEGIN INITIAL;
    return 0;
}
<HANDLE_STRING>{NEWLINE} {
    fprintf(errorout,"LEXICAL Error at ine no %d: %s  < %s >\n",string_line_no, "Unfinished String", str.data());
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


"if" 		{return IF;}
"else" 		{return ELSE;}
"switch" 	{return SWITCH;}
"case" 		{return CASE;}
"for" 		{return FOR;}
"do" 		{return DO;}
"while" 	{return WHILE;}
"int" 		{return INT;}
"float" 	{return FLOAT;}
"char" 		{return CHAR;}
"double" 	{return DOUBLE;}
"void" 		{return VOID;}
"default" 	{return DEFAULT;}
"break" 	{return BREAK;}
"return"	{return RETURN;}
"continue" 	{return CONTINUE;}

{ADDOP} {addOperators(ADDOP_TOKEN); return ADDOP;}
{MULOP} {addOperators(MULOP_TOKEN); return MULOP;}
{INCOP} {addOperators(INCOP_TOKEN); return INCOP;}
{DECOP} {addOperators(DECOP_TOKEN); return DECOP;}
{RELOP} {addOperators(RELOP_TOKEN); return RELOP;}
{ASSIGNOP} {addOperators(ASSIGNOP_TOKEN); return ASSIGNOP;}
{LOGICOP} {addOperators(LOGICOP_TOKEN); return LOGICOP;}
{BITOP} {addOperators(BITOP_TOKEN); return BITOP;}
{NOT} {addOperators(NOT_TOKEN); return NOT;}
{LPAREN} {addOperators(LPAREN_TOKEN); return LPAREN;}
{RPAREN} {addOperators(RPAREN_TOKEN); return RPAREN;}
{LCURL} {addOperators(LCURL_TOKEN); return LCURL;}
{RCURL} {addOperators(RCURL_TOKEN); return RCURL;}
{LTHIRD} {addOperators(LTHIRD_TOKEN); return LTHIRD;}
{RTHIRD} {addOperators(RTHIRD_TOKEN); return RTHIRD;}
{COMMA} {addOperators(COMMA_TOKEN); return COMMA;}
{SEMICOLON} {addOperators(SEMICOLON_TOKEN); return SEMICOLON;}


{IDENTIFIER} {addIdentifier(IDENTIFIER_TOKEN); return ID;}
{NON_IDENTIFIER} {printErrorLog("Invalid Suffix on numeric constant or invalid prefix on identifier");}

{ANYTHING} {printErrorLog("Unrecognized Character");}

<<EOF>> {
    return 0;
}
%%