#include <stack>
#include "IntermediateCodeGenerator.h"

#define INFINITY_INT numeric_limits<int>::max();
#define INFINITY_FLOAT numeric_limits<float>::infinity()

FILE *logout, *errorout, *parserout;
extern int line_count;
extern int error_count;
extern FILE *yyin;
extern char *yytext;
string variableType;
size_t syntax_error_count = 0;
size_t warning_count = 0;
size_t offset = 2;

size_t noOfParam = 0;
size_t totalParams = 0;
vector<string> paramType;
vector<SymbolInfo> parameters;
vector<int> offsets;

bool errorRule = false;
bool isReturnedFromFunction = false;
string lookAheadBuf;

int yyparse();
int yylex();

// string formatCode(string code){
// 	string formattedCode = code;
// 	while (replaceAll(formattedCode, " ;", ";"));
// 	while (replaceAll(formattedCode, " ,", ","));
// 	replaceAll(formattedCode, ";", ";\n");
// 	replaceAll(formattedCode, ";\n\n", ";\n");
// 	while (replaceAll(formattedCode, "( ", "("));
// 	while (replaceAll(formattedCode, " (", "("));
// 	while (replaceAll(formattedCode, ") ", ")"));
// 	while (replaceAll(formattedCode, " )", ")"));
// 	while (replaceAll(formattedCode, "\n{", "{"));
// 	while (replaceAll(formattedCode, "{ ", "{"));
// 	while (replaceAll(formattedCode, " {", "{"));
// 	while (replaceAll(formattedCode, "} ", "}"));
// 	while (replaceAll(formattedCode, " }", "}"));
// 	replaceAll(formattedCode, "{", "\n{\n");
// 	replaceAll(formattedCode, "}", "\n}\n");
// 	while (replaceAll(formattedCode, "\n\n}", "\n}"));
// 	//while (replaceAll(formattedCode, "}\n\n", "}\n"));
// 	while (replaceAll(formattedCode, "  = ", " = "));
// 	while (replaceAll(formattedCode, " =  ", " = "));
// 	while (replaceAll(formattedCode, "  == ", " == "));
// 	while (replaceAll(formattedCode, " ==  ", " == "));
// 	//replaceAll(formattedCode, "(", " ( ");
// 	//replaceAll(formattedCode, ")", " ) ");
// 	while (replaceAll(formattedCode, " ++", "++"));
// 	while (replaceAll(formattedCode, " --", "--"));
// 	while (replaceAll(formattedCode, "\n ", "\n"));
// 	while (replaceAll(formattedCode, " \n", "\n"));
// 	while (replaceAll(formattedCode, "  ", " "));
// 	return formattedCode;
// }

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
	temp->intValues.push_back(0);
	temp->floatValues.push_back(0);
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
	{ // Semantic error
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
			// addVarInDataSegment(temp->getName());
			if (!isArray)
			{
				if (symbolTable->getCurrentScope()->getId() != "1" && (error_count + syntax_error_count) == 0)
				{
					// writeInCodeSegment("\t\tPUSH BX    ;line no " + to_string(lineCount) + ": " + idName + " declared");
					string code = "\t\tPUSH AX\t; In line no " + to_string(line_count) + ": " + temp->getName() + " declared";
					temp->setOffset(offset);
					offset += 2;
					addInCodeSegment(code);
				}
				else if (symbolTable->getCurrentScope()->getId() == "1")
				{
					if (variableType == FLOAT_TYPE)
					{
						printError("Float type global variable is not supported!!");
					}
					if ((error_count + syntax_error_count) == 0)
					{
						return nullValue();
					}
					temp->isGlobal = true;
					// temp->setOffset(-1);
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
	// int l = arr->getArrSize();
	// for(int i=0;i<l;i++){
	// 	arr->intValues.push_back(0);
	// 	arr->floatValues.push_back(0);
	// }
	int arrsize = arr->getArrSize();
	if (symbolTable->getCurrentScope()->getId() != "1" && (error_count + syntax_error_count) == 0)
	{
		// writeInCodeSegment("\t\tPUSH BX    ;line no " + to_string(lineCount) + ": " + idName + " declared");
		string code = "\t\tIn line no " + to_string(line_count) + ": Array named " + arr->getName() + " with size " + to_string(arrsize) + " declared";
		for (int i = 0; i < arraySize; i++)
		{
			code += "\n\t\tPUSH AX";
		}
		code += "\n\t\t;array declared";
		arr->setOffset(offset);
		offset += arrsize * 2;
		addInCodeSegment(code);
	}
	else if (symbolTable->getCurrentScope()->getId() == "1")
	{
		if (variableType == FLOAT_TYPE)
		{
			printError("Float type global variable is not supported!!");
		}
		if ((error_count + syntax_error_count) == 0)
		{
			return nullValue();
		}
		arr->isGlobal = true;
		// arr->setOffset(-1);
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
		// totalParams = 0;
		temp->setIsFuncDeclared(true);
		currentFunc = temp;
		return;
	}
	// totalParams = 0;
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
			code += "\t\tPUSH [BP + " + to_string(-1 * var->getOffset()) + "];" + var->getName() + " pushed for expression evaluation";
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
			// temp->setArrIndex(static_cast<size_t>(index->intValue())); // Setting the index to the index value that we are trying to access
			temp->setArrIndex(index->getName());
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
	// string code = "";
	// code += "\t\t; At line no " + to_string(line_count) + ": getting value of " + var->getName() + "[" + index->getName() + "]\n";
	// code += "\t\tPOP BX" + "\t; Getting index of the array\n";
	// code += "\t\tSHL BX, 1" + "\t; Multiplying index by 2 to match size of a word\n";
	// if(var->isGlobal){
	// 	code += "\t\tMOV AX, " + var->getName() + "[BX]" + "\t; Getting the value of the array at index BX\n";
	// }
	// else{
	// 	code += "SUB BX, " + to_string(var->getOffset()) + "\t; Adding the offset of the array to get the offset of array element\n";
	// 	code += "ADD BX, BP" + "\t; Adding BP to BX to get the address of the array\n";
	//     code += "\t\tMOV AX, [BX]" + "\t; Getting the value of the array at address BX\n";
	// }
	// // Pushing the index and value of the array element on the stack
	// // This will allow the ASSIGNOP and INCOP to use it later
	// code += "\t\tPUSH AX" + "\t; Pushing the value of the array element at index " + index->getName() + "\n";
	// code += "\t\tPUSH BX" + "\t; Pushing the index of the array\n";
	// addInCodeSegment(code);
	evaluateArrayVariable(var, index->getName());
	return var;
}

SymbolInfo *endFuncDef(bool endProc = false, string name = "", string retType = "")
{
	SymbolInfo *temp = new SymbolInfo("", TEMPORARY_TYPE);
	offset = offsets.back();
	offsets.pop_back();
	if (endProc)
	{
		endProcedure(name, retType);
	}
	totalParams = 0;
	currentFunc = nullptr;
	return temp;
}

SymbolInfo *getConstValue(SymbolInfo *sym, string varType)
{
	sym->setDecType(VARIABLE);
	sym->setVarType(varType);
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
		sym->floatValues.push_back(0);
		sym->fltValue() = static_cast<float>(atof(sym->getName().data()));
	}
	else if (varType == INT_TYPE)
	{
		sym->setType("CONST_INT");
		sym->intValues.push_back(0);
		sym->intValue() = atoi(sym->getName().data());
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
	SymbolInfo *opVal = new SymbolInfo("", "");
	opVal = getConstValue(opVal, sym->getVarType());
	if (sym->getVarType() == FLOAT_TYPE)
	{
		opVal->fltValue() = (uniop == "+") ? (sym->fltValue()) : -(sym->fltValue());
	}
	else if (sym->getVarType() == INT_TYPE)
	{
		opVal->intValue() = (uniop == "+") ? (sym->intValue()) : -(sym->intValue());
	}
	return opVal;
}

SymbolInfo *getNotOpVal(SymbolInfo *sym)
{
	if (sym->getVarType() == VOID_TYPE)
	{
		printError("Invalid Operand for Logical Not Operation");
		return nullValue();
	}
	SymbolInfo *opVal = new SymbolInfo("", "");
	opVal = getConstValue(opVal, INT_TYPE);
	int ans = 0;
	if (sym->getVarType() == INT_TYPE)
	{
		ans = sym->intValue();
	}
	else if (sym->getVarType() == FLOAT_TYPE)
	{
		ans = (int)sym->fltValue();
	}
	opVal->intValue() = !ans;
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
	SymbolInfo *opVal = new SymbolInfo(newTemp(), TEMPORARY_TYPE);
	if (left->getVarType() == FLOAT_TYPE || right->getVarType() == FLOAT_TYPE)
	{
		opVal = getConstValue(opVal, FLOAT_TYPE);
	}
	else
	{
		opVal = getConstValue(opVal, INT_TYPE);
	}
	// opVal->code = left->code + addop + right->code;
	// opVal->code += addAddOpAsmCode(addop, opVal->getName(), left, right);
	addAddOpAsmCode(addop, left, right);
	return opVal;
}

string getMulopOperator(string mulop)
{
	if (mulop == "%")
	{
		return "MODULUS";
	}
	else if (mulop == "*")
	{
		return "MULTIPLICATION";
	}
	else
	{
		return "DIVISION";
	}
	return "";
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
	SymbolInfo *opVal = new SymbolInfo("", "");
	if (left->getVarType() == FLOAT_TYPE || right->getVarType() == FLOAT_TYPE)
	{
		opVal = getConstValue(opVal, FLOAT_TYPE);
	}
	else
	{
		opVal = getConstValue(opVal, INT_TYPE);
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
	string operation = getMulopOperator();
	string code = "";
	code += "\t\t; " + operation + " operation between " + left->getName() " and " + right->getName() + NEWLINE;
	code += "\t\tPOP BX" + "\t; " + right->getName() + " popped from stack" + NEWLINE;
	code += "\t\tPOP AX" + "\t; " + left->getName() + " popped from stack" + NEWLINE;
	if (mulop == "*")
	{
		// AX = AX * BX
		code += "\t\tIMUL BX" + "\t; Multiplying " + left->getName() + " and " + right->getName() + NEWLINE;
	}
	else
	{
		code += "\t\tXOR DX, DX" + "\t; Setting value of DX to 0" + NEWLINE;
		code += "\t\tIDIV BX" + "\t; Dividing " + left->getName() + " by " + right->getName() + NEWLINE;
		// AX = AX / BX and DX = AX % BX
		if (mulop == "%")
		{
			code += "\t\tMOV AX, DX" + "\t; Saving remainder after division from DX to AX" + NEWLINE;
		}
	}
	code += "\t\tPUSH AX" + "\t; Saving result of " + left->getName() + mulop + right->getName() + " in stack" + NEWLINE;
	addInCodeSegment(code);
	return opVal;
}

SymbolInfo *getAssignExpVal(SymbolInfo *left, SymbolInfo *right)
{
	// Handles assignment expressions e.g. x=2
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Assign Operation on void type");
		return nullValue();
	}
	string code = "";
	code += "\t\tPOP BX" + "\t; " + right->getName() + " popped from stack\n";
	if (left->isVariable())
	{
		if (right->getVarType() == INT_TYPE)
		{
			if (left->getVarType() == FLOAT_TYPE)
			{
				printWarning("Assigning integer value to float");
				// left->setVarValue(static_cast<float>(right->intValue()*1.0));
			}
			// else{
			// 	left->setVarValue(right->intValue());
			// }
		}
		else
		{
			if (left->getVarType() == INT_TYPE)
			{
				printWarning("Assigning float value to integer");
				// left->setVarValue(static_cast<int>(right->fltValue()));
			}
			// else{
			// 	left->setVarValue(right->fltValue());
			// }
		}
		if (left->isGlobal)
		{
			code += "\t\tMOV " + left->getName() + ", BX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + NEWLINE;
		}
		else
		{
			code += "\t\tMOV [BP + " + to_string(-1 * left->getOffset()) + "], BX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + NEWLINE;
		}
	}
	else if (left->isArray())
	{
		code += "\t\tPOP AX" + "\t; Index of the array popped\n";
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
		code += "\t\tPUSH BP" + "\t; Saving value of BP in stack, so that we can restore it's value later" + NEWLINE;
		code += "\t\tMOV BP, AX" + "\t; Saving address of the array index in BP to access array from stack" + NEWLINE;

		if (left->isGlobal)
		{
			code += "\t\tMOV " + left->getName() + "[BP], BX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + "[AX]" + NEWLINE;
		}
		else
		{
			code += "\t\tMOV [BP], BX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + "[AX]" + NEWLINE;
		}
		code += "\t\tPOP BP" + "\t; Restoring value of BP" + NEWLINE;
	}
	else if (left->isFunction())
	{
		printError("Can't assign value to a function");
	}
	addInCodeSegment(code);
	return left;
}

SymbolInfo *getRelOpVal(SymbolInfo *left, SymbolInfo *op, SymbolInfo *right)
{
	if (left->getVarType() == VOID_TYPE || right->getVarType() == VOID_TYPE)
	{
		printError("Can't compare with void type expressions");
		return nullValue();
	}
	string relop = op->getName();
	SymbolInfo *opVal = new SymbolInfo("", "");
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
	SymbolInfo *opVal = new SymbolInfo("", "");
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
	clearFunctionParam();
	return ans;
}

SymbolInfo *getINDECOpVal(SymbolInfo *sym, string op, string type)
{
	SymbolInfo *opVal = new SymbolInfo("", "");
	opVal = getConstValue(opVal, sym->getVarType());
	if (op == "++")
	{
		if (sym->getVarType() == INT_TYPE && type == "pre")
		{
			opVal->intValue() = ++sym->intValue();
		}
		else if (sym->getVarType() == FLOAT_TYPE && type == "pre")
		{
			opVal->fltValue() = ++sym->fltValue();
		}
		else if (sym->getVarType() == INT_TYPE && type == "post")
		{
			opVal->intValue() = sym->intValue();
		}
		else if (sym->getVarType() == FLOAT_TYPE && type == "post")
		{
			opVal->fltValue() = sym->fltValue();
		}
		if (sym->getDecType() == VARIABLE)
		{
			if (sym->getVarType() == INT_TYPE)
			{
				sym->intValue() = sym->intValue() + 1;
			}
			else if (sym->getVarType() == FLOAT_TYPE)
			{
				sym->fltValue() = sym->fltValue() + 1;
			}
		}
	}
	else if (op == "--")
	{
		if (sym->getVarType() == INT_TYPE && type == "pre")
		{
			opVal->intValue() = --sym->intValue();
		}
		else if (sym->getVarType() == FLOAT_TYPE && type == "pre")
		{
			opVal->fltValue() = --sym->fltValue();
		}
		else if (sym->getVarType() == INT_TYPE && type == "post")
		{
			opVal->intValue() = sym->intValue();
		}
		else if (sym->getVarType() == FLOAT_TYPE && type == "post")
		{
			opVal->fltValue() = sym->fltValue();
		}
		if (sym->getDecType() == VARIABLE)
		{
			if (sym->getVarType() == INT_TYPE)
			{
				sym->intValue() = sym->intValue() - 1;
			}
			else if (sym->getVarType() == FLOAT_TYPE)
			{
				sym->fltValue() = sym->fltValue() - 1;
			}
		}
	}
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
	offset = 2;
	for (auto param : parameters)
	{
		SymbolInfo *temp = insertIntoSymbolTable(&param, true);
		temp->setOffset(-offset); // As in function first parameters are pushed then Bp is pushed. So offset must be positive
								  // w.r.t BP (We are using negative as while working with offset we have multiplied bby -1
								  // everywhere for making general rule)
		offset += 2;
	}
	clearFunctionParam();
}

void exitScope()
{
	if (currentFunc != nullptr && isReturnedFromFunction == false && currentFunc->getFuncRetType() != VOID_TYPE)
	{
		printWarning(currentFunc->getName() + " function with return type " + currentFunc->getFuncRetType() + " has no return statement");
	}
	// currentFunc = nullptr;
	symbolTable->printAllScope(logout);
	fprintf(logout, "\n\n");
	scope = symbolTable->exitScope();
	isReturnedFromFunction = false;
}