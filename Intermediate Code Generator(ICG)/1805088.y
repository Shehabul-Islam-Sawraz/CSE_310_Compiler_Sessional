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

%type <symbolInfo>type_specifier expression logic_expression rel_expression simple_expression term unary_expression factor variable

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
                        {
                                setValue(start,popValue(program));
                                printRuleAndCode(start,"program");
                        }
                ;

program: program unit
                                {
                                        setValue(program,popValue(program)+"\n"+popValue(unit));
                                        printRuleAndCode(program,"program unit");
                                }
                |       unit
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
                |       func_declaration
                                {
                                        setValue(unit,popValue(func_declaration));
                                        printRuleAndCode(unit,"func_declaration");
                                }
                |       func_definition
                                {
                                        setValue(unit,popValue(func_definition));
                                        printRuleAndCode(unit,"func_definition");
                                }
                ;

func_declaration: type_specifier ID LPAREN parameter_list RPAREN SEMICOLON
                                {
                                        insertFunc($2, $1);
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+";");
                                        printRuleAndCode(func_declaration,"type_specifier ID LPAREN parameter_list RPAREN SEMICOLON");
                                }       
                |       type_specifier ID LPAREN RPAREN SEMICOLON
                                {
                                        insertFunc($2, $1);
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+";");
                                        printRuleAndCode(func_declaration,"type_specifier ID LPAREN RPAREN SEMICOLON");
                                }
                |       type_specifier ID LPAREN parameter_list RPAREN error
                                {
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+"\n");
                                        printErrorRecovery("; missing",func_declaration,"type_specifier ID LPAREN parameter_list RPAREN");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN SEMICOLON
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(int x-y);
                                        **/
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+";");
                                        printErrorRecovery("Invalid parameter list",func_declaration,"type_specifier ID LPAREN parameter_list RPAREN SEMICOLON");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(int x-y)
                                        **/
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+"\n");
                                        printErrorRecovery("; missing. Error in function parameter list",func_declaration,"type_specifier ID LPAREN parameter_list RPAREN");
                                }
                |       type_specifier ID LPAREN RPAREN error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo()
                                        **/
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+"\n");
                                        printErrorRecovery("; missing",func_declaration,"type_specifier ID LPAREN RPAREN");
                                }
                |       type_specifier ID LPAREN error RPAREN SEMICOLON
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(-);
                                        **/
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+";");
                                        printErrorRecovery("Invalid parameter list",func_declaration,"type_specifier ID LPAREN RPAREN SEMICOLON");
                                }
                |       type_specifier ID LPAREN error RPAREN error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(-)
                                        **/
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+"\n");
                                        printErrorRecovery("; missing. Error in function parameter list",func_declaration,"type_specifier ID LPAREN RPAREN");
                                }
                ;

func_definition: type_specifier ID LPAREN parameter_list RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+popValue(compound_statement));
                                        printRuleAndCode(func_definition,"type_specifier ID LPAREN parameter_list RPAREN compound_statement");
                                }
                |       type_specifier ID LPAREN RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+")"+popValue(compound_statement));
                                        printRuleAndCode(func_definition,"type_specifier ID LPAREN RPAREN compound_statement");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        /**
                                                To handle cases like :
                                                void foo(int x-y){}
                                        **/
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+popValue(compound_statement));
                                }
                |       type_specifier ID LPAREN error RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        /**
                                                To handle cases like :
                                                void foo(-){}
                                        **/
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+")"+popValue(compound_statement));
                                }
                ;

