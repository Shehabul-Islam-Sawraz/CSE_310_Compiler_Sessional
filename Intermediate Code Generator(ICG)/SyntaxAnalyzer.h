#include <stack>
#include "IntermediateCodeGenerator.h"

#define INFINITY_INT numeric_limits<int>::max();
#define INFINITY_FLOAT numeric_limits<float>::infinity()

extern int line_count;
extern int error_count;
extern FILE *yyin;
extern char *yytext;
string variableType;

size_t noOfParam = 0;
size_t totalParams = 0;
vector<string> paramType;
vector<SymbolInfo> parameters;

bool errorRule = false;
bool isReturnedFromFunction = false;
string lookAheadBuf;

int yyparse();
int yylex();

void clearFunctionParam()
{
	paramType.clear();
	parameters.clear();
	noOfParam = 0;
}

SymbolInfo *nullValue()
{
	SymbolInfo *temp = new SymbolInfo("$NULL$", "");
	temp->setDecType(VARIABLE);
	temp->setVarType(INT_TYPE);
	return temp;
}

SymbolInfo *insertIntoSymbolTable(SymbolInfo *symbol)
{
	if (scope == nullptr)
	{
		scope = symbolTable->createScopeTable(NoOfBuckets);
	}
	scope->insertSymbol(symbol->getName(), symbol->getType(), (int)(hashValue(symbol->getName()) % NoOfBuckets), symbol->getVarType(), symbol->getDecType());
	SymbolInfo *temp = symbolTable->lookUp(symbol->getName(), (int)(hashValue(symbol->getName()) % NoOfBuckets));
	temp->setScopeID(scope->getId());
	return temp;
}

SymbolInfo *getSymbolInfoOfType(string type)
{
	variableType = type;
	return new SymbolInfo(variableType, variableType);
}

void printError(string error_msg)
{
	if (error_msg == "")
	{
		error_msg = "Syntax error";
	}
	fprintf(errorout, "Error at line %d: %s\n", line_count, error_msg.data());
	syntax_error_count++;
}

void printWarning(string error_msg)
{
	fprintf(errorout, "Warning at line %d: %s\n", line_count, error_msg.data());
	warning_count++;
}

void yyerror(const char *s)
{
	fprintf(errorout, "Error at line %d: %s\n", line_count, "Syntax error");
	syntax_error_count++;
	setValue(error, popValue(error) + " " + lookAheadBuf);
	errorRule = true;
	lookAheadBuf = yytext;
}

SymbolInfo *insertVar(SymbolInfo *var, bool isArray = false)
{
	if (variableType == VOID_TYPE)
	{ 
		// Semantic error
		printError("Variable type cannot be void");
	}
	else
	{
		if (symbolTable->lookUpCurrentScope(var->getName(), (int)(hashValue(var->getName()) % NoOfBuckets)) != nullptr)
		{
			printError("Multiple declaration of " + var->getName());
		}
		else
		{
			var->setDecType(VARIABLE);
			var->setVarType(variableType);
			SymbolInfo *temp = insertIntoSymbolTable(var);
			temp->setDecType(VARIABLE);
			temp->setVarType(variableType);
			if (!isArray)
			{
				if (temp->getScopeID() != "1" && (error_count + syntax_error_count) == 0)
				{
					string code = "\t\tPUSH BX\t; In line no " + to_string(line_count) + ": " + temp->getName() + " declared";
					temp->setOffset(offset);
					offset += 2;
					addInCodeSegment(code);
				}
				else if (temp->getScopeID() == "1" && (error_count + syntax_error_count) == 0)
				{
					if (variableType == FLOAT_TYPE)
					{
						printError("Float type global variable is not supported!!");
					}
					if ((error_count + syntax_error_count) > 0)
					{
						return nullValue();
					}
					temp->isGlobal = true;
					addGlobalVarInDataSegment(temp->getName());
				}
			}
			return temp;
		}
	}
	return nullValue();
}

