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
%type <vectorsymbol>declaration_list var_declaration func_declaration parameter_list unit expression_statement arguments argument_list statement statements compound_statement func_definition program 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%error-verbose

%%
start: program
                        {
                                setValue(start,popValue(program));
                                printRuleAndCode(start,"program");
                        }
                ;

program: program unit
                |   unit
                        {
                            setValue(program,popValue(unit));
                            printRuleAndCode(program,"unit");
                        }
                ;

unit:   var_declaration
                        {
                            setValue(unit,popValue(var_declaration));
                            printRuleAndCode(unit,"var_declaration");
                        }
                |   func_declaration
                |   func_definition
                ;

var_declaration: type_specifier declaration_list SEMICOLON
                        {
                            setValue(var_declaration, popValue(type_specifier)+" "+popValue(declaration_list)+ ";");
                            printRuleAndCode(var_declaration,"type_specifier declaration_list SEMICOLON");
                        }
                ;

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

declaration_list : declaration_list COMMA ID
                        {
                            insertVar($3);
                            setValue(declaration_list, popValue(declaration_list)+","+$3->getName());
                            printRuleAndCode(declaration_list,"declaration_list COMMA ID");
                        }
                |   declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
                |   ID
                        {
                            insertVar($1);
                            setValue(declaration_list,$1->getName());
                            printRuleAndCode(declaration_list,"ID");
                        }
                |   ID LTHIRD CONST_INT RTHIRD
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

	logout= fopen("log.txt","w");
	errorout= fopen("error.txt","w");
	parserout= fopen("parser.txt","w");

    yyin = inputFile;
	yyparse();
    line_count--;
    fprintf(logout,"\nTotal Lines: %d\nTotal Errors: %d\n",line_count, error_count);

    fclose(yyin);
	fclose(logout);
    fclose(errorout);
    fclose(parserout);

	return 0;
}