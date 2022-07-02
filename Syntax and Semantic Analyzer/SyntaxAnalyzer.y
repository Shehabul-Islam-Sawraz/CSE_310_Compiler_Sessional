%{
    #include "SyntaxAnalyzer.h"
%}

%union{
    SymbolInfo* symbolInfo;
    vector<SymbolInfo*>* vectorsymbol;
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
                            setValue(type_specifier,"int");
                            printRuleAndCode(type_specifier,"INT");
                        }
                | FLOAT
                        {
                            $$ = getSymbolInfoOfType(FLOAT_TYPE);
                            setValue(type_specifier,"float");
                            printRuleAndCode(type_specifier,"FLOAT");
                        }
                | VOID
                        {
                            $$ = getSymbolInfoOfType(VOID_TYPE);
                            setValue(type_specifier,"void");
                            printRuleAndCode(type_specifier,"VOID");
                        }
                ;
%%

int main(int argc,char *argv[])
{
    if(argc!=2){
		cout << "Please provide input file name" << endl;
		return 0;
	}
	
	FILE *inputFile=fopen(argv[1],"r");
	if(inputFile==NULL){
		cout << "Cannot open input file\n";
		return 0;
	}

	logout.open("log.txt");
	errorout.open("error.txt");
	parserout.open("parser.txt");

    yyin = inputFile;
	yyparse();
    line_count--;
    fprintf(logout,"\nTotal Lines: %d\nTotal Errors: %d\n",line_count, error_count);

    fclose(yyin);
	logout.close();
	errorout.close();
    parserout.close();

	return 0;
}