void insertFunc(SymbolInfo *func, SymbolInfo *retType)
{
	if (symbolTable->lookUp(func->getName(), (int)(hashValue(func->getName()) % NoOfBuckets)) != nullptr)
	{
		printError("Multiple declaration of " + func->getName());
		clearFunctionParam();
		return;
	}
	if (noOfParam && find(paramType.begin(), paramType.end(), VOID_TYPE) != paramType.end())
	{
		printError("Parameters can't be of void type in function " + func->getName());
		return;
	}
	func->setType("ID");
	func->setDecType(FUNCTION);
	func->setVarType(retType->getName());
	SymbolInfo *temp = insertIntoSymbolTable(func);
	temp->setFuncRetType(retType->getName());
	temp->setparamType(paramType);
	temp->setFuncRetLabel(getLabelForFunction(temp->getName()));
}

void insertArr(SymbolInfo *sym, SymbolInfo *index)
{
	SymbolInfo *arr = insertVar(sym, true);
	arr->setDecType(ARRAY);
	arr->setArrSize(static_cast<size_t>(atoi(index->getName().data()))); // Set the size of the array as defined size
	int arrsize = arr->getArrSize();
	if (arr->getScopeID() != "1" && (error_count + syntax_error_count) == 0)
	{
		string code = "\t\t; In line no " + to_string(line_count) + ": Array named " + arr->getName() + " with size " + to_string(arrsize) + " declared";
		for (int i = 0; i < arrsize; i++)
		{
			code += "\n\t\tPUSH BX";
		}
		code += "\n\t\t;array declared";
		arr->setOffset(offset);
		offset += arrsize * 2;
		addInCodeSegment(code);
	}
	else if (arr->getScopeID() == "1")
	{
		if (variableType == FLOAT_TYPE)
		{
			printError("Float type global variable is not supported!!");
		}
		if ((error_count + syntax_error_count) > 0)
		{
			return;
		}
		arr->isGlobal = true;
		addGlobalVarInDataSegment(arr->getName(), arrsize, true);
	}
}

void addFunctionDef(SymbolInfo *retType, SymbolInfo *func)
{
	SymbolInfo *temp = symbolTable->lookUp(func->getName(), (int)(hashValue(func->getName()) % NoOfBuckets));
	// to prevent f(int x,float,int y){} type defn but f(void){} allowed
	if (!(paramType.size() == 1 && paramType[0] == VOID_TYPE) && paramType.size() != noOfParam)
	{
		printError("Unnamed prototype parameter not allowed in definition of function " + func->getName());
	}
	else if (temp != nullptr)
	{
		if (temp->getDecType() != FUNCTION)
		{
			printError("Multiple declaration of " + temp->getName());
			return;
		}
		else if (temp->getIsFuncDeclared())
		{
			printError("Multiple Definition of " + temp->getName());
			return;
		}
		else if (temp->getFuncRetType() != retType->getName())
		{
			printError("Return type mismatch with function declaration in function " + temp->getName());
			return;
		}
		else if (temp->getparamType().size() != paramType.size())
		{
			printError("Total number of arguments mismatch with declaration in function " + temp->getName());
			return;
		}
		vector<string> v = temp->getparamType();
		bool iserr = false;
		int l = paramType.size();
		for (int i = 0; i < l; i++)
		{
			if (v[i].compare(paramType[i]) != 0)
			{
				stringstream ss;
				ss << (i + 1);
				string s;
				ss >> s;
				printError(s + "th argument mismatch in function " + temp->getName());
				iserr = true;
			}
		}
		if (iserr)
		{
			return;
		}
		temp->setIsFuncDeclared(true);
		currentFunc = temp;
		return;
	}
	insertFunc(func, retType);
	func = symbolTable->lookUp(func->getName(), (int)(hashValue(func->getName()) % NoOfBuckets));
	func->setIsFuncDeclared(true);
	currentFunc = func;
}

void insertIntoParamType(SymbolInfo *var)
{
	int l = parameters.size();
	string str = var->getName();
	for (int i = 0; i < l; i++)
	{
		if ((&parameters[i])->getName().compare(str) == 0)
		{
			printError("Multiple declaration of " + str + " in parameters");
			break;
		}
	}
	paramType.push_back(variableType);
	noOfParam++;
	totalParams++;
	var->setDecType(VARIABLE);
	var->setVarType(variableType);
	SymbolInfo *temp = new SymbolInfo(var->getName(), var->getType());
	temp->setDecType(VARIABLE);
	temp->setVarType(variableType);
	parameters.push_back(*temp);
}

