%{
    #include "SyntaxAnalyzer.h"
%}

%define parse.error verbose

%union{
    SymbolInfo* symbolInfo;
}

%token IF ELSE SWITCH CASE DEFAULT FOR DO WHILE INT FLOAT DOUBLE CHAR STRING VOID BREAK RETURN CONTINUE
%token INCOP DECOP ASSIGNOP NOT PRINTLN
%token LPAREN RPAREN LCURL RCURL LTHIRD RTHIRD COMMA SEMICOLON

%token <symbolInfo>ID CONST_INT CONST_FLOAT CONST_CHAR ADDOP MULOP LOGICOP RELOP BITOP

%type <symbolInfo>type_specifier expression logic_expression rel_expression simple_expression term unary_expression factor variable program unit var_declaration func_declaration func_definition parameter_list compound_statement declaration_list statements statement expression_statement argument_list arguments

%left COMMA
%right ASSIGNOP
%left LOGICOP
%left RELOP
%left ADDOP
%left MULOP
%left LCURL RCURL
%left LPAREN RPAREN

%right PREFIX_INCOP
%left POSTFIX_INCOP
%right PREFIX_DECOP
%left POSTFIX_DECOP

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
start: program
                        {}
                ;

program: program unit
                                {}
                |       unit
                                {
                                        //$$ = $1;
                                }
                ;

unit:   var_declaration
                                {
                                        //$$ = $1;
                                }
                |       func_declaration
                                {
                                        //$$ = $1;
                                }
                |       func_definition
                                {
                                        //$$ = $1;
                                }
                ;

func_declaration: type_specifier ID LPAREN parameter_list RPAREN {insertFunc($2, $1); startProcedure($2->getName());} SEMICOLON
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ");", "func_declaration");
                                        clearFunctionParam();
                                }       
                |       type_specifier ID LPAREN RPAREN {insertFunc($2, $1); startProcedure($2->getName());} SEMICOLON
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "();", "func_declaration");
                                }
                |       type_specifier ID LPAREN parameter_list RPAREN {insertFunc($2, $1);} error
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")", "func_declaration");
                                        clearFunctionParam();
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {insertFunc($2, $1);} SEMICOLON
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(int x-y);
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ");", "func_declaration");
                                        clearFunctionParam();
                                        //printErrorRecovery("Invalid parameter list",func_declaration,"type_specifier ID LPAREN parameter_list RPAREN SEMICOLON");
                                        printError("Invalid parameter list");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {insertFunc($2, $1);} error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(int x-y)
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")", "func_declaration");
                                        clearFunctionParam();
                                        //printErrorRecovery("; missing. Error in function parameter list",func_declaration,"type_specifier ID LPAREN parameter_list RPAREN");
                                        printError("; missing. Error in function parameter list");
                                }
                |       type_specifier ID LPAREN RPAREN {insertFunc($2, $1);} error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo()
                                        **/
                                        //printErrorRecovery("; missing",func_declaration,"type_specifier ID LPAREN RPAREN");
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()", "func_declaration");
                                        printError("; missing");
                                }
                |       type_specifier ID LPAREN error RPAREN {insertFunc($2, $1);} SEMICOLON
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(-);
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "();", "func_declaration");
                                        //printErrorRecovery("Invalid parameter list",func_declaration,"type_specifier ID LPAREN RPAREN SEMICOLON");
                                        printError("Invalid parameter list");
                                }
                |       type_specifier ID LPAREN error RPAREN {insertFunc($2, $1);} error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(-)
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()", "func_declaration");
                                        //printErrorRecovery("; missing. Error in function parameter list",func_declaration,"type_specifier ID LPAREN RPAREN");
                                        printError("; missing. Error in function parameter list");
                                }
                ;