parameter_list: parameter_list COMMA type_specifier ID
                                {
                                        insertIntoParamType($4);
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier)+" "+$4->getName());
                                        printRuleAndCode(parameter_list,"parameter_list COMMA type_specifier ID");
                                }
                |       parameter_list COMMA type_specifier
                                {
                                        paramType.push_back(variableType);
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier));
                                        printRuleAndCode(parameter_list,"parameter_list COMMA type_specifier");
                                }
                |       type_specifier ID
                                {
                                        insertIntoParamType($2);
                                        setValue(parameter_list,popValue(type_specifier)+" "+$2->getName());
                                        printRuleAndCode(parameter_list,"type_specifier ID");
                                }
                |       type_specifier
                                {
                                        paramType.push_back(variableType);
                                        setValue(parameter_list,popValue(type_specifier));
                                        printRuleAndCode(parameter_list,"type_specifier");
                                }
                |       parameter_list error COMMA type_specifier ID
                                {
                                         /**
                                                To handle errors like:
                                                void foo(int x-y,int z){}
                                        **/
                                        insertIntoParamType($5);
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier)+" "+$5->getName());
                                        printErrorRecovery("Invalid parameter list",parameter_list,"parameter_list COMMA type_specifier ID");
                                }
                |       parameter_list error COMMA type_specifier
                                {
                                        /**
                                                To handle cases like:
                                                void foo(int x-y,int);
                                        **/
                                        paramType.push_back(variableType);
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier));
                                        printErrorRecovery("Invalid parameter list",parameter_list,"parameter_list COMMA type_specifier");
                                }
                |       parameter_list COMMA error ID
                                {
                                        setValue(parameter_list,popValue(parameter_list)+", "+$4->getName());
                                        printErrorRecovery("Type specifier missing in parameters",parameter_list,"parameter_list COMMA ID");
                                }
                |       error ID
                                {
                                        setValue(parameter_list,$2->getName());
                                        printErrorRecovery("Type specifier missing in parameters",parameter_list,"ID");
                                }
                ;

compound_statement: LCURL create_scope statements RCURL
                                {
                                        setValue(compound_statement,"{"+popValue(statements)+"\n}");
                                        printRuleAndCode(compound_statement,"LCURL statements RCURL");
                                        exitScope();
                                }
                |       LCURL create_scope RCURL
                                {
                                        setValue(compound_statement,"{}");
                                        printRuleAndCode(compound_statement,"LCURL RCURL");
                                        exitScope();
                                } 
                |       LCURL create_scope error statements RCURL
                                {
                                        setValue(compound_statement,"{"+popValue(statements)+"\n}");
                                        printRuleAndCode(compound_statement,"LCURL statements RCURL");
                                        exitScope();
                                }     
                |       LCURL create_scope error RCURL
                                {
                                        setValue(compound_statement,"{}");
                                        printRuleAndCode(compound_statement,"LCURL RCURL");
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
                                        setValue(var_declaration, popValue(type_specifier)+" "+popValue(declaration_list)+ ";");
                                        printRuleAndCode(var_declaration,"type_specifier declaration_list SEMICOLON");
                                }
                |       type_specifier declaration_list error SEMICOLON
                                {
                                        /**
                                                To handle errors like :
                                                int x-y;
                                                int x[10]-y;
                                                int x[10.5]-y;
                                        **/ 
                                        setValue(var_declaration, popValue(type_specifier)+" "+popValue(declaration_list)+";");
                                        printErrorRecovery("Invalid variable declaration",var_declaration,"type_specifier declaration_list SEMICOLON");
                                }
                ;

type_specifier : INT
                                {
                                        $$ = getSymbolInfoOfType(INT_TYPE);
                                        setValue(type_specifier,"int");
                                        printRuleAndCode(type_specifier,"INT");
                                }
                |       FLOAT
                                {
                                        $$ = getSymbolInfoOfType(FLOAT_TYPE);
                                        setValue(type_specifier,"float");
                                        printRuleAndCode(type_specifier,"FLOAT");
                                }
                |       VOID
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
                |       declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
                                {
                                        insertArr($3,$5);
                                        setValue(declaration_list,popValue(declaration_list)+","+$3->getName()+"["+$5->getName()+"]");
                                        printRuleAndCode(declaration_list,"declaration_list COMMA ID LTHIRD CONST_INT RTHIRD");
                                }
                |       ID
                                {
                                        insertVar($1);
                                        setValue(declaration_list,$1->getName());
                                        printRuleAndCode(declaration_list,"ID");
                                }
                |       ID LTHIRD CONST_INT RTHIRD
                                {
                                        insertArr($1, $3);
                                        setValue(declaration_list,$1->getName()+"["+$3->getName()+"]");
                                        printRuleAndCode(declaration_list,"ID LTHIRD CONST_INT RTHIRD");
                                }
                |       ID LTHIRD error RTHIRD
                                {
                                        setValue(declaration_list,$1->getName()+"["+"]");
                                        printErrorRecovery("Constant integer type array size must be provided",declaration_list,"ID LTHIRD RTHIRD");
                                }
                |       declaration_list COMMA ID LTHIRD error RTHIRD
                                {
                                        setValue(declaration_list,popValue(declaration_list)+","+$3->getName()+"["+"]");
                                        printErrorRecovery("Constant integer type array size must be provided",declaration_list,"declaration_list COMMA ID LTHIRD RTHIRD");
                                }
                |       declaration_list error COMMA ID
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z;
                                        **/ 
                                        insertVar($4);
                                        setValue(declaration_list, popValue(declaration_list)+","+$4->getName());
                                        printErrorRecovery("Invalid declaration of variable/array",declaration_list,"declaration_list COMMA ID");
                                }
                |       declaration_list error COMMA ID LTHIRD CONST_INT RTHIRD
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z[10];
                                        **/ 
                                        insertArr($4,$6);
                                        setValue(declaration_list,popValue(declaration_list)+","+$4->getName()+"["+$6->getName()+"]");
                                        printErrorRecovery("Invalid declaration of variable/array",declaration_list,"declaration_list COMMA ID LTHIRD CONST_INT RTHIRD");
                                }
                |       declaration_list error COMMA ID LTHIRD error RTHIRD
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z[10.5];
                                        **/
                                        //string s1 = popValue(error);
                                        //string s2 = popValue(error);
                                        setValue(declaration_list,popValue(declaration_list)+","+$4->getName()+"["+"]");
                                        printErrorRecovery("Constant integer type array size must be provided",declaration_list,"declaration_list COMMA ID LTHIRD RTHIRD");
                                }
                ;   