SymbolInfo *getVariable(SymbolInfo *sym)
{
	SymbolInfo *temp = symbolTable->lookUp(sym->getName(), (int)(hashValue(sym->getName()) % NoOfBuckets));
	if (temp == nullptr)
	{
		printError("Undeclared variable " + sym->getName());
	}
	else if (!temp->isVariable())
	{
		if (temp->isArray())
		{
			printError("Type mismatch, " + sym->getName() + " is an array");
		}
		else if (temp->isFunction())
		{
			printError("Type mismatch, " + sym->getName() + " is an function");
		}
		else
		{
			printError("Type mismatch, " + sym->getName() + " is not a variable");
		}
	}
	else
	{
		// A variable used inside a function must be used as a temporary variable
		SymbolInfo *var = new SymbolInfo(temp->getName(), temp->getType());
		var->setDecType(temp->getDecType());
		var->setVarType(temp->getVarType());
		var->setType(TEMPORARY_TYPE);
		var->setOffset(temp->getOffset());
		var->isGlobal = temp->isGlobal;
		string code = "";
		if (var->isGlobal)
		{
			code += "\t\tPUSH " + var->getName() + "; Pushing global variable to stack for expression evaluation";
		}
		else
		{
			int x = -1 * var->getOffset();
			code += "\t\tPUSH [BP + " + to_string(x) + "]\t; " + var->getName() + " pushed for expression evaluation" + NEWLINE;
		}
		addInCodeSegment(code);
		return var;
	}
	return nullValue();
}

void checkExistance(SymbolInfo *sym)
{
	SymbolInfo *temp = symbolTable->lookUp(sym->getName(), (int)(hashValue(sym->getName()) % NoOfBuckets));
	if (temp == nullptr)
	{
		printError("Undeclared variable " + sym->getName());
	}
}

void printIdValue(SymbolInfo *sym){
	SymbolInfo *temp = symbolTable->lookUp(sym->getName(), (int)(hashValue(sym->getName()) % NoOfBuckets));
	printId(temp);
}

SymbolInfo *getArrVar(SymbolInfo *sym, SymbolInfo *index)
{
	SymbolInfo *temp = symbolTable->lookUp(sym->getName(), (int)(hashValue(sym->getName()) % NoOfBuckets));
	if (temp == nullptr)
	{
		printError("Undeclared variable " + sym->getName());
		return nullValue();
	}
	else
	{
		if (!temp->isArray())
		{
			printError(sym->getName() + " is not an array");
		}
		else if (index->getVarType() != INT_TYPE)
		{
			printError("Expression inside third brackets not an integer");
		}
		else
		{
			temp->setArrIndex(index->getName()); // Setting the index to the index value that we are trying to access
		}
	}
	// A variable used inside a function must be used as a temporary variable
	SymbolInfo *var = new SymbolInfo(temp->getName(), temp->getType());
	var->setDecType(temp->getDecType());
	var->setVarType(temp->getVarType());
	var->setArrSize(temp->getArrSize());
	var->setArrIndex(temp->getArrIndex());
	var->setType(TEMPORARY_TYPE);
	var->setOffset(temp->getOffset());
	var->isGlobal = temp->isGlobal;
	evaluateArrayVariable(var, index->getName());
	return var;
}

void showFuncRetWarning() {
	if(currentFunc!=nullptr && isReturnedFromFunction==false && currentFunc->getFuncRetType()!=VOID_TYPE){
		printError(currentFunc->getName() + " function with return type " + currentFunc->getFuncRetType() + " has no return statement");
	}
}

void endFuncDef(bool endProc = false, string name = "", string retType = "")
{
	showFuncRetWarning();
	offset = offsets.back();
	offsets.pop_back();
	if (endProc)
	{
		if(retType == VOID_TYPE){
			endProcedure(name, retType);
		}
	}
	totalParams = 0;
	currentFunc = nullptr;
	isReturnedFromFunction = false;
}

SymbolInfo *getConstValue(SymbolInfo *sym, string varType)
{
	sym->setDecType(VARIABLE);
	sym->setVarType(varType);
	if(varType == INT_TYPE){
		string code = "";
        code += "\t\tPUSH " + sym->getName() + "\t; Pushing constant value in stack";
		addInCodeSegment(code);
	}
	return sym;
}

SymbolInfo *getConstValue(string val, string varType)
{
	SymbolInfo *sym = new SymbolInfo(val, "CONST");
	sym->setDecType(VARIABLE);
	sym->setVarType(varType);
	if (varType == FLOAT_TYPE)
	{
		sym->setType("CONST_FLOAT");
	}
	else if (varType == INT_TYPE)
	{
		sym->setType("CONST_INT");
	}
	return sym;
}

