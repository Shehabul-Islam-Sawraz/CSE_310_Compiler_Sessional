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
%type <symbolInfo>program unit var_declaration func_declaration func_definition parameter_list compound_statement declaration_list 
%type <symbolInfo>statements statement expression_statement argument_list arguments create_if_block

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
                                        $$ = new SymbolInfo($1->getName() + "\n" + $2->getName(), TEMPORARY_TYPE);
                                        setValue(program,popValue(program)+"\n"+popValue(unit));
                                        printRuleAndCode(program,"program unit");
                                }
                |       unit
                                {
                                        $$ = $1;
                                        setValue(program,popValue(unit));
                                        printRuleAndCode(program,"unit");
                                }
                ;

unit:   var_declaration
                                {
                                        $$ = $1;
                                        setValue(unit,popValue(var_declaration));
                                        printRuleAndCode(unit,"var_declaration");
                                }
                |       func_declaration
                                {
                                        $$ = $1;
                                        setValue(unit,popValue(func_declaration));
                                        printRuleAndCode(unit,"func_declaration");
                                }
                |       func_definition
                                {
                                        $$ = $1;
                                        setValue(unit,popValue(func_definition));
                                        printRuleAndCode(unit,"func_definition");
                                }
                ;

func_declaration: type_specifier ID LPAREN parameter_list RPAREN {insertFunc($2, $1); startProcedure($2->getName());} SEMICOLON
                                {
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ");", "func_declaration");
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+";");
                                        printRuleAndCode(func_declaration,"type_specifier ID LPAREN parameter_list RPAREN SEMICOLON");
                                }       
                |       type_specifier ID LPAREN RPAREN {insertFunc($2, $1); startProcedure($2->getName());} SEMICOLON
                                {
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "();", "func_declaration");
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+";");
                                        printRuleAndCode(func_declaration,"type_specifier ID LPAREN RPAREN SEMICOLON");
                                }
                |       type_specifier ID LPAREN parameter_list RPAREN {insertFunc($2, $1);} error
                                {
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")", "func_declaration");
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+"\n");
                                        printError("; missing");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {insertFunc($2, $1);} SEMICOLON
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(int x-y);
                                        **/
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ");", "func_declaration");
                                        clearFunctionParam();
                                        printError("Invalid parameter list");
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+";");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {insertFunc($2, $1);} error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(int x-y)
                                        **/
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")", "func_declaration");
                                        clearFunctionParam();
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+"\n");
                                        printError("; missing. Error in function parameter list");
                                }
                |       type_specifier ID LPAREN RPAREN {insertFunc($2, $1);} error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo()
                                        **/
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()", "func_declaration");
                                        printError("; missing");
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+"\n");
                                }
                |       type_specifier ID LPAREN error RPAREN {insertFunc($2, $1);} SEMICOLON
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(-);
                                        **/
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "();", "func_declaration");
                                        printError("Invalid parameter list");
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+";");
                                }
                |       type_specifier ID LPAREN error RPAREN {insertFunc($2, $1);} error
                                {
                                        /**
                                                To handle errors like: 
                                                void foo(-)
                                        **/
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()", "func_declaration");
                                        printError("; missing. Error in function parameter list");
                                        setValue(func_declaration,popValue(type_specifier)+" "+$2->getName()+"("+")"+"\n");
                                }
                ;