statements: statement
                                {
                                        setValue(statements,popValue(statement));
                                        printRuleAndCode(statements,"statement");
                                }
                |       statements statement
                                {
                                        setValue(statements,popValue(statements)+popValue(statement));
                                        printRuleAndCode(statements,"statements statement");
                                }
                |       statements error
                                {
                                        setValue(statements,popValue(statements));
                                }                
                ;

statement: var_declaration 
                                {
                                        setValue(statement,"\t"+popValue(var_declaration));
                                        printRuleAndCode(statement,"var_declaration");
                                }
                |       expression_statement
                                {
                                        setValue(statement,"\t"+popValue(expression_statement));
                                        printRuleAndCode(statement,"expression_statement");
                                }
                |       compound_statement
                                {
                                        setValue(statement,popValue(compound_statement));
                                        printRuleAndCode(statement,"compound_statement");
                                }
                |       func_definition
                                {
                                        setValue(statement,popValue(func_definition));
                                        printErrorRecovery("Invalid scoping of function", statement, "func_definition");
                                }
                |       func_declaration
                                {
                                        setValue(statement,popValue(func_declaration));
                                        printErrorRecovery("Invalid scoping of function", statement, "func_declaration");
                                }
                |       FOR LPAREN expression_statement expression_statement expression RPAREN statement
                                {
                                        setValue(statement,(string("for")+"("+popValue(expression_statement)+popValue(expression_statement)+popValue(expression)+")\n"+popValue(statement)));
                                        printRuleAndCode(statement,"FOR LPAREN expression_statement expression_statement expression RPAREN statement");
                                }
                |       IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE
                                {
                                        setValue(statement,(string("if")+"("+popValue(expression)+")\n"+popValue(statement)));
                                        printRuleAndCode(statement,"IF LPAREN expression RPAREN statement");
                                }
                |       IF LPAREN expression RPAREN statement ELSE statement
                                {
                                        setValue(statement,(string("if")+"("+popValue(expression)+")\n"+popValue(statement)+"\nelse\n"+popValue(statement)));
                                        printRuleAndCode(statement,"IF LPAREN expression RPAREN statement ELSE statement");
                                }
                |       WHILE LPAREN expression RPAREN statement
                                {
                                        setValue(statement,(string("while")+"("+popValue(expression)+")\n"+popValue(statement)));
                                        printRuleAndCode(statement,"WHILE LPAREN expression RPAREN statement");
                                }
                |       PRINTLN LPAREN ID RPAREN SEMICOLON
                                {
                                        checkExistance($3);
                                        setValue(statement,"\tprintf("+$3->getName()+")"+";");
                                        printRuleAndCode(statement,"PRINTLN LPAREN ID RPAREN SEMICOLON");
                                }
                |       RETURN expression SEMICOLON
                                {
                                        checkFuncReturnType($2);
                                        setValue(statement,"\t"+string("return ")+popValue(expression)+";");
                                        printRuleAndCode(statement,"RETURN expression SEMICOLON");
                                }
                |       RETURN SEMICOLON
                                {
                                        /***
                                                EXTRA RULE ADDED 
                                        ***/
                                        checkFuncReturnType();
                                        setValue(statement,"\t"+string("return ")+";");
                                        printRuleAndCode(statement,"RETURN SEMICOLON");
                                }
                |       IF LPAREN error RPAREN statement %prec LOWER_THAN_ELSE
                                {
                                        setValue(statement,(string("if")+"("+")\n"+popValue(statement)));
                                        printErrorRecovery("Invalid expression inside conditional if statement",statement,"IF LPAREN RPAREN statement");
                                }
                |       IF LPAREN error RPAREN statement ELSE statement
                                {
                                        setValue(statement,(string("if")+"("+")\n"+popValue(statement)+"\nelse\n"+popValue(statement)));
                                        printErrorRecovery("Invalid expression inside conditional if statement",statement,"IF LPAREN RPAREN statement ELSE statement");
                                }
                |       error ELSE statement
                                {
                                        setValue(statement,"else"+popValue(statement));
                                        printErrorRecovery("Else conditional statement without an if",statement,"ELSE statement");
                                }
                |       RETURN expression error		
                                {
                                        setValue(statement,"return "+popValue(expression)+"");
                                        printErrorRecovery("; missing",statement,"RETURN expression");
                                }
                ;