SymbolInfo *getUnaryOpVal(SymbolInfo *op, SymbolInfo *sym)
{
	if (sym->getVarType() == VOID_TYPE)
	{
		printError("Invalid Operand for Unary Operation.Operand can't be void");
		return nullValue();
	}
	string uniop = op->getName();
	SymbolInfo *opVal = new SymbolInfo(op->getName() + sym->getName(), TEMPORARY_TYPE);
	opVal = getConstValue(opVal->getName(), sym->getVarType());
	addUnaryOpAsmCode(sym,uniop);
	return opVal;
}

SymbolInfo *getNotOpVal(SymbolInfo *sym)
{
	if (sym->getVarType() == VOID_TYPE)
	{
		printError("Invalid Operand for Logical Not Operation");
		return nullValue();
	}
	SymbolInfo *opVal = new SymbolInfo("!" + sym->getName(), TEMPORARY_TYPE);
	opVal = getConstValue(opVal->getName(), INT_TYPE);
	addNotOpAsmCode(sym);
	return opVal;
}

SymbolInfo *getAddOpVal(SymbolInfo *left, SymbolInfo *op, SymbolInfo *right)
{
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Operand of void type.");
		return nullValue();
	}
	string addop = op->getName();
	SymbolInfo *opVal = new SymbolInfo(left->getName() + op->getName() + right->getName(), TEMPORARY_TYPE);
	if (left->getVarType() == FLOAT_TYPE || right->getVarType() == FLOAT_TYPE)
	{
		opVal = getConstValue(opVal->getName(), FLOAT_TYPE);
	}
	else
	{
		opVal = getConstValue(opVal->getName(), INT_TYPE);
	}
	addAddOpAsmCode(addop, left, right);
	return opVal;
}

SymbolInfo *getMulOpVal(SymbolInfo *left, SymbolInfo *op, SymbolInfo *right)
{
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Operand of void type.");
		return nullValue();
	}
	string mulop = op->getName();
	if (mulop == "%" && (left->getVarType() != INT_TYPE || right->getVarType() != INT_TYPE))
	{
		printError("Non-Integer operand on modulus operator");
		return nullValue();
	}
	if (left->getVarType() != right->getVarType())
	{
		printWarning("Type mismatch");
	}
	SymbolInfo *opVal = new SymbolInfo(left->getName() + op->getName() + right->getName(), TEMPORARY_TYPE);
	if (left->getVarType() == FLOAT_TYPE || right->getVarType() == FLOAT_TYPE)
	{
		opVal = getConstValue(opVal->getName(), FLOAT_TYPE);
	}
	else
	{
		opVal = getConstValue(opVal->getName(), INT_TYPE);
	}
	if (mulop == "%")
	{
		if (left->getVarType() == INT_TYPE && right->getVarType() == INT_TYPE && right->getName() == "0")
		{
			printError("Modulus by Zero");
		}
	}
	if (mulop == "/" && right->getName() == "0")
	{
		printError("Divide by zero");
	}

	addMulOpAsmCode(mulop,left,right);
	return opVal;
}

void getAssignExpVal(SymbolInfo *left, SymbolInfo *right)
{
	// Handles assignment expressions e.g. x=2
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Assign Operation on void type");
	}
	if (left->isVariable())
	{
		if (right->getVarType() == INT_TYPE)
		{
			if (left->getVarType() == FLOAT_TYPE)
			{
				printWarning("Assigning integer value to float");
			}
		}
		else
		{
			if (left->getVarType() == INT_TYPE)
			{
				printWarning("Assigning float value to integer");
			}
		}
		addAssignExpAsmCode(left, right);
	}
	else if (left->isArray())
	{
		if (right->getVarType() == INT_TYPE)
		{
			if (left->getVarType() == FLOAT_TYPE)
			{
				printWarning("Assigning integer value to float type array");
			}
		}
		else
		{
			if (left->getVarType() == INT_TYPE)
			{
				printWarning("Assigning float value to integer type array");
			}
		}
		addAssignExpAsmCode(left, right);
	}
	else if (left->isFunction())
	{
		printError("Can't assign value to a function");
	}
}