func_definition: type_specifier ID LPAREN parameter_list RPAREN {addFunctionDef($1, $2); startProcedure($2->getName());} compound_statement
                                {
                                        endFuncDef(true, $2->getName(), $1->getName());
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")" + $7->getName(), "func_definition");
                                        writeENDPForFunc($2->getName());
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+popValue(compound_statement));
                                        printRuleAndCode(func_definition,"type_specifier ID LPAREN parameter_list RPAREN compound_statement");
                                }
                |       type_specifier ID LPAREN RPAREN {addFunctionDef($1, $2); startProcedure($2->getName());} compound_statement
                                {
                                        endFuncDef(true, $2->getName(), $1->getName());
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()" + $6->getName(), "func_definition");
                                        writeENDPForFunc($2->getName());
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+")"+popValue(compound_statement));
                                        printRuleAndCode(func_definition,"type_specifier ID LPAREN RPAREN compound_statement");
                                }
                |       type_specifier ID LPAREN parameter_list error RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        /**
                                                To handle cases like :
                                                void foo(int x-y){}
                                        **/
                                        endFuncDef();
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "(" + $4->getName() + ")" + $8->getName(), "func_definition");
                                        writeENDPForFunc($2->getName());
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+popValue(parameter_list)+")"+popValue(compound_statement));
                                }
                |       type_specifier ID LPAREN error RPAREN {addFunctionDef($1, $2);} compound_statement
                                {
                                        /**
                                                To handle cases like :
                                                void foo(-){}
                                        **/
                                        endFuncDef();
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + "()" + $7->getName(), "func_definition");
                                        writeENDPForFunc($2->getName());
                                        setValue(func_definition,popValue(type_specifier)+" "+$2->getName()+"("+")"+popValue(compound_statement));
                                }
                ;

parameter_list: parameter_list COMMA type_specifier ID
                                {
                                        $$ = new SymbolInfo($1->getName() + "," + $3->getName() + " " + $4->getName(), "parameter_list");
                                        insertIntoParamType($4);
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier)+" "+$4->getName());
                                        printRuleAndCode(parameter_list,"parameter_list COMMA type_specifier ID");
                                }
                |       parameter_list COMMA type_specifier
                                {
                                        $$ = new SymbolInfo($1->getName() + "," + $3->getName(), "parameter_list");
                                        paramType.push_back(variableType);
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier));
                                        printRuleAndCode(parameter_list,"parameter_list COMMA type_specifier");
                                }
                |       type_specifier ID
                                {
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName(), "VARIABLE");
                                        insertIntoParamType($2);
                                        setValue(parameter_list,popValue(type_specifier)+" "+$2->getName());
                                        printRuleAndCode(parameter_list,"type_specifier ID");
                                }
                |       type_specifier
                                {
                                        $$ = $1;
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
                                        $$ = new SymbolInfo($1->getName() + "," + $4->getName() + " " + $5->getName(), "parameter_list");
                                        insertIntoParamType($5);
                                        printError("Invalid parameter list");
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier)+" "+$5->getName());
                                }
                |       parameter_list error COMMA type_specifier
                                {
                                        /**
                                                To handle cases like:
                                                void foo(int x-y,int);
                                        **/
                                        $$ = new SymbolInfo($1->getName() + "," + $4->getName(), "parameter_list");
                                        paramType.push_back(variableType);
                                        printError("Invalid parameter list");
                                        setValue(parameter_list,popValue(parameter_list)+", "+popValue(type_specifier));
                                }
                |       parameter_list COMMA error ID
                                {
                                        $$ = new SymbolInfo($1->getName() + "," + $4->getName(), "parameter_list");
                                        printError("Type specifier missing in parameters");
                                        setValue(parameter_list,popValue(parameter_list)+", "+$4->getName());
                                }
                |       error ID
                                {
                                        $$ = $2;
                                        printError("Type specifier missing in parameters");
                                        setValue(parameter_list,$2->getName());
                                }
                ;

