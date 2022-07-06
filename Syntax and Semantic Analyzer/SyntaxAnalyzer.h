#include "SymbolTable.h"
#include<stack>

#define NoOfBuckets 7
#define ARRAY "ARR"
#define VARIABLE "VAR"
#define FUNCTION "FUNC"
#define INT_TYPE "INT"
#define FLOAT_TYPE "FLOAT"
#define VOID_TYPE "VOID"
#define CHAR_TYPE "CHAR"

FILE *logout, *errorout, *parserout;
extern int line_count;
extern int error_count;
extern FILE *yyin;
extern char *yytext;
string variableType;
size_t syntax_error_count = 0;
size_t warning_count = 0;

size_t noOfParam = 0
vector<string> paramType;
vector<SymbolInfo> parameters;
SymbolInfo* currentFunc = nullptr;

ScopeTable* scope = nullptr;
SymbolTable* symbolTable = new SymbolTable();

int yyparse();
int yylex();

string ruleName[] = {"start", "program", "unit", "func_declaration", "func_definition", "parameter_list",
                           "compound_statement", "var_declaration", "type_specifier", "declaration_list", "statements",
                           "statement", "expression_statement", "variable", "expression", "logic_expression",
                           "rel_expression", "simple_expression", "term", "unary_expression", "factor", "argument_list",
                           "arguments"};

enum NONTERMINAL_TYPE {
	start = 0, program, unit, func_declaration, func_definition,
	parameter_list, compound_statement, var_declaration, type_specifier,
	declaration_list, statements, statement, expression_statement,
	variable, expression, logic_expression, rel_expression, simple_expression,
	term, unary_expression, factor, argument_list, arguments, error
};

bool replaceAll(string source, string toReplace, string replaceBy) {
	if (source.find(toReplace, 0) == string::npos){
		return false;
	}
	for (string::size_type i = 0; (i = source.find(toReplace, i)) != string::npos;) {
		source.replace(i, toReplace.length(), replaceBy);
		i += replaceBy.length();
	}
	return true;
}

string formatCode(string code){
	string formattedCode = code;
	while (replaceAll(formattedCode, " ;", ";"));
	replaceAll(formattedCode, ";", ";\n");
	replaceAll(formattedCode, ";\n\n", ";\n");
	while (replaceAll(formattedCode, "( ", "("));
	while (replaceAll(formattedCode, " (", "("));
	while (replaceAll(formattedCode, ") ", ")"));
	while (replaceAll(formattedCode, " )", ")"));
	while (replaceAll(formattedCode, "{ ", "{"));
	while (replaceAll(formattedCode, " {", "{"));
	while (replaceAll(formattedCode, "} ", "}"));
	while (replaceAll(formattedCode, " }", "}"));
	replaceAll(formattedCode, "{", "\n{\n");
	replaceAll(formattedCode, "}", "\n}\n");
	while (replaceAll(formattedCode, "\n\n}", "\n}"));
	while (replaceAll(formattedCode, "}\n\n", "}\n"));
	while (replaceAll(formattedCode, "  = ", " = "));
	while (replaceAll(formattedCode, " =  ", " = "));
	while (replaceAll(formattedCode, "  == ", " == "));
	while (replaceAll(formattedCode, " ==  ", " == "));
	replaceAll(formattedCode, "(", " ( ");
	replaceAll(formattedCode, ")", " ) ");
	while (replaceAll(formattedCode, " ++", "++"));
	while (replaceAll(formattedCode, " --", "--"));
	while (replaceAll(formattedCode, "\n ", "\n"));
	while (replaceAll(formattedCode, " \n", "\n"));
	while (replaceAll(formattedCode, "  ", " "));
	return formattedCode;
}

uint32_t hashValue(string str){
    uint32_t hash = 0;
    for(int c:str)
        hash = c + (hash * 64) + (hash *65536) - hash;
    return hash;
}