SymbolInfo *getRelOpVal(SymbolInfo *left, SymbolInfo *op, SymbolInfo *right)
{
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Can't compare with void type expressions");
		return nullValue();
	}
	string relop = op->getName();
	SymbolInfo *opVal = new SymbolInfo(left->getName() + op->getName() + right->getName(), TEMPORARY_TYPE);
	opVal->setVarType(INT_TYPE);
	addRelOpAsmCode(op->getName(), left, right);
	if (relop == "==" && left->getVarType() != right->getVarType())
	{
		printWarning("Comparision between two different types");
	}
	return opVal;
}

SymbolInfo *getLogicOpVal(SymbolInfo *left, SymbolInfo *op, SymbolInfo *right)
{
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Can't compare with void type expressions");
		return nullValue();
	}
	string logicOp = op->getName();
	SymbolInfo *opVal = new SymbolInfo(left->getName() + op->getName() + right->getName(), TEMPORARY_TYPE);
	opVal->setVarType(INT_TYPE);
	addLogicOpAsmCode(op->getName(), left, right);
	if (left->getVarType() != right->getVarType())
	{
		printWarning("Comparision between two different types");
	}
	return opVal;
}

SymbolInfo *getFuncCallValue(SymbolInfo *sym)
{
	SymbolInfo *temp = symbolTable->lookUp(sym->getName(), (int)(hashValue(sym->getName()) % NoOfBuckets));
	SymbolInfo *ans = nullValue();
	if (temp == nullptr)
	{
		printError("Undeclared function " + sym->getName());
	}
	else if (!temp->isFunction())
	{
		printError(sym->getName() + " is not a function");
	}
	else if (!temp->getIsFuncDeclared())
	{
		printError(sym->getName() + " does not have a body");
	}
	else
	{
		if (temp->getparamType().size() != paramType.size())
		{
			printError("Total number of arguments mismatch in function " + temp->getName());
		}
		else
		{
			vector<string> v = temp->getparamType();
			int l = paramType.size();
			for (int i = 0; i < l; i++)
			{
				if (v[i].compare(paramType[i]) != 0)
				{
					stringstream ss;
					ss << (i + 1);
					string s;
					ss >> s;
					printError(s + "th argument mismatch in function " + temp->getName());
				}
			}
		}
		ans = getConstValue("", temp->getFuncRetType());
	}

	string code = "";
	code += callFunction(sym->getName());
	code += "\t\tPUSH AX" + string("\t; Return value of function named ") + sym->getName() + " pushed in stack" + NEWLINE;
	addInCodeSegment(code);

	clearFunctionParam();
	return ans;
}

SymbolInfo *getINDECOpVal(SymbolInfo *sym, string op, string type)
{
	SymbolInfo *opVal = new SymbolInfo("", "");
	opVal = getConstValue("", sym->getVarType());
	addIncDecAsmCode(sym, op, type);
	return opVal;
}

void checkFuncReturnType(SymbolInfo *sym)
{
	isReturnedFromFunction = true;
	if (currentFunc != nullptr && currentFunc->getFuncRetType() != sym->getVarType())
	{
		printError("Function return type does not match with return expression type in function " + currentFunc->getName());
	}
}

void checkFuncReturnType()
{
	isReturnedFromFunction = true;
	if (currentFunc != nullptr && currentFunc->getFuncRetType() != "VOID")
	{
		printError("Function return type does not match with return expression type in function " + currentFunc->getName());
	}
}

void createScope()
{
	scope = symbolTable->createScopeTable(NoOfBuckets);
	offsets.push_back(offset);
	offset = 4; // We have to start with offset 4 for parameters as when we call a function then an extra push happens. 
				// So to nullify that we have to take extra offset by 2.
	reverse(parameters.begin(), parameters.end());
	for (auto param : parameters)
	{
		SymbolInfo *temp = insertIntoSymbolTable(&param);
		temp->setOffset(-1* offset); // As in function first parameters are pushed then Bp is pushed. So offset must be positive
								  // w.r.t BP (We are using negative as while working with offset we have multiplied bby -1
								  // everywhere for making general rule)
		offset += 2;
	}
	reverse(parameters.begin(), parameters.end());
	clearFunctionParam();
	offset = 2;
}

void exitScope()
{
	symbolTable->printAllScope(logout);
	fprintf(logout, "\n\n");
	scope = symbolTable->exitScope();
}