func_definition: type_specifier ID LPAREN parameter_list RPAREN {addFunctionDef($1, $2); startProcedure($2->getName());} compound_statement
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")" + $7->getName(), "func_definition");
                                        $$ = endFuncDef(true, $2->getName(), $2->getFuncRetType());
                                        // $$ = new SymbolInfo("",TEMPORARY_TYPE);
                                        // offset = offsets.back();
                                        // offsets.pop_back();
                                        // endProcedure($2->getName(), $2->getFuncRetType());
                                        // totalParams = 0;
                                        // currentFunc = nullptr;
                                }
                |       type_specifier ID LPAREN RPAREN {addFunctionDef($1, $2); startProcedure($2->getName());} compound_statement
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()" + $6->getName(), "func_definition");
                                        $$ = endFuncDef(true, $2->getName(), $2->getFuncRetType());
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        /**
                                                To handle cases like :
                                                void foo(int x-y){}
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")" + $8->getName(), "func_definition");
                                        $$ = endFuncDef();
                                }
                |       type_specifier ID LPAREN error RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        /**
                                                To handle cases like :
                                                void foo(-){}
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()" + $7->getName(), "func_definition");
                                        $$ = endFuncDef();
                                }
                ;

parameter_list: parameter_list COMMA type_specifier ID
                                {
                                        //$$ = new SymbolInfo($1->getName() + "," + $3->getName() + " " + $4->getName(), "parameter_list");
                                        insertIntoParamType($4);
                                }
                |       parameter_list COMMA type_specifier
                                {
                                        //$$ = new SymbolInfo($1->getName() + "," + $3->getName(), "parameter_list");
                                        paramType.push_back(variableType);
                                }
                |       type_specifier ID
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName(), "VARIABLE");
                                        insertIntoParamType($2);
                                }
                |       type_specifier
                                {
                                        //$$ = $1;
                                        paramType.push_back(variableType);
                                }
                |       parameter_list error COMMA type_specifier ID
                                {
                                         /**
                                                To handle errors like:
                                                void foo(int x-y,int z){}
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + "," + $4->getName() + " " + $5->getName(), "parameter_list");
                                        insertIntoParamType($5);
                                        //printErrorRecovery("Invalid parameter list",parameter_list,"parameter_list COMMA type_specifier ID");
                                        printError("Invalid parameter list");
                                }
                |       parameter_list error COMMA type_specifier
                                {
                                        /**
                                                To handle cases like:
                                                void foo(int x-y,int);
                                        **/
                                        //$$ = new SymbolInfo($1->getName() + "," + $4->getName(), "parameter_list");
                                        paramType.push_back(variableType);
                                        //printErrorRecovery("Invalid parameter list",parameter_list,"parameter_list COMMA type_specifier");
                                        printError("Invalid parameter list");
                                }
                |       parameter_list COMMA error ID
                                {
                                        //$$ = new SymbolInfo($1->getName() + "," + $4->getName(), "parameter_list");
                                        //printErrorRecovery("Type specifier missing in parameters",parameter_list,"parameter_list COMMA ID");
                                        printError("Type specifier missing in parameters");
                                }
                |       error ID
                                {
                                        //$$ = $2;
                                        //printErrorRecovery("Type specifier missing in parameters",parameter_list,"ID");
                                        printError("Type specifier missing in parameters");
                                }
                ;

compound_statement: LCURL create_scope statements RCURL
                                {
                                        $$ = $3;
                                        exitScope();
                                }
                |       LCURL create_scope RCURL
                                {
                                        exitScope();
                                } 
                |       LCURL create_scope error statements RCURL
                                {
                                        $$ = $4;
                                        exitScope();
                                }     
                |       LCURL create_scope error RCURL
                                {
                                        exitScope();
                                }    
                ;

create_scope:
                                {
                                        createScope();
                                }
                ;
var_declaration: type_specifier declaration_list SEMICOLON
                                {
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + ";", "VARIABLE");
                                }
                |       type_specifier declaration_list error SEMICOLON
                                {
                                        /**
                                                To handle errors like :
                                                int x-y;
                                                int x[10]-y;
                                                int x[10.5]-y;
                                        **/ 
                                        //printErrorRecovery("Invalid variable declaration",var_declaration,"type_specifier declaration_list SEMICOLON");
                                        //$$ = new SymbolInfo($1->getName() + " " + $2->getName() + ";", "VARIABLE");
                                        printError("Invalid variable declaration");
                                }
                ;

type_specifier : INT
                                {
                                        $$ = getSymbolInfoOfType(INT_TYPE);
                                }
                |       FLOAT
                                {
                                        $$ = getSymbolInfoOfType(FLOAT_TYPE);
                                }
                |       VOID
                                {
                                        $$ = getSymbolInfoOfType(VOID_TYPE);
                                }
                ;