expression_statement: SEMICOLON
                                {
                                        setValue(expression_statement,";");
                                        printRuleAndCode(expression_statement,"SEMICOLON");
                                }
                |       expression SEMICOLON
                                {
                                        setValue(expression_statement,popValue(expression)+";");
                                        printRuleAndCode(expression_statement,"expression SEMICOLON");
                                }
                ;

variable: ID            
                                {
                                        $$ = getVariable($1);
                                        setValue(variable,$1->getName());
                                        printRuleAndCode(variable,"ID");
                                }
                |       ID LTHIRD expression RTHIRD
                                {
                                        $$ = getArrVar($1,$3);
                                        setValue(variable,$1->getName()+"["+popValue(expression)+"]");
                                        printRuleAndCode(variable,"ID LTHIRD expression RTHIRD");
                                }
                ;

expression: logic_expression
                                {
                                        setValue(expression,popValue(logic_expression));
                                        printRuleAndCode(expression,"logic_expression");
                                }
                |       variable ASSIGNOP logic_expression
                                {
                                        $$ = getAssignExpVal($1, $3);
                                        setValue(expression,popValue(variable)+"="+popValue(logic_expression));
                                        printRuleAndCode(expression,"variable ASSIGNOP logic_expression");
                                }
                ;

logic_expression: rel_expression
                                {
                                        setValue(logic_expression,popValue(rel_expression));
                                        printRuleAndCode(logic_expression,"rel_expression");
                                }
                |       rel_expression LOGICOP rel_expression
                                {
                                        $$ = getLogicOpVal($1,$2,$3);
                                        string r2 = popValue(rel_expression);
                                        string r1 = popValue(rel_expression);
                                        setValue(logic_expression,r1+$2->getName()+r2);
                                        printRuleAndCode(logic_expression,"rel_expression LOGICOP rel_expression");
                                }
                ;

rel_expression: simple_expression
                                {
                                        setValue(rel_expression,popValue(simple_expression));
                                        printRuleAndCode(rel_expression,"simple_expression");
                                }
                |       simple_expression RELOP simple_expression
                                {
                                        $$ = getRelOpVal($1,$2,$3);
                                        string s2 = popValue(simple_expression);
                                        string s1 = popValue(simple_expression);
                                        setValue(rel_expression,s1+$2->getName()+s2);
                                        printRuleAndCode(rel_expression,"simple_expression RELOP simple_expression");
                                }
                ;

simple_expression: term
                                {
                                        setValue(simple_expression,popValue(term));
                                        printRuleAndCode(simple_expression,"term");
                                }
                |       simple_expression ADDOP term
                                {
                                        $$ = getAddOpVal($1,$2,$3);
                                        setValue(simple_expression,popValue(simple_expression)+$2->getName()+popValue(term));
                                        printRuleAndCode(simple_expression,"simple_expression ADDOP term");
                                }
                ;