class NonTerminalHandler
{
private:
	stack<string> nonterminals[NONTERMINAL_TYPE::error +1];
public:
	string getValue(NONTERMINAL_TYPE nonterminal){
		return nonterminals[nonterminal].top();
	}
	string popValue(NONTERMINAL_TYPE nonterminal){
		if(nonTerminalBuf[nonterminal].empty()){
			return "";
		}
		string str = nonterminals[nonterminal].top();
		nonterminals[nonterminal].pop();
		return str;
	}
	void setValue(NONTERMINAL_TYPE nonterminal, string value){
		nonterminals[nonterminal].push(value);
	}
};

NonTerminalHandler nonTerminalHandler;

void yyerror(const char *s) {
	cout << "Error at line: " << line_count << endl;
}

void clearFunctionParam(){
	paramType.clear();
	parameters.clear();
	noOfParam = 0;
}

SymbolInfo* insertIntoSymbolTable(SymbolInfo* symbol){
    if(scope==nullptr){
        scope = symbolTable->createScopeTable(NoOfBuckets);
    }
    scope->insertSymbol(symbol->getName(), symbol->getType(),(int)(hashValue(symbol->getName())%NoOfBuckets), symbol->getVarType(), symbol->getDecType());
	return symbolTable->lookUp(symbol->getName(), (int)(hashValue(symbol->getName())%NoOfBuckets));
}

SymbolInfo* getSymbolInfoOfType(string type){
	variableType = type;
	return new SymbolInfo(variableType,variableType);
}

void setValue(NONTERMINAL_TYPE nonterminal, string value){
	nonTerminalHandler.setValue(nonterminal,value);
}

string popValue(NONTERMINAL_TYPE nonterminal) {
	string val = nonTerminalHandler.popValue(nonterminal);
	val = (isalnum(val[0]) ? " " : "") + val + (isalnum(val[val.length() - 1]) ? " " : "");
	return val + ((val[val.length() - 1] == ' ') ? "" : " ");
}

void printRuleAndCode(NONTERMINAL_TYPE nonterminal, string rule){
	fprintf(logout,"Line %d: %s : %s\n",line_count, ruleName[nonterminal].data(), rule.data());
	fprintf(logout,"%s\n", formatCode(nonTerminalHandler.getValue(nonterminal)).data());
}

void printError(string error_msg){
	fprintf(errorout,"Error at line %d: %s\n",line_count, error_msg.data());
	syntax_error_count++;
}

void printWarning(string error_msg){
	fprintf(errorout,"Warning at line %d: %s\n",line_count, error_msg.data());
	warning_count++;
}

SymbolInfo* insertVar(SymbolInfo* var){
	if(variableType==VOID_TYPE) { // Semantic error
		printError("Variable type cannot be void");
	}
	else {
		if(symbolTable->lookUp(var->getName(), (int)(hashValue(var->getName())%NoOfBuckets))!=nullptr){
			printError("Multiple declaration of " + var->getName());
		}
		else{
			var->setDecType(VARIABLE);
			var->setVarType(variableType);
			SymbolInfo* temp = insertIntoSymbolTable(var);
			return temp;
		}
	}
	return nullptr;
}

void insertFunc(SymbolInfo* func, SymbolInfo* retType){
	if(symbolTable->lookUp(func->getName(), (int)(hashValue(func->getName())%NoOfBuckets))!=nullptr){
		printError("Multiple declaration of "+printError("Multiple declaration of " + var->getName());->getName());
		clearFunctionParam();
		return;
	}
	if (noOfParam && find(paramType.begin(), paramType.end(), VOID_TYPE) != paramType.end()) {
		printErrorLog("Parameters can't be of void type in function "+func->getName());
		return;
	}
	func->setType("ID");
	func->setDecType(FUNCTION);
	func->setVarType(retType->getName());
	SymbolInfo* temp = insertIntoSymbolTable(func);
	temp->setFuncRetType(retType->getName());
	temp->setparamType(paramType);
}