declaration_list : declaration_list COMMA ID
                                {
                                        //$$ = new SymbolInfo($1->getName() + "," + $3->getName(), "VARIABLE");
                                        insertVar($3);
                                }
                |       declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
                                {
                                        //$$ = new SymbolInfo($1->getName() + "," + $3->getName() + "[" + $5->getName() + "]", "VARIABLE");
                                        insertArr($3,$5);
                                }
                |       ID
                                {
                                        //$$ = $1;
                                        insertVar($1);
                                }
                |       ID LTHIRD CONST_INT RTHIRD
                                {
                                        //$$ = new SymbolInfo($1->getName() + "[" + $3->getName() + "]", "VARIABLE");
                                        insertArr($1, $3);
                                }
                |       ID LTHIRD error RTHIRD
                                {
                                        //$$ = new SymbolInfo($1->getName() + "[]", "VARIABLE");
                                        //printErrorRecovery("Constant integer type array size must be provided",declaration_list,"ID LTHIRD RTHIRD");
                                        printError("Constant integer type array size must be provided");
                                        insertVar($1);
                                }
                |       declaration_list COMMA ID LTHIRD error RTHIRD
                                {
                                        //$$ = new SymbolInfo($1->getName() + "," + $3->getName() + "[]", "VARIABLE");
                                        //printErrorRecovery("Constant integer type array size must be provided",declaration_list,"declaration_list COMMA ID LTHIRD RTHIRD");
                                        printError("Constant integer type array size must be provided");
                                        insertVar($3);
                                }
                |       declaration_list error COMMA ID
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z;
                                        **/ 
                                        //printErrorRecovery("Invalid declaration of variable/array",declaration_list,"declaration_list COMMA ID");
                                        //$$ = new SymbolInfo($1->getName() + "," + $4->getName(), "VARIABLE");
                                        printError("Invalid declaration of variable/array");
                                        insertVar($4);
                                }
                |       declaration_list error COMMA ID LTHIRD CONST_INT RTHIRD
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z[10];
                                        **/ 
                                        //printErrorRecovery("Invalid declaration of variable/array",declaration_list,"declaration_list COMMA ID LTHIRD CONST_INT RTHIRD");
                                        //$$ = new SymbolInfo($1->getName() + "," + $4->getName() + "[" + $6->getName() + "]", "VARIABLE");
                                        printError("Invalid declaration of variable/array");
                                        insertArr($4,$6);
                                }
                |       declaration_list error COMMA ID LTHIRD error RTHIRD
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z[10.5];
                                        **/
                                        //printErrorRecovery("Constant integer type array size must be provided",declaration_list,"declaration_list COMMA ID LTHIRD RTHIRD");
                                        //$$ = new SymbolInfo($1->getName() + "," + $4->getName() + "[]", "VARIABLE");
                                        printError("Constant integer type array size must be provided");
                                        insertVar($4);
                                }
                ;   

statements: statement
                                {
                                        $1->code += NEWLINE;
                                }
                |       statements statement
                                {}
                |       statements error
                                {}                
                ;

statement: var_declaration
                                {
                                        // $$ = $1;
                                }
                |       expression_statement
                                {
                                        // $$ = $1;
                                }
                |       compound_statement
                                {
                                        // $$ = $1;
                                }
                |       func_definition
                                {
                                        // printErrorRecovery("Invalid scoping of function", statement, "func_definition");
                                        printError("Invalid scoping of function");
                                }
                |       func_declaration
                                {
                                        // printErrorRecovery("Invalid scoping of function", statement, "func_declaration");
                                        printError("Invalid scoping of function");
                                }
                |       FOR LPAREN expression_statement { forLoopStart(); } expression_statement { forLoopConditionCheck(); } expression { gotoNextStepInForLoop($7->getName()); } RPAREN statement
                                {
                                        // $$ = new SymbolInfo("for(" + $3->getName() + $5->getName() + $7->getName() + ")" + $10->getName(), TEMPORARY_TYPE);
                                        endForLoop();
                                }
                |       IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE
                                {}
                |       IF LPAREN expression RPAREN statement ELSE statement
                                {}
                |       WHILE LPAREN expression RPAREN statement
                                {}
                |       PRINTLN LPAREN ID RPAREN SEMICOLON
                                {
                                        checkExistance($3);
                                }
                |       RETURN expression SEMICOLON
                                {
                                        checkFuncReturnType($2);
                                        $$->code += getExpressionCode($2);
                                }
                |       RETURN SEMICOLON
                                {
                                        /***
                                                EXTRA RULE ADDED 
                                        ***/
                                        checkFuncReturnType();
                                }
                |       IF LPAREN error RPAREN statement %prec LOWER_THAN_ELSE
                                {
                                        //printErrorRecovery("Invalid expression inside conditional if statement",statement,"IF LPAREN RPAREN statement");
                                        printError("Invalid expression inside conditional if statement");
                                }
                |       IF LPAREN error RPAREN statement ELSE statement
                                {
                                        //printErrorRecovery("Invalid expression inside conditional if statement",statement,"IF LPAREN RPAREN statement ELSE statement");
                                        printError("Invalid expression inside conditional if statement");
                                }
                |       error ELSE statement
                                {
                                        //printErrorRecovery("Else conditional statement without an if",statement,"ELSE statement");
                                        printError("Else conditional statement without an if");
                                }
                |       RETURN expression error		
                                {
                                        //printErrorRecovery("; missing",statement,"RETURN expression");
                                        printError("; missing");
                                }
                ;

expression_statement: SEMICOLON
                                {
                                        $$ = new SymbolInfo(";", TEMPORARY_TYPE);
                                }
                |       expression SEMICOLON
                                {
                                        $$ = new SymbolInfo($1->getName() + ";", TEMPORARY_TYPE);
                                        handleExtraExpressionPush($1->getName());
                                }
                ;

variable: ID            
                                {
                                        $$ = getVariable($1);
                                }
                |       ID LTHIRD expression RTHIRD
                                {
                                        $$ = getArrVar($1,$3);
                                }
                ;

expression: logic_expression
                                {
                                        $$ = $1;
                                }
                |       variable ASSIGNOP logic_expression
                                {
                                        $$ = getAssignExpVal($1, $3);
                                }
                ;

logic_expression: rel_expression
                                {
                                        $$ = $1;
                                }
                |       rel_expression LOGICOP rel_expression
                                {
                                        $$ = getLogicOpVal($1,$2,$3);
                                }
                ;

rel_expression: simple_expression
                                {
                                        $$=$1;
                                }
                |       simple_expression RELOP simple_expression
                                {
                                        $$ = getRelOpVal($1,$2,$3);
                                }
                ;

simple_expression: term
                                {
                                        $$ = $1;
                                }
                |       simple_expression ADDOP term
                                {
                                        $$ = getAddOpVal($1,$2,$3);
                                }
                ;

term:	unary_expression
                                {
                                        $$ = $1;
                                }
                |       term MULOP unary_expression
                                {
                                        SymbolInfo* s = getMulOpVal($1,$2,$3);
                                        if(s!=nullptr){
                                                $$ = s;
                                        }
                                }
                ;

unary_expression: ADDOP unary_expression
                                {
                                        $$ = getUnaryOpVal($1,$2);
                                }
                |       NOT unary_expression
                                {
                                        $$ = getNotOpVal($2);
                                }
                |       factor
                                {
                                        $$ = $1;
                                }
                ;

factor: variable                     
                                {
                                        $$ = $1;
                                        popArrayFromStack("BX", $1);
                                }
                |       ID LPAREN argument_list RPAREN
                                {
                                        SymbolInfo* s = getFuncCallValue($1);
                                        if(s!=nullptr){
                                                $$ = s;
                                        }
                                }
                |       LPAREN expression RPAREN
                                {
                                        $$ = $2;
                                }
                |       CONST_INT
                                {
                                        $$ = getConstValue($1,INT_TYPE);
                                }
                |       CONST_FLOAT
                                {
                                        $$ = getConstValue($1,FLOAT_TYPE);
                                }
                |       variable INCOP %prec POSTFIX_INCOP
                                {
                                        $$ = getINDECOpVal($1,"++","post");
                                }
                |       variable DECOP %prec POSTFIX_DECOP
                                {
                                        $$ = getINDECOpVal($1,"--","post");
                                }
                |       INCOP variable %prec PREFIX_INCOP
                                {
                                        $$ = getINDECOpVal($2,"++","pre");
                                }
                |       DECOP variable %prec PREFIX_DECOP
                                {
                                        $$ = getINDECOpVal($2,"--","pre");
                                }
                |       ID LPAREN argument_list error
                                {
                                        //printErrorRecovery("Right parentheses missing",factor,"ID LPAREN argument_list");
                                        printError("Right parentheses missing");
                                        clearFunctionParam();
                                }
                |       LPAREN expression error
                                {
                                        //printErrorRecovery("Right parentheses missing",factor,"LPAREN expression");
                                        printError("Right parentheses missing");
                                }
                ;

argument_list: arguments
                                {}
                |       
                                {}
                ;

arguments: arguments COMMA logic_expression
                                {
                                        paramType.push_back($3->getVarType());
                                }
                |       logic_expression
                                {
                                        paramType.push_back($1->getVarType());
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

	logout= fopen("log.txt","w");
	errorout= fopen("error.txt","w");
	parserout= fopen("parser.txt","w");
        asmFile.open("code.asm");

        yyin = inputFile;
        init_model();
	yyparse();
        symbolTable->printAllScope(logout);
        line_count--;
        fprintf(logout,"\nTotal Lines: %d\n",line_count);
        fprintf(errorout,"\nTotal Syntax/Semantic Errors: %d\n",syntax_error_count);
        fprintf(errorout,"\nTotal Lexical Errors: %d\n",error_count);
        fprintf(errorout,"\nTotal Warning: %d\n",warning_count);

        fclose(yyin);
	fclose(logout);
        fclose(errorout);
        fclose(parserout);

	return 0;
}