%{
    #include "SyntaxAnalyzer.h"
%}

%union{
    SymbolInfo* symbolInfo;
}

%token IF ELSE SWITCH CASE DEFAULT FOR DO WHILE INT FLOAT DOUBLE CHAR STRING VOID BREAK RETURN CONTINUE
%token INCOP DECOP ASSIGNOP NOT PRINTLN
%token LPAREN RPAREN LCURL RCURL LTHIRD RTHIRD COMMA SEMICOLON

%token <symbolInfo>ID CONST_INT CONST_FLOAT CONST_CHAR ADDOP MULOP LOGICOP RELOP BITOP

%type <symbolInfo>type_specifier expression logic_expression rel_expression simple_expression term unary_expression factor variable
%type<vectorsymbol> declaration_list var_declaration func_declaration parameter_list unit expression_statement arguments argument_list statement statements compound_statement func_definition program 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%error-verbose

%%
type_specifier : INT
                        {
                            $$ = getSymbolInfoOfType(INT_TYPE);
                        }
                | FLOAT
                        {
                            $$ = getSymbolInfoOfType(FLOAT_TYPE);
                        }
                | VOID
                        {
                            $$ = getSymbolInfoOfType(VOID_TYPE);
                        }
                ;
%%