void insertArr(SymbolInfo *sym, SymbolInfo *index) {
	SymbolInfo *arr = insertVar(sym);
	arr->setDecType(ARRAY);
	arr->setArrSize(static_cast<size_t>(atoi(index->getName().data()))); // Set the size of the array as defined size
	int l = arr->getArrSize();
	for(int i=0;i<l;i++){
		arr->intValues.push_back(0);
		arr->floatValues.push_back(0);
	}
}

void addFunctionDef(SymbolInfo* retType, SymbolInfo* func){
	SymbolInfo* temp = symbolTable->lookUp(func->getName(),(int)(hashValue(func->getName())%NoOfBuckets));
	// to prevent f(int x,float,int y){} type defn but f(void){} allowed
	if (!(paramType.size() == 1 && paramType[0] == VOID_TYPE) && paramType.size() != noOfParam) {
		printError("Unnamed prototype parameter not allowed in definition of function " + funcVal->getName());
	}
	else if(temp!=nullptr){
		if(temp->getDecType()!=FUNCTION){
			printError("Multiple declaration of "+temp->getName());
			return;
		}
		else if(temp->getIsFuncDeclared()){
			printError("Multiple Definition of " + temp->getName());
			return;
		}
		else if(temp->getFuncRetType()!=retType->getName()){
			printError("Return type mismatch with function declaration in function "+temp->getName());
			return;
		}
		else if(temp->getparamType().size()!=paramType.size()){
			printError("Total number of arguments mismatch with declaration in function "+temp->getName());
			return;
		}
		vector<string> v = temp->getparamType();
		bool iserr = false;
		for(int i=0;i,paramType.size()){
			if(v[i].compare(paramType[i])!=0){
				printError((i+1)+" th argument mismatch in function "+temp->getName());
				iserr = true;
			}
		}
		if(iserr){
			return;
		}
		temp->setIsFuncDeclared(true);
	}
	else{
		insertFunc(func,retType);
		func = symbolTable->lookUp(func->getName(),(int)(hashValue(func->getName())%NoOfBuckets));
		func->setIsFuncDeclared(true);
		currentFunc = func;
	}
}

void insertIntoParamType(SymbolInfo* var){
	int l = parameters.size();
	string str = var->getName();
	for(int i=0;i<l;i++){
		if(parameters[i]->getName().compare(str)==0){
			printError("Multiple declaration of "+str+" in parameters");
			break;
		}
	}
	paramType.push_back(variableType);
	noOfParam++;
	//var->setDecType(VARIABLE);
	//var->setVarType(variableType);
	SymbolInfo* temp = new SymbolInfo(var->getName(), var->getType());
	temp->setDecType(VARIABLE);
	temp->setVarType(variableType);
	parameters.push_back(*temp);
}

SymbolInfo* getVariable(SymbolInfo* sym){
	SymbolInfo* temp = symbolTable->lookUp(sym->getName(),(int)(hashValue(sym->getName())%NoOfBuckets));
	if(temp==nullptr){
		printError("Undeclared variable "+sym->getName());
	}
	else if(!temp->isVariable()){
		if(temp->isArray()){
			printError("Type mismatch, "+sym->getName()+" is an array");
		}
		else if(temp->isFunction()){
			printError("Type mismatch, "+sym->getName()+" is an function");
		}
		else{
			printError("Type mismatch, "+sym->getName()+" is not a variable");
		}
	}
	else{
		return temp;
	}
	return nullptr;
}

SymbolInfo* getArrVar(SymbolInfo* sym, SymbolInfo* index){
	SymbolInfo* temp = symbolTable->lookUp(sym->getName(),(int)(hashValue(sym->getName())%NoOfBuckets));
	if(temp==nullptr){
		printError("Undeclared variable "+sym->getName());
	}
	else{
		if(!temp->isArray()){
			printError(sym->getName()+" is not an array");
		}
		else if(index->getVarType()!=INT_TYPE){
			printError("Expression inside third brackets not an integer");
		}
		else{
			temp->setArrIndex(static_cast<size_t>(index->intValue())); // Setting the index to the index value that we are trying to access
		}
	}
	return temp;
}