compound_statement: LCURL create_scope statements RCURL
                                {
                                        $$ = new SymbolInfo("{\n" + $3->getName() + "\n}", TEMPORARY_TYPE);
                                        setValue(compound_statement,"{"+popValue(statements)+"\n}");
                                        printRuleAndCode(compound_statement,"LCURL statements RCURL");
                                        exitScope();
                                }
                |       LCURL create_scope RCURL
                                {
                                        $$ = new SymbolInfo("{}", TEMPORARY_TYPE);
                                        setValue(compound_statement,"{}");
                                        printRuleAndCode(compound_statement,"LCURL RCURL");
                                        exitScope();
                                } 
                |       LCURL create_scope error statements RCURL
                                {
                                        $$ = new SymbolInfo("{\n" + $4->getName() + "\n}", TEMPORARY_TYPE);
                                        setValue(compound_statement,"{"+popValue(statements)+"\n}");
                                        printRuleAndCode(compound_statement,"LCURL statements RCURL");
                                        exitScope();
                                }     
                |       LCURL create_scope error RCURL
                                {
                                        $$ = new SymbolInfo("{}", TEMPORARY_TYPE);
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
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + ";", "VARIABLE");
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
                                        $$ = new SymbolInfo($1->getName() + " " + $2->getName() + ";", "VARIABLE");
                                        printError("Invalid variable declaration");
                                        setValue(var_declaration, popValue(type_specifier)+" "+popValue(declaration_list)+";");
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
                                        $$ = new SymbolInfo($1->getName() + "," + $3->getName(), "VARIABLE");
                                        insertVar($3);
                                        setValue(declaration_list, popValue(declaration_list)+","+$3->getName());
                                        printRuleAndCode(declaration_list,"declaration_list COMMA ID");
                                }
                |       declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
                                {
                                        $$ = new SymbolInfo($1->getName() + "," + $3->getName() + "[" + $5->getName() + "]", "VARIABLE");
                                        insertArr($3,$5);
                                        setValue(declaration_list,popValue(declaration_list)+","+$3->getName()+"["+$5->getName()+"]");
                                        printRuleAndCode(declaration_list,"declaration_list COMMA ID LTHIRD CONST_INT RTHIRD");
                                }
                |       ID
                                {
                                        $$ = $1;
                                        insertVar($1);
                                        setValue(declaration_list,$1->getName());
                                        printRuleAndCode(declaration_list,"ID");
                                }
                |       ID LTHIRD CONST_INT RTHIRD
                                {
                                        $$ = new SymbolInfo($1->getName() + "[" + $3->getName() + "]", "VARIABLE");
                                        insertArr($1, $3);
                                        setValue(declaration_list,$1->getName()+"["+$3->getName()+"]");
                                        printRuleAndCode(declaration_list,"ID LTHIRD CONST_INT RTHIRD");
                                }
                |       ID LTHIRD error RTHIRD
                                {
                                        $$ = new SymbolInfo($1->getName() + "[]", "VARIABLE");
                                        printError("Constant integer type array size must be provided");
                                        insertVar($1);
                                        setValue(declaration_list,$1->getName()+"["+"]");
                                }
                |       declaration_list COMMA ID LTHIRD error RTHIRD
                                {
                                        $$ = new SymbolInfo($1->getName() + "," + $3->getName() + "[]", "VARIABLE");
                                        printError("Constant integer type array size must be provided");
                                        insertVar($3);
                                        setValue(declaration_list,popValue(declaration_list)+","+$3->getName()+"["+"]");
                                }
                |       declaration_list error COMMA ID
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z;
                                        **/ 
                                        $$ = new SymbolInfo($1->getName() + "," + $4->getName(), "VARIABLE");
                                        printError("Invalid declaration of variable/array");
                                        insertVar($4);
                                        setValue(declaration_list, popValue(declaration_list)+","+$4->getName());
                                }
                |       declaration_list error COMMA ID LTHIRD CONST_INT RTHIRD
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z[10];
                                        **/ 
                                        $$ = new SymbolInfo($1->getName() + "," + $4->getName() + "[" + $6->getName() + "]", "VARIABLE");
                                        printError("Invalid declaration of variable/array");
                                        insertArr($4,$6);
                                        setValue(declaration_list,popValue(declaration_list)+","+$4->getName()+"["+$6->getName()+"]");
                                }
                |       declaration_list error COMMA ID LTHIRD error RTHIRD
                                {
                                        /**
                                                To handle errors like :
                                                int x-y,z[10.5];
                                        **/
                                        $$ = new SymbolInfo($1->getName() + "," + $4->getName() + "[]", "VARIABLE");
                                        printError("Constant integer type array size must be provided");
                                        insertVar($4);
                                        setValue(declaration_list,popValue(declaration_list)+","+$4->getName()+"["+"]");
                                }
                ;   