term:	unary_expression
                                {
                                        setValue(term,popValue(unary_expression));
                                        printRuleAndCode(term,"unary_expression");
                                }
                |       term MULOP unary_expression
                                {
                                        SymbolInfo* s = getMulOpVal($1,$2,$3);
                                        if(s!=nullptr){
                                                $$ = s;
                                        }
                                        setValue(term,popValue(term)+$2->getName()+popValue(unary_expression));
                                        printRuleAndCode(term,"term MULOP unary_expression");
                                }
                ;

unary_expression: ADDOP unary_expression
                                {
                                        $$ = getUnaryOpVal($1,$2);
                                        setValue(unary_expression,$1->getName()+popValue(unary_expression));
                                        printRuleAndCode(unary_expression,"ADDOP unary_expression");
                                }
                |       NOT unary_expression
                                {
                                        $$ = getNotOpVal($2);
                                        setValue(unary_expression,"!"+popValue(unary_expression));
                                        printRuleAndCode(unary_expression,"NOT unary_expression");
                                }
                |       factor
                                {
                                        setValue(unary_expression,popValue(factor));
                                        printRuleAndCode(unary_expression,"factor");
                                }
                ;

factor: variable        
                                {
                                        setValue(factor,popValue(variable));
                                        printRuleAndCode(factor,"variable");
                                }
                |       ID LPAREN argument_list RPAREN
                                {
                                        SymbolInfo* s = getFuncCallValue($1);
                                        if(s!=nullptr){
                                                $$ = s;
                                        }
                                        setValue(factor,$1->getName()+"("+popValue(argument_list)+")");
                                        printRuleAndCode(factor,"ID LPAREN argument_list RPAREN");
                                }
                |       LPAREN expression RPAREN
                                {
                                        $$ = $2;
                                        setValue(factor,"("+popValue(expression)+")");
                                        printRuleAndCode(factor,"LPAREN expression RPAREN");
                                }
                |       CONST_INT
                                {
                                        $$ = getConstValue($1,INT_TYPE);
                                        setValue(factor,$1->getName());
                                        printRuleAndCode(factor,"CONST_INT");
                                }
                |       CONST_FLOAT
                                {
                                        $$ = getConstValue($1,FLOAT_TYPE);
                                        setValue(factor,$1->getName());
                                        printRuleAndCode(factor,"CONST_FLOAT");
                                }
                |       variable INCOP %prec POSTFIX_INCOP
                                {
                                        $$ = getINDECOpVal($1,"++","post");
                                        setValue(factor,popValue(variable)+"++");
                                        printRuleAndCode(factor,"variable INCOP");
                                }
                |       variable DECOP %prec POSTFIX_DECOP
                                {
                                        $$ = getINDECOpVal($1,"--","post");
                                        setValue(factor,popValue(variable)+"--");
                                        printRuleAndCode(factor,"variable DECOP");
                                }
                |       INCOP variable %prec PREFIX_INCOP
                                {
                                        $$ = getINDECOpVal($2,"++","pre");
                                        setValue(factor,"++"+popValue(variable));
                                        printRuleAndCode(factor,"variable INCOP");
                                }
                |       DECOP variable %prec PREFIX_DECOP
                                {
                                        $$ = getINDECOpVal($2,"--","pre");
                                        setValue(factor,"--"+popValue(variable));
                                        printRuleAndCode(factor,"variable DECOP");
                                }
                |       ID LPAREN argument_list error
                                {
                                        setValue(factor,$1->getName()+"("+popValue(argument_list));
                                        printErrorRecovery("Right parentheses missing",factor,"ID LPAREN argument_list");
                                        clearFunctionParam();
                                }
                |       LPAREN expression error
                                {
                                        setValue(factor,"("+popValue(expression)+"");
                                        printErrorRecovery("Right parentheses missing",factor,"LPAREN expression");
                                }
                ;

argument_list: arguments
                                {
                                        setValue(argument_list,popValue(arguments));
                                        printRuleAndCode(argument_list,"arguments");
                                }
                |       
                                {
                                        setValue(argument_list,"");
                                        printRuleAndCode(argument_list,"");
                                }
                ;

arguments: arguments COMMA logic_expression
                                {
                                        paramType.push_back($3->getVarType());
                                        setValue(arguments,popValue(arguments)+", "+popValue(logic_expression));
                                        printRuleAndCode(arguments,"arguments COMMA logic_expression");
                                }
                |       logic_expression
                                {
                                        paramType.push_back($1->getVarType());
                                        setValue(arguments,popValue(logic_expression));
                                        printRuleAndCode(arguments,"logic_expression");
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

        yyin = inputFile;
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