SymbolInfo* getConstValue(SymbolInfo* sym, string varType){
	sym->setDecType(VARIABLE);
	sym->setVarType(varType);
	if (varType == FLOAT_TYPE) {
		sym->floatValues.push_back(0);
		sym->fltValue() = static_cast<float>(atof(sym->getName().data()));
	} else if (varType == INT_TYPE) {
		sym->intValues.push_back(0);
		sym->intValue() = atoi(sym->getName().data());
	}
	return sym;
}

SymbolInfo* getAddOpVal(SymbolInfo *left, SymbolInfo *op, SymbolInfo *right){
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE) {
		printError("Operand of void type.");
		return nullptr;
	}
	string addop = op->getName();
	SymbolInfo* opVal = new SymbolInfo("","");
	if(left->getVarType()==FLOAT_TYPE || right->getVarType()==FLOAT_TYPE){
		getConstValue(opVal,FLOAT_TYPE);
	}
	else{
		getConstValue(opVal,INT_TYPE);
	}
	if(addop=="+"){
		if(left->getVarType()==FLOAT_TYPE){
			if(right->getVarType()==INT_TYPE){
				opVal->fltValue() = left->fltValue() + right->intValue();
			}
			else{
				opVal->fltValue() = left->fltValue() + right->fltValue();
			}
		}
		else if(right->getVarType()==FLOAT_TYPE){
			if (left->getVarType()==INT_TYPE) {
				opVal->fltValue() = left->intValue() + right->fltValue();
			} else {
				opVal->fltValue() = left->fltValue() + right->fltValue();
			}
		}
		else if (right->getVarType()==INT_TYPE && left->getVarType()==INT_TYPE){
			opVal->setVarType(INT_TYPE);
			opVal->intValue() = left->intValue()+right->intValue();
		}
	}
	else if(addop=="-"){
		if(left->getVarType()==FLOAT_TYPE){
			if(right->getVarType()==INT_TYPE){
				opVal->fltValue() = left->fltValue() - right->intValue();
			}
			else{
				opVal->fltValue() = left->fltValue() - right->fltValue();
			}
		}
		else if(right->getVarType()==FLOAT_TYPE){
			if (left->getVarType()==INT_TYPE) {
				opVal->fltValue() = left->intValue() - right->fltValue();
			} else {
				opVal->fltValue() = left->fltValue() - right->fltValue();
			}
		}
		else if (right->getVarType()==INT_TYPE && left->getVarType()==INT_TYPE){
			opVal->setVarType(INT_TYPE);
			opVal->intValue() = left->intValue() - right->intValue();
		}
	}
	return opVal;
}

SymbolInfo* getAssignExpVal(SymbolInfo* left, SymbolInfo* right){ // Handles assignment expressions e.g. x=2
	if(left->getVarType()==VOID_TYPE || right->getVarType()==VOID_TYPE){
		printError("Assign Operation on void type");
		return nullptr;
	}
	if(left->isVariable()){
		if(right->getVarType()==INT_TYPE){
			if(left->getVarType()==FLOAT_TYPE){
				printWarning("Assigning integer value to float");
			}
			left->setVarValue(right->intValue());
		}
		else{
			if(left->getVarType()==INT_TYPE){
				printWarning("Assigning float value to integer");
			}
			left->setVarValue(right->fltValue());
		}
	}
	return left;
}

void checkFuncReturnType(SymbolInfo *sym) {
	if (currentFunc != nullptr && currentFunc->getFuncRetType() != sym->getVarType()) {
		printError(currentFunc->getName() + ": function return type does not match with return expression type");
	}
}

void createScope(){
	scope = symbolTable->createScopeTable(NoOfBuckets);
	for(auto param: parameters){
		insertIntoSymbolTable(param);
	}
	clearFunctionParam();
}
`
void exitScope(){
	currentFunc = nullptr;
	symbolTable->printAllScope(logout);
	symbolTable->exitScope();
}