statements: statement
                                {
                                        $$ = $1;
                                        setValue(statements,popValue(statement));
                                        printRuleAndCode(statements,"statement");
                                }
                |       statements statement
                                {
                                        $$ = new SymbolInfo($1->getName() + NEWLINE + $2->getName(), TEMPORARY_TYPE);
                                        setValue(statements,popValue(statements)+popValue(statement));
                                        printRuleAndCode(statements,"statements statement");
                                }
                |       statements error
                                {
                                        $$ = $1;
                                        setValue(statements,popValue(statements));
                                }                
                ;

statement: var_declaration
                                {
                                        $$ = $1;
                                        setValue(statement,"\t"+popValue(var_declaration));
                                        printRuleAndCode(statement,"var_declaration");
                                }
                |       expression_statement
                                {
                                        $$ = $1;
                                        setValue(statement,"\t"+popValue(expression_statement));
                                        printRuleAndCode(statement,"expression_statement");
                                }
                |       compound_statement
                                {
                                        $$ = $1;
                                        setValue(statement,popValue(compound_statement));
                                        printRuleAndCode(statement,"compound_statement");
                                }
                |       func_definition
                                {
                                        printError("Invalid scoping of function");
                                        setValue(statement,popValue(func_definition));
                                }
                |       func_declaration
                                {
                                        printError("Invalid scoping of function");
                                        setValue(statement,popValue(func_declaration));
                                }
                |       FOR LPAREN expression_statement { forLoopStart(); } expression_statement { forLoopConditionCheck(); } expression { gotoNextStepInForLoop($7->getName()); } RPAREN statement
                                {
                                        $$ = new SymbolInfo("for(" + $3->getName() + $5->getName() + $7->getName() + ")" + $10->getName(), TEMPORARY_TYPE);
                                        setValue(statement,(string("for")+"("+popValue(expression_statement)+popValue(expression_statement)+popValue(expression)+")\n"+popValue(statement)));
                                        printRuleAndCode(statement,"FOR LPAREN expression_statement expression_statement expression RPAREN statement");
                                        endForLoop();
                                }
                |       IF LPAREN expression RPAREN create_if_block statement %prec LOWER_THAN_ELSE
                                {
                                        $$ = new SymbolInfo("if(" + $3->getName() + ")" + $6->getName(), TEMPORARY_TYPE);
                                        setValue(statement,(string("if")+"("+popValue(expression)+")\n"+popValue(statement)));
                                        printRuleAndCode(statement,"IF LPAREN expression RPAREN statement");
                                        endIfBlock($5->getName());
                                }
                |       IF LPAREN expression RPAREN create_if_block statement ELSE { createElseBlock($5->getName()); } statement
                                {
                                        $$ = new SymbolInfo("if(" + $3->getName() + ")" + $6->getName() + "else\n" + $9->getName(), TEMPORARY_TYPE);
                                        setValue(statement,(string("if")+"("+popValue(expression)+")\n"+popValue(statement)+"\nelse\n"+popValue(statement)));
                                        printRuleAndCode(statement,"IF LPAREN expression RPAREN statement ELSE statement");
                                        endIfElseBlock();
                                }
                |       WHILE { whileLoopStart(); } LPAREN expression { whileLoopConditionCheck($4->getName()); } RPAREN statement
                                {
                                        $$ = new SymbolInfo("while(" + $4->getName() + ")" + $7->getName(), TEMPORARY_TYPE);
                                        setValue(statement,(string("while")+"("+popValue(expression)+")\n"+popValue(statement)));
                                        printRuleAndCode(statement,"WHILE LPAREN expression RPAREN statement");
                                        endWhileLoop();
                                }
                |       PRINTLN LPAREN ID RPAREN SEMICOLON
                                {
                                        $$ = new SymbolInfo("println(" + $3->getName() + ");", TEMPORARY_TYPE);
                                        checkExistance($3);
                                        setValue(statement,"\tprintf("+$3->getName()+")"+";");
                                        printRuleAndCode(statement,"PRINTLN LPAREN ID RPAREN SEMICOLON");
                                        printIdValue($3);
                                }
                |       RETURN expression SEMICOLON
                                {
                                        $$ = new SymbolInfo("return " + $2->getName() + ";", TEMPORARY_TYPE);
                                        checkFuncReturnType($2);
                                        returnFunction();
                                        setValue(statement,"\t"+string("return ")+popValue(expression)+";");
                                        printRuleAndCode(statement,"RETURN expression SEMICOLON");
                                }
                |       RETURN SEMICOLON
                                {
                                        /***
                                                EXTRA RULE ADDED 
                                        ***/
                                        $$ = new SymbolInfo("return ;", TEMPORARY_TYPE);
                                        checkFuncReturnType();
                                        returnFunction();
                                        setValue(statement,"\t"+string("return ")+";");
                                        printRuleAndCode(statement,"RETURN SEMICOLON");
                                }
                |       IF LPAREN error RPAREN statement %prec LOWER_THAN_ELSE
                                {
                                        $$ = new SymbolInfo("if()" + $5->getName(), TEMPORARY_TYPE);
                                        printError("Invalid expression inside conditional if statement");
                                        setValue(statement,(string("if")+"("+")\n"+popValue(statement)));
                                }
                |       IF LPAREN error RPAREN statement ELSE statement
                                {
                                        $$ = new SymbolInfo("if()" + $5->getName() + "else\n" + $7->getName(), TEMPORARY_TYPE);
                                        printError("Invalid expression inside conditional if statement");
                                        setValue(statement,(string("if")+"("+")\n"+popValue(statement)+"\nelse\n"+popValue(statement)));
                                }
                |       error ELSE statement
                                {
                                        $$ = new SymbolInfo("else\n" + $3->getName(), TEMPORARY_TYPE);
                                        printError("Else conditional statement without an if");
                                        setValue(statement,"else"+popValue(statement));
                                }
                |       RETURN expression error		
                                {
                                        $$ = new SymbolInfo("return " + $2->getName(), TEMPORARY_TYPE);
                                        setValue(statement,"return "+popValue(expression)+"");
                                        printError("; missing");
                                }
                ;

create_if_block:
                                {
                                        $$ = createIfBlock();
                                }
                ;

expression_statement: SEMICOLON
                                {
                                        $$ = new SymbolInfo(";", TEMPORARY_TYPE);
                                        setValue(expression_statement,";");
                                        printRuleAndCode(expression_statement,"SEMICOLON");
                                }
                |       expression SEMICOLON
                                {
                                        $$ = new SymbolInfo($1->getName() + ";", TEMPORARY_TYPE);
                                        handleExtraExpressionPush($1->getName());
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
                                        $$ = $1;
                                        setValue(expression,popValue(logic_expression));
                                        printRuleAndCode(expression,"logic_expression");
                                }
                |       variable ASSIGNOP logic_expression
                                {
                                        getAssignExpVal($1, $3);
                                        $$ = new SymbolInfo($1->getName() + "=" + $3->getName(), $1->getType());
                                        setValue(expression,popValue(variable)+"="+popValue(logic_expression));
                                        printRuleAndCode(expression,"variable ASSIGNOP logic_expression");
                                }
                ;

logic_expression: rel_expression
                                {
                                        $$ = $1;
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
                                        $$=$1;
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
                                        $$ = $1;
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
                                        $$ = $1;
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
                                        $$ = $1;
                                        setValue(unary_expression,popValue(factor));
                                        printRuleAndCode(unary_expression,"factor");
                                }
                ;

factor: variable                     
                                {
                                        $$ = $1;
                                        popArrayFromStack("BX", $1);
                                        setValue(factor,popValue(variable));
                                        printRuleAndCode(factor,"variable");
                                }
                |       ID LPAREN argument_list RPAREN
                                {
                                        $$ = getFuncCallValue($1);
                                        $$->setName($1->getName() + "(" + $3->getName() + ")");
                                        setValue(factor,$1->getName()+"("+popValue(argument_list)+")");
                                        printRuleAndCode(factor,"ID LPAREN argument_list RPAREN");
                                }
                |       LPAREN expression RPAREN
                                {
                                        $$ = new SymbolInfo("(" + $2->getName() + ")", $2->getType());
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
                                        $$->setName($1->getName() + "++");
                                        setValue(factor,popValue(variable)+"++");
                                        printRuleAndCode(factor,"variable INCOP");
                                }
                |       variable DECOP %prec POSTFIX_DECOP
                                {
                                        $$ = getINDECOpVal($1,"--","post");
                                        $$->setName($1->getName() + "--");
                                        setValue(factor,popValue(variable)+"--");
                                        printRuleAndCode(factor,"variable DECOP");
                                }
                |       INCOP variable %prec PREFIX_INCOP
                                {
                                        $$ = getINDECOpVal($2,"++","pre");
                                        $$->setName("++" + $2->getName());
                                        setValue(factor,"++"+popValue(variable));
                                        printRuleAndCode(factor,"variable INCOP");
                                }
                |       DECOP variable %prec PREFIX_DECOP
                                {
                                        $$ = getINDECOpVal($2,"--","pre");
                                        $$->setName("--" + $2->getName());
                                        setValue(factor,"--"+popValue(variable));
                                        printRuleAndCode(factor,"variable DECOP");
                                }
                |       ID LPAREN argument_list error
                                {
                                        printError("Right parentheses missing");
                                        setValue(factor,$1->getName()+"("+popValue(argument_list));
                                        clearFunctionParam();
                                }
                |       LPAREN expression error
                                {
                                        setValue(factor,"("+popValue(expression)+"");
                                        printError("Right parentheses missing");
                                }
                ;

argument_list: arguments
                                {
                                        $$ = $1;
                                        setValue(argument_list,popValue(arguments));
                                        printRuleAndCode(argument_list,"arguments");
                                }
                |       
                                {
                                        $$ = new SymbolInfo("", TEMPORARY_TYPE);
                                        setValue(argument_list,"");
                                        printRuleAndCode(argument_list,"");
                                }
                ;

arguments: arguments COMMA logic_expression
                                {
                                        $$ = new SymbolInfo($1->getName() + "," + $3->getName(), TEMPORARY_TYPE);
                                        paramType.push_back($3->getVarType());
                                        setValue(arguments,popValue(arguments)+", "+popValue(logic_expression));
                                        printRuleAndCode(arguments,"arguments COMMA logic_expression");
                                }
                |       logic_expression
                                {
                                        $$ = $1;
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
        asmFile.open("code.asm");
        // optimizedFile.open("optimizedCode.asm");

        yyin = inputFile;
        init_model();
        writeInDataSegment();
	yyparse();
        symbolTable->printAllScope(logout);
        line_count--;
        fprintf(logout,"\nTotal Lines: %d\n",line_count);
        fprintf(errorout,"\nTotal Syntax/Semantic Errors: %d\n",syntax_error_count);
        fprintf(errorout,"\nTotal Lexical Errors: %d\n",error_count);
        fprintf(errorout,"\nTotal Warning: %d\n",warning_count);
        
        if((error_count+syntax_error_count)>0){
                asmFile.close();
                asmFile.open("code.asm");
        }
        else{
                writePrintNumProc();
                endAssemblyCode();
        }

        optimizedFile.open("optimized_code.asm");
        optimizeCodeSegment(1); // For first level pass
        optimizedFile.close();

        optimizedFile.open("final_optimized_code.asm");
        optimizeCodeSegment(2);  // For second level pass
        optimizedFile.close();

        fclose(yyin);
	fclose(logout);
        fclose(errorout);
        fclose(parserout);
        asmFile.close();
	return 0;
}