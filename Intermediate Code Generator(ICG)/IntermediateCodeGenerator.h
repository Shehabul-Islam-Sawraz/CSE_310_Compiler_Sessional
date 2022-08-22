#include "SymbolTable.h"
#include <bits/stdc++.h>

#define DEFINE_WORD " DW "
#define ARRAY_DUP " DUP(0)"
#define NEWLINE string("\r\n")

string codesegment, datasegment;
ofstream asmFile, optimizedFile;
FILE *logout, *errorout, *parserout;

extern int line_count;
extern int error_count;
size_t syntax_error_count = 0;
size_t warning_count = 0;
int offset = 2;
vector<int> offsets;

int tempCount = 0, maxTempCount = -1;
int labelCount = 0;
string forLoopIncDecCode = "";
string forLoopExpressionCode = "";
bool isForLoop = false;
stack<string> forLoopEndLabel, forLoopStartLabel;
stack<string> elseBlockEndLabel;
stack<string> forLoop;
stack<string> whileLoopStartLabel, whileLoopEndLabel;

ScopeTable *scope = nullptr;                  // Have to declare it in ICG header file
SymbolTable *symbolTable = new SymbolTable(); // Have to declare it in ICG header file

SymbolInfo *currentFunc = nullptr;

const string WHITESPACE = " \n\r\t\f\v";

string ruleName[] = {"init_asm_model", "data_segment", "code_segment", "init_data",
                        "start", "program", "unit", "func_declaration", "func_definition", "parameter_list",
                        "compound_statement", "var_declaration", "type_specifier", "declaration_list", "statements",
                        "statement", "expression_statement", "variable", "expression", "logic_expression",
                        "rel_expression", "simple_expression", "term", "unary_expression", "factor", "argument_list",
                        "arguments"};

enum NONTERMINAL_TYPE
{
    init_asm_model = 0, data_segment, code_segment, init_data,
    start, program, unit, func_declaration, func_definition,
	parameter_list, compound_statement, var_declaration, type_specifier,
	declaration_list, statements, statement, expression_statement,
	variable, expression, logic_expression, rel_expression, simple_expression,
	term, unary_expression, factor, argument_list, arguments, error
};

class NonTerminalHandler
{
private:
    stack<string> nonterminals[NONTERMINAL_TYPE::error + 1];

public:
    string getValue(NONTERMINAL_TYPE nonterminal)
    {
        if (nonterminals[nonterminal].empty())
        {
            return "";
        }
        return nonterminals[nonterminal].top();
    }
    string popValue(NONTERMINAL_TYPE nonterminal)
    {
        if (nonterminals[nonterminal].empty())
        {
            return "";
        }
        string str = nonterminals[nonterminal].top();
        nonterminals[nonterminal].pop();
        return str;
    }
    void setValue(NONTERMINAL_TYPE nonterminal, string value)
    {
        nonterminals[nonterminal].push(value);
    }
};

NonTerminalHandler nonTerminalHandler;

void setValue(NONTERMINAL_TYPE nonterminal, string value)
{
    nonTerminalHandler.setValue(nonterminal, value);
}

string popValue(NONTERMINAL_TYPE nonterminal)
{
    string val = nonTerminalHandler.popValue(nonterminal);
    return val;
}

bool replaceAll(string &source, string toReplace, string replaceBy)
{
    if (source.find(toReplace, 0) == string::npos)
    {
        return false;
    }
    for (string::size_type i = 0; (i = source.find(toReplace, i)) != string::npos;)
    {
        source.replace(i, toReplace.length(), replaceBy);
        i += replaceBy.length();
    }
    return true;
}

string formatCode(string code){
	string formattedCode = code;
	while (replaceAll(formattedCode, " ;", ";"));
	while (replaceAll(formattedCode, " ,", ","));
	replaceAll(formattedCode, ";", ";\n");
	replaceAll(formattedCode, ";\n\n", ";\n");
	while (replaceAll(formattedCode, "( ", "("));
	while (replaceAll(formattedCode, " (", "("));
	while (replaceAll(formattedCode, ") ", ")"));
	while (replaceAll(formattedCode, " )", ")"));
	while (replaceAll(formattedCode, "\n{", "{"));
	while (replaceAll(formattedCode, "{ ", "{"));
	while (replaceAll(formattedCode, " {", "{"));
	while (replaceAll(formattedCode, "} ", "}"));
	while (replaceAll(formattedCode, " }", "}"));
	replaceAll(formattedCode, "{", "\n{\n");
	replaceAll(formattedCode, "}", "\n}\n");
	while (replaceAll(formattedCode, "\n\n}", "\n}"));
	//while (replaceAll(formattedCode, "}\n\n", "}\n"));
	while (replaceAll(formattedCode, "  = ", " = "));
	while (replaceAll(formattedCode, " =  ", " = "));
	while (replaceAll(formattedCode, "  == ", " == "));
	while (replaceAll(formattedCode, " ==  ", " == "));
	//replaceAll(formattedCode, "(", " ( ");
	//replaceAll(formattedCode, ")", " ) ");
	while (replaceAll(formattedCode, " ++", "++"));
	while (replaceAll(formattedCode, " --", "--"));
	while (replaceAll(formattedCode, "\n ", "\n"));
	while (replaceAll(formattedCode, " \n", "\n"));
	while (replaceAll(formattedCode, "  ", " "));
	return formattedCode;
}

/*bool isNumber(string str)
{
    for (char c : str) {
        if (isdigit(c) == 0){
            return false;
        }
    }
    return true;
}

int strToInt(string str)
{
    stringstream ss; 
    int num;
    ss << str;
    ss >> num;
    return num;
}

string& ltrim(string &s)
{
    auto it = find_if(s.begin(), s.end(),
                    [](char c) {
                        return !std::isspace<char>(c, std::locale::classic());
                    });
    s.erase(s.begin(), it);
    return s;
}

vector<string> split(string s){
    vector<string> tokens;
    string token = "";
    string str = "";
    for(int i=0;i<s.length();i++){
        if(s[i]!='\t'){
            str += s[i];
        }
    }
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ' ' || str[i] == ',' || str[i] == '\t') {
            if (token != "") {
                tokens.push_back(token);
                token = "";
            }
        }
        else {
            token += str[i];
        }
    }

    if (token != "") {
        tokens.push_back(token);
    }

    return tokens;
}

void optimizeCodeSegment()
{
    string line, nextLine;
	vector<string> portions, nextPortions;
    asmFile.open("code.asm");
    vector<string> lineVector;
    while (getline(asmFile,line)){
        lineVector.push_back(line);
    }
		
    for (int i = 0; i < lineVector.size()-1; i++)
    {
        if((split(lineVector[i]).size()==0)){
            continue;
        }
        
        line=lineVector[i];
        nextLine=lineVector[i+1];
        portions=split(line);
        nextPortions=split(nextLine);
        if(portions[0]=="PUSH"){
            if(nextPortions[0]=="POP"){ 
                if(portions[1]==nextPortions[1]){ // PUSH AX ; POP AX
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";" + lineVector[i];
                    lineVector[i+1]=";"+lineVector[i+1];
                }
            }
        }
        if(portions[0]=="MOV"){
            if(portions[1]==portions[2]){ // MOV AX, AX
                lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
            }
            if(nextPortions[0]=="MOV"){ 
                if((portions[1]==nextPortions[2]) && (portions[2]==nextPortions[1])){ // MOV AX, BX ; MOV BX, AX
                    lineVector[i+1]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i+1];
                }
                if(portions[1]==nextPortions[1]){ // MOV AX, BX ; MOV AX, CX
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                }
            }
        }
        if(portions[0] == "ADD" || portions[0] == "SUB"){
            if(isNumber(portions[2])){
                if(strToInt(portions[2]) == 0){
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                }
            }
        }
        if(portions[0] == "IMUL" || portions[0] == "IDIV"){
            if(isNumber(portions[2])){
                if(strToInt(portions[2]) == 1){
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                }
            }
        }
        if((portions[0] == "ADD" && nextPortions[0] == "ADD") || (portions[0] == "SUB" && nextPortions[0] == "SUB")){
            if(isNumber(portions[2]) && isNumber(nextPortions[2])){
                if(portions[1]==nextPortions[1]){
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                    int x = strToInt(portions[2]) + strToInt(nextPortions[2]);
                    lineVector[i+1]=portions[0] + " " + portions[1] + ", " + to_string(x);
                }
            }
        }
    }
    for (int i = 0; i < lineVector.size(); i++){
        optimizeCodeSegment <<lineVector[i] << endl;
    }
}*/

void replaceByNewLine(string &str)
{
    replaceAll(str, "\r", "");
    replaceAll(str, "\n", "\r\n");
    if (str[str.size() - 1] != '\n')
    {
        str += "\r\n";
    }
}

uint32_t hashValue(string str)
{
    uint32_t hash = 0;
    for (int c : str)
        hash = c + (hash * 64) + (hash * 65536) - hash;
    return hash;
}

void printRuleAndCode(NONTERMINAL_TYPE nonterminal, string rule){
	fprintf(logout,"Line %d: %s : %s\n",line_count, ruleName[nonterminal].data(), rule.data());
	fprintf(logout,"%s\n", formatCode(nonTerminalHandler.getValue(nonterminal)).data());
}

void writeInDataSegment()
{
    asmFile.close();
    asmFile.open("code.asm");
    asmFile << nonTerminalHandler.getValue(init_asm_model);
    asmFile << nonTerminalHandler.getValue(data_segment);
    asmFile << nonTerminalHandler.getValue(code_segment);
}

void writeInCodeSegment(string str)
{
    asmFile << str;
}

string GET_ASM_VAR_NAME(string var)
{
    SymbolInfo *temp = symbolTable->lookUp(var, (int)(hashValue(var) % NoOfBuckets));
    if (temp == nullptr || temp->getType() != "ID")
    {
        return var;
    }
    return var + temp->getScopeID();
}

void addGlobalVarInDataSegment(string var, int arrsize = 0, bool isArray = false)
{
    // string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + "?";
    string code = "";
    if (!isArray)
    {
        code += "\t" + var + DEFINE_WORD + "?";
    }
    else
    {
        code += "\t" + var + DEFINE_WORD + to_string(arrsize) + ARRAY_DUP;
    }
    replaceByNewLine(code);
    setValue(data_segment, popValue(data_segment) + code);
    writeInDataSegment();
}

void addInCodeSegment(string code)
{
    // If isForLoop = true, then we have got the expression code. We are not
    // adding it to code segment as we want the expression is evaluated after 
    // the for loop statement is executed
    if ((error_count + syntax_error_count) == 0)
    {
        if(!isForLoop){
            string str = code;
            replaceByNewLine(str);
            // codesegment+=str;
            setValue(code_segment, popValue(code_segment) + str);
            writeInCodeSegment(str);
        }
        else{
            forLoopIncDecCode += code;
        }
    }
}

string getLabelForFunction(string func_name)
{
    return func_name + "_EXIT";
}

void init_model()
{
    string code = ".MODEL SMALL" + NEWLINE;
    code += NEWLINE + ".STACk 800H" + NEWLINE;
    setValue(init_asm_model, code);
    setValue(data_segment, NEWLINE + ".DATA" + NEWLINE);
    setValue(code_segment, NEWLINE + ".CODE" + NEWLINE);
    setValue(init_data, NEWLINE + "MOV AX, @DATA" + NEWLINE +
                                  "MOV DS, AX" + NEWLINE);
}

string newTemp()
{
    string temp = "t";
    char c[3];
    sprintf(c, "%d", tempCount);
    temp += c;
    if (tempCount > maxTempCount)
    {
        maxTempCount++;
        addGlobalVarInDataSegment(temp);
    }
    tempCount++;
    return temp;
}

string newLabel()
{
    string lb = "L";
    char b[3];
    sprintf(b, "%d", labelCount);
    labelCount++;
    lb += b;
    return lb;
}

string setSIForArray(string reg, string offset)
{
    string code = "";
    code += "MOV " + reg + ", " + GET_ASM_VAR_NAME(offset) + NEWLINE;
    code += "ADD " + reg + ", " + reg + NEWLINE;
    return code;
}

string arrayMemoryToReg(string command, string reg, string arrayName, string offset)
{
    string code = "";
    code += setSIForArray("SI", offset);
    code += command + " " + reg + ", " + GET_ASM_VAR_NAME(arrayName) + "[SI]" + NEWLINE;
    return code;
}

string memoryToReg(string command, string reg, string memory)
{
    return command + " " + reg + ", " + GET_ASM_VAR_NAME(memory) + NEWLINE;
}

string operatorToReg(string command, string reg, SymbolInfo *sym)
{
    if (sym->isArray())
    {
        return arrayMemoryToReg(command, reg, sym->getName(), sym->getArrIndex());
    }
    else
    {
        return memoryToReg(command, reg, sym->getName());
    }
}

string declareLabel(string label, bool conditionValue)
{
    string code = "";
    code += "\t\t" + label + ":" + NEWLINE;
    if (conditionValue)
    {
        code += "\t\tPUSH 1\t; Saving expression value to be true" + NEWLINE;
    }
    else
    {
        code += "\t\tPUSH 0\t; Saving expression value to be false" + NEWLINE;
    }
    return code;
}

string callFunction(string label){
    return "\t\tCALL " + label + "\t; Function with name " + label + " called" + NEWLINE;
}

string conditionalJump(string condition, string label)
{
    return "\t\t" + condition + " " + label + NEWLINE;
}

string jumpInstant(string label)
{
    return "\t\tJMP " + label + NEWLINE;
}

void addAssignExpAsmCode(SymbolInfo *left, SymbolInfo *right)
{
    string code = "";
	code += "\t\t; At line no " + to_string(line_count) + ": Assigning " + right->getName() + " into " + left->getName() + NEWLINE;
	code += "\t\tPOP AX" + string("\t; ") + right->getName() + " popped from stack" + NEWLINE;

    if (left->isVariable())
	{
        if (left->isGlobal)
		{
			code += "\t\tMOV " + left->getName() + ", AX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + NEWLINE;
		}
		else
		{
            int x = -1 * left->getOffset();
			code += "\t\tMOV [BP + " + to_string(x) + "], AX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + NEWLINE;
		}
    }
    else if (left->isArray())
	{
        code += "\t\tPOP BX" + string("\t; Index of the array popped") + NEWLINE;
        code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack, so that we can restore it's value later") + NEWLINE;
		code += "\t\tMOV BP, BX" + string("\t; Saving address of the array index in BP to access array from stack") + NEWLINE;
		if (left->isGlobal)
		{
			code += "\t\tMOV " + left->getName() + "[BP], AX" + "\t; Value of " + right->getName() + " assigned into " + left->getName() + "[AX]" + NEWLINE;
		}
		else
		{
			code += "\t\tMOV [BP], AX" + string("\t; Value of ") + right->getName() + " assigned into " + left->getName() + "[AX]" + NEWLINE;
		}
		code += "\t\tPOP BP" + string("\t; Restoring value of BP") + NEWLINE;
    }

    addInCodeSegment(code);
}

void addAddOpAsmCode(string op, SymbolInfo *left, SymbolInfo *right)
{
    if (op == "+")
    {
        op = "ADD";
    }
    else if (op == "-")
    {
        op = "SUB";
    }

    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": " + op + " " + left->getName() + " and " + right->getName() + "\n";
    code += "\t\tPOP BX" + string("\t; ") + right->getName() + " popped from stack\n";
    code += "\t\tPOP AX" + string("\t; ") + left->getName() + " popped from stack\n";
    code += "\t\t" + op + " AX, BX\n";
    code += "\t\tPUSH AX" + string("\t; Pushed evaluated value of ") + left->getName() + op + right->getName() + " in the stack\n";
    addInCodeSegment(code);
}

string getRelOpASM(string op)
{
    if (op == ">")
    {
        return "JG";
    }
    else if (op == ">=")
    {
        return "JGE";
    }
    else if (op == "<")
    {
        return "JL";
    }
    else if (op == "<=")
    {
        return "JLE";
    }
    else if (op == "!=")
    {
        return "JnE";
    }
    else if (op == "==")
    {
        return "JE";
    }
    return "";
}

void addRelOpAsmCode(string op, SymbolInfo *left, SymbolInfo *right)
{
    op = getRelOpASM(op);
    string labelIfTrue = newLabel();
    string labelIfFalse = newLabel();

    string code = "";
    code += "\t\t; At line no  " + to_string(line_count) + ": Checking if " + left->getName() + op + right->getName() + "\n";
    code += "\t\tPOP BX" + string("\t; Popped out ") + right->getName() + " from stack\n";
    code += "\t\tPOP AX" + string("\t; Popped out ") + left->getName() + " from stack\n";
    code += "\t\tCMP AX, BX" + string("\t; Comparing ") + left->getName() + " with " + right->getName() + "\n";
    code += conditionalJump(op, labelIfTrue); // If true then do conditional jump
    code += "\t\tPUSH 0" + string("\t; Saving expression value to be false") + NEWLINE;
    code += jumpInstant(labelIfFalse);        // If false then do long jump
    code += declareLabel(labelIfTrue, true);
    code += "\t\t" + labelIfFalse + ":" + NEWLINE + NEWLINE;
    addInCodeSegment(code);
}

void addLogicOpAsmCode(string op, SymbolInfo *left, SymbolInfo *right)
{
    string labelForRightCheck = newLabel();
    string endLabel = newLabel();
    string resultLabel = newLabel();

    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": " + left->getName() + op + right->getName() + NEWLINE;
    code += "\t\tPOP BX" + string("\t; ") + right->getName() + " popped from stack\n";
    code += "\t\tPOP AX" + string("\t; ") + left->getName() + " popped from stack\n";
    code += "\t\tCMP AX, 0" + string("\t; Comparing ") + left->getName() + " and 0\n";
    // If operator is &&, then getting a 0 will cause result to be 0
    // and if the operator is ||, then ggetting a 1 will cause result
    // to be 1
    if (op == "&&")
    {
        code += "\t\t; If " + left->getName() + " is not 0, then check " + right->getName() + ". So, jump to " + labelForRightCheck + "\n";
        code += conditionalJump("JNE", labelForRightCheck);
        code += "\t\tPUSH 0" + string("\t; Saving expression value to be false") + NEWLINE;
    }
    else
    {
        code += "\t\t; if " + left->getName() + " is 0, check " + right->getName() + ". So, jump to " + labelForRightCheck + "\n";
        code += conditionalJump("JE", labelForRightCheck);
        code += "\t\tPUSH 1" + string("\t; Saving expression value to be true") + NEWLINE;
    }
    code += jumpInstant(endLabel);
    code += "\t\t" + labelForRightCheck + ":\n";
    code += "\t\tCMP BX, 0" + string("\t; Comparing ") + right->getName() + " and 0\n";
    // If operator is &&, then getting 0 in right will cause result to be 0
    // and if the operator is ||, then ggetting a 1 in right will cause result
    // to be 1
    if (op == "&&")
    {
        code += "\t\t; If " + right->getName() + " is not 0, the whole expression is true. So, jump to " + resultLabel + NEWLINE;
        code += conditionalJump("JNE", resultLabel);
        code += "\t\tPUSH 0" + string("\t; Saving expression value to be false") + NEWLINE;
    }
    else
    {
        code += "\t\t; If " + right->getName() + " is 0, the whole expression is false. So, jump to " + resultLabel + NEWLINE;
        code += conditionalJump("JE", resultLabel);
        code += "\t\tPUSH 1" + string("\t; Saving expression value to be true") + NEWLINE;
    }
    code += jumpInstant(endLabel);
    if (op == "&&")
    {
        code += declareLabel(resultLabel, true);
        code += "\t\t" + endLabel + ":" + NEWLINE + NEWLINE;
    }
    else
    {
        code += declareLabel(resultLabel, false);
        code += "\t\t" + endLabel + ":" + NEWLINE + NEWLINE;
    }

    addInCodeSegment(code);
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

void addMulOpAsmCode(string mulop, SymbolInfo *left, SymbolInfo *right)
{
    string operation = getMulopOperator(mulop);
	string code = "";
	code += "\t\t; " + operation + " operation between " + left->getName() + " and " + right->getName() + NEWLINE;
	code += "\t\tPOP BX" + string("\t; ") + right->getName() + " popped from stack" + NEWLINE;
	code += "\t\tPOP AX" + string("\t; ") + left->getName() + " popped from stack" + NEWLINE;
	if (mulop == "*")
	{
		// AX = AX * BX
		code += "\t\tIMUL BX" + string("\t; Multiplying ") + left->getName() + " and " + right->getName() + NEWLINE;
	}
	else
	{
		code += "\t\tXOR DX, DX" + string("\t; Setting value of DX to 0") + NEWLINE;
		code += "\t\tIDIV BX" + string("\t; Dividing ") + left->getName() + " by " + right->getName() + NEWLINE;
		// AX = AX / BX and DX = AX % BX
		if (mulop == "%")
		{
			code += "\t\tMOV AX, DX" + string("\t; Saving remainder after division from DX to AX") + NEWLINE;
		}
	}
	code += "\t\tPUSH AX" + string("\t; Saving result of ") + left->getName() + mulop + right->getName() + " in stack" + NEWLINE;
	addInCodeSegment(code);
}

void addUnaryOpAsmCode(SymbolInfo *sym, string uniop)
{
    if (uniop == "-")
	{
		string code = "";
		code += "\t\t; At line no " + to_string(line_count) + ": Negating " + sym->getName() + NEWLINE;
		code += "\t\tPOP AX" + string("\t; ") + sym->getName() + " popped from stack" + NEWLINE;
		code += "\t\tNEG AX" + string("\t; Negating ") + sym->getName() + NEWLINE;
		code += "\t\tPUSH AX" + string("\t; Saving result of -") + sym->getName() + " in stack" + NEWLINE;
		addInCodeSegment(code);
	}
}

void addNotOpAsmCode(SymbolInfo *sym)
{
    string labelFalse = newLabel();
	string endLabel = newLabel();
	string code = "";
	code += "\t\t; At line no " + to_string(line_count) + ": Evaluating !" + sym->getName() + NEWLINE;
	code += "\t\tPOP AX" + string("\t; Popped ") + sym->getName() + " from stack" + NEWLINE;
	code += "\t\tCMP AX,0" + string("\t; Comparing ") + sym->getName() + " with 0" + NEWLINE;
	code += "\t\tJNE " + labelFalse + "\t; Go to label " + labelFalse + " if BX is not 0" + NEWLINE;
	code += "\t\tPUSH 1" + string("\t; Pushing 0 in stack if BX is 0") + NEWLINE;
	code += jumpInstant(endLabel);
	code += declareLabel(labelFalse, false);
	code += "\t\t" + endLabel + ":" + NEWLINE + NEWLINE;
	addInCodeSegment(code);
}

void addIncDecAsmCode(SymbolInfo *sym, string op, string type)
{
    string code = "";
    if(type == "post"){
        code += "\t\t; At line no " + to_string(line_count) + ": Evaluating postfix " + op + " of " + sym->getName() + NEWLINE;
        if(sym->getDecType() == ARRAY){
            code += "\t\tPOP BX" + string("\t; Array index popped from stack") + NEWLINE;
            code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack") + NEWLINE;
            code += "\t\tMOV BP, BX" + string("\t; Saving value of array index in BP") + NEWLINE;
            code += "\t\tMOV AX, [BP]" + string("\t; Saving value of ") + sym->getName() + " in AX" + NEWLINE;
            code += "\t\tPOP BP" + string("\t; Resetting value of BP") + NEWLINE;
        }
        else{
            code += "\t\tPOP AX" + string("\t; Saving value of ") + sym->getName() + " in AX" + NEWLINE;
            code += "\t\tPUSH AX" + string("\t; Pushing the value of ") + sym->getName() + " back to stack" + NEWLINE;
        }

        if(op == "++"){
            code += "\t\tINC AX" + string("\t; Incrementing ") + sym->getName() + NEWLINE;
        }
        else{
            code += "\t\tDEC AX" + string("\t; Decrementing ") + sym->getName() + NEWLINE;
        }
        
        if(sym->isGlobal) {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack") + NEWLINE;
                code += "\t\tMOV BP, BX" + string("\t; Saving value of array index in BP") + NEWLINE;
                code += "\t\tMOV " + sym->getName() + "[BP], AX" + string("\t; Saving the result in stack") + NEWLINE;
                code += "\t\tPOP BP" + string("\t; Resetting value of BP") + NEWLINE;
            }
            else{
                code += "\t\tMOV " + sym->getName() + ", AX" + string("\t; Saving the result in stack") + NEWLINE;
            }
        } 
        else {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack") + NEWLINE;
                code += "\t\tMOV BP, BX" + string("\t; Saving value of array index in BP") + NEWLINE;
                code += "\t\tMOV [BP], AX" + string("\t; Saving the result in stack") + NEWLINE;
                code += "\t\tPOP BP" + string("\t; Resetting value of BP") + NEWLINE;
            }
            else{
                int x = -1 * sym->getOffset();
                code += "\t\tMOV [BP + " + to_string(x) + "], AX" + string("\t; Saving result in stack") + NEWLINE;
            }
        }
    }
    else{
        code += "\t\t; At line no " + to_string(line_count) + ": Evaluating prefix " + op + " of " + sym->getName() + NEWLINE;
        if(sym->getDecType() == ARRAY){
            code += "\t\tPOP BX" + string("\t; Array index popped from stack") + NEWLINE;
        }
        code += "\t\tPOP AX" + string("\t; Saving value of ") + sym->getName() + " in AX" + NEWLINE;

        if(op == "++"){
            code += "\t\tINC AX" + string("\t; Incrementing ") + sym->getName() + NEWLINE;
        }
        else{
            code += "\t\tDEC AX" + string("\t; Decrementing ") + sym->getName() + NEWLINE;
        }
        
        code += "\t\tPUSH AX" + string("\t; Pushing the value of ") + sym->getName() + " back to stack" + NEWLINE;

        if(sym->isGlobal) {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack") + NEWLINE;
                code += "\t\tMOV BP, BX" + string("\t; Saving value of array index in BP") + NEWLINE;
                code += "\t\tMOV " + sym->getName() + "[BP], AX" + string("\t; Saving the result in stack") + NEWLINE;
                code += "\t\tPOP BP" + string("\t; Resetting value of BP") + NEWLINE;
            }
            else{
                code += "\t\tMOV " + sym->getName() + ", AX" + string("\t; Saving the result in stack") + NEWLINE;
            }
        } 
        else {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack") + NEWLINE;
                code += "\t\tMOV BP, BX" + string("\t; Saving value of array index in BP") + NEWLINE;
                code += "\t\tMOV [BP], AX" + string("\t; Saving the result in stack") + NEWLINE;
                code += "\t\tPOP BP" + string("\t; Resetting value of BP") + NEWLINE;
            }
            else{
                int x = -1 * sym->getOffset();
                code += "\t\tMOV [BP + " + to_string(x) + "], AX" + "\t; Saving result in stack" + NEWLINE;
            }
        }
    }

    addInCodeSegment(code);
}

string getExpressionCode(SymbolInfo *sym)
{
    string code = "";
    code += sym->code;
    code += operatorToReg("MOV", "DX", sym);
    code += jumpInstant(currentFunc->getFuncRetLabel());
    return code;
}

void initMainProc()
{
    if ((syntax_error_count + error_count) > 0)
    {
        return;
    }
    string code = "";
    code += "\t\t; DATA SEGMENT INITIALIZATION" + NEWLINE;
    code += "\t\tMOV AX, @DATA\n\t\tMOV DS, AX";
    offset = 2;
    addInCodeSegment(code);
}

void startProcedure(string name)
{
    if ((syntax_error_count + error_count) > 0)
    {
        return;
    }
    string code = "";
    code += "\t" + name + " PROC" + NEWLINE;
    code += "\t\t; Function with name " + name + " started" + NEWLINE;
    if(name != "main"){
        code += "\t\tPUSH BP\n";
        code += "\t\tMOV BP, SP\t; All the offsets of a function depends on the value of BP" + NEWLINE + NEWLINE;
    }
    addInCodeSegment(code);
    if (name == "main")
    {
        initMainProc();
    }
}

void endProcedure(string name, string retType)
{
    if ((syntax_error_count + error_count) > 0)
    {
        return;
    }

    string code = "";
    //code += "\t\t" + getLabelForFunction(name) + ":\n";
    code += NEWLINE + NEWLINE;
    code += "\t\tMOV SP, BP\t; Restoring SP at the end of function\n";
    code += "\t\tPOP BP\t; Restoring BP at the end of function\n";

    if (currentFunc->getFuncRetType() == VOID_TYPE)
    {
        code += "\t\tRET 0" + NEWLINE;
    }
    else if(name != "main")
    {
        vector<string> v = currentFunc->getparamType();
        code += "\t\tRET " + to_string(2 * v.size()) + NEWLINE;
    }
    addInCodeSegment(code);
}

void writeENDPForFunc(string name)
{
    string code = "";
    if (name == "main")
    {
        code += "\t\t; Setting interrupt for function end\n";
        code += "\t\tMOV AH, 4CH\n\t\tINT 21H\n";
    }
    code += "\t" + name + " ENDP" + NEWLINE;
    addInCodeSegment(code);
}

void popArrayFromStack(string reg, SymbolInfo *sym)
{
    // If after "variable: Id" or "variable: Array" rule matches and next rule doesn't match with INCOP or DECOP,
    // and goes to "factor : variable" rule then we no longer need the index of the previously accessed array
    if (sym->getDecType() == ARRAY)
    {
        addInCodeSegment("\t\tPOP BX" + string("\t; Array index popped because it is no longer required"));
    }
}

void evaluateArrayVariable(SymbolInfo *var, string index)
{
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": getting value of " + var->getName() + "[" + index + "]\n";
    code += "\t\tPOP BX" + string("\t; Getting index of the array") + NEWLINE;
    code += "\t\tSHL BX, 1" + string("\t; Multiplying index by 2 to match size of a word") + NEWLINE;
    if (var->isGlobal)
    {
        code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack, so that we can restore it's value later") + NEWLINE;
        code += "\t\tMOV BP, BX" + string("\t; Saving address of the array index in BP to access array from stack") + NEWLINE;
        code += "\t\tMOV AX, " + var->getName() + "[BP]" + string("\t; Getting the value of the array from index BP") + NEWLINE;
    }
    else
    {
        code += "\t\tSUB BX, " + to_string(var->getOffset()) + "\t; Adding the offset of the array to get the offset of array element\n";
        code += "\t\tADD BX, BP" + string("\t; Adding BP to BX to get the address of the array") + NEWLINE;
        code += "\t\tPUSH BP" + string("\t; Saving value of BP in stack, so that we can restore it's value later") + NEWLINE;
        code += "\t\tMOV BP, BX" + string("\t; Saving address of the array index in BP to access array from stack") + NEWLINE;
        code += "\t\tMOV AX, [BP]" + string("\t; Getting the value of the array at address BP") + NEWLINE;
    }
    code += "\t\tPOP BP" + string("\t; Restoring value of BP") + NEWLINE;
    // Pushing the index and value of the array element on the stack
    // This will allow the ASSIGNOP and INCOP to use it later
    code += "\t\tPUSH AX" + string("\t; Pushing the value of the array element at index ") + index + NEWLINE;
    code += "\t\tPUSH BX" + string("\t; Pushing the index of the array") + NEWLINE;
    addInCodeSegment(code);
}

void handleExtraExpressionPush(string name)
{
    // There is always an extra push after every expression
    // So, we need to pop it out after we get a semicolon after an expression
    string code = "";
    code += "\t\tPOP AX" + string("\t; Popped out ") + name + NEWLINE;
    addInCodeSegment(code);
}

void forLoopStart()
{
    // forLoop.push("");
    string startLabel = newLabel();
    forLoopStartLabel.push(startLabel);
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": Starting for loop" + NEWLINE;
    code += "\t\t" + startLabel + ":\t; For loop start label" + NEWLINE;
    addInCodeSegment(code);
}

void forLoopConditionCheck(){
    string labelIfTrue = newLabel();
    string endLabel = newLabel();
    forLoopEndLabel.push(endLabel);
    string code = "";
    // We have already popped AX from stack after getting expression_statement. This AX contains
    // the result of the condition inside for loop
    code += "\t\tCMP AX, 0" + string("\t; Checking if the condition is true or false") + NEWLINE;
    // If AX != 0, then statement will compile, otherwise go to end label
    code += conditionalJump("JNE", labelIfTrue);
    code += "\t\t; If false then jump to the end label of for loop" + NEWLINE;
    code += jumpInstant(endLabel);
    code += "\t\t" + labelIfTrue + ":" + NEWLINE;
    addInCodeSegment(code);
    isForLoop = true; // We are making it true, so that the expression is evaluated after the for loop statement is executed
}

void gotoNextStepInForLoop(string var)
{
    string code = "";
    code += "\t\tPOP AX" + string("\t; Popped ") + var + " from stack" + NEWLINE;
    if(!forLoopStartLabel.empty()){
        code += "\t\tJMP " + forLoopStartLabel.top() + "\t; Jump back to for loop" + NEWLINE;
        forLoopStartLabel.pop();
    }
    if(!forLoopEndLabel.empty()){
        code += "\t\t; End label of for loop" + NEWLINE;
        code += "\t\t" + forLoopEndLabel.top() + ":" + NEWLINE;
        forLoopEndLabel.pop();
    }
    forLoopExpressionCode = forLoopIncDecCode + code;
    forLoop.push(forLoopExpressionCode);
    forLoopIncDecCode = "";
    forLoopExpressionCode = "";
    isForLoop = false;
}

void endForLoop()
{   
    if(!forLoop.empty()){
        addInCodeSegment(forLoop.top());
        forLoop.pop();
    }
    forLoopIncDecCode = "";
    forLoopExpressionCode = "";
}

SymbolInfo* createIfBlock()
{
    string labelIfTrue = newLabel();
    string labelIfFalse = newLabel();
    SymbolInfo *sym = new SymbolInfo(labelIfFalse, TEMPORARY_TYPE);
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": Evaluationg if statement" + NEWLINE;
    code += "\t\tPOP AX" + string("\t; Popped expression value from stack") + NEWLINE;
    code += "\t\tCMP AX, 0" + string("\t; Checking whether expression is true or false") + NEWLINE;
    code += conditionalJump("JNE", labelIfTrue);
    code += "\t\t; If expression is false then jump to the end of if block" + NEWLINE;
    code += jumpInstant(labelIfFalse);
    code += "\t\t" + labelIfTrue + ":" + NEWLINE;
    addInCodeSegment(code);
    return sym;
}

void endIfBlock(string label){
    string code = "";
    code += "\t\t; End label for if statement" + NEWLINE;
    code += "\t\t" + label + ":" + NEWLINE;
    addInCodeSegment(code);
}

void createElseBlock(string label)
{
    string endLabel = newLabel();
    elseBlockEndLabel.push(endLabel);
    string code = "";
    code += "\t\t; If expression was true and statement is evaluated then jump to end label" + NEWLINE;
    code += jumpInstant(endLabel);
    code += "\t\t; Label for else block" + NEWLINE;
    code += "\t\t" + label + ":" + NEWLINE;
    addInCodeSegment(code);
}

void endIfElseBlock()
{
    string code = "";
    if(!elseBlockEndLabel.empty()){
        code += "\t\t; End label for if else statement" + NEWLINE;
        code += "\t\t" + elseBlockEndLabel.top() + ":" + NEWLINE;
        elseBlockEndLabel.pop();
        addInCodeSegment(code);
    }
}

void whileLoopStart()
{
    string startLabel = newLabel();
    whileLoopStartLabel.push(startLabel);
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": Starting while loop" + NEWLINE;
    code += "\t\t" + startLabel + ":\t; While loop start label" + NEWLINE;
    addInCodeSegment(code);
}

void whileLoopConditionCheck(string var){
    string labelIfTrue = newLabel();
    string endLabel = newLabel();
    whileLoopEndLabel.push(endLabel);
    string code = "";
    code += "\t\tPOP AX" + string("\t; Popped ") + var + " from stack" + NEWLINE;
    code += "\t\tCMP AX, 0" + string("\t; Checking if the condition is true or false") + NEWLINE;
    // If AX != 0, then statement will compile, otherwise go to end label
    code += conditionalJump("JNE", labelIfTrue);
    code += "\t\t; If false then jump to the end label of while loop" + NEWLINE;
    code += jumpInstant(endLabel);
    code += "\t\t" + labelIfTrue + ":" + NEWLINE;
    addInCodeSegment(code);
}

void endWhileLoop()
{
    string code = "";
    if(!whileLoopStartLabel.empty()){
        code += "\t\tJMP " + whileLoopStartLabel.top() + "\t; Jump back to while loop" + NEWLINE;
        whileLoopStartLabel.pop();
    }
    if(!whileLoopEndLabel.empty()){
        code += "\t\t; End label of while loop" + NEWLINE;
        code += "\t\t" + whileLoopEndLabel.top() + ":" + NEWLINE;
        whileLoopEndLabel.pop();
    }
    addInCodeSegment(code);
}

void printId(SymbolInfo *sym)
{
    string code = "";
    code += NEWLINE;
    if(sym->isGlobal){
        code += "PUSH " + sym->getName() + "\t; Passing " + sym->getName() + " to PRINT_NUM for printing it" + NEWLINE;
    }
    else{
        int x = -1 * sym->getOffset();
        code += "\t\tPUSH [BP + " + to_string(x) + "]" + "\t; Passing " + sym->getName() + " to PRINT_NUM for printing it" + NEWLINE;
    }
    code += "\t\tCALL PRINT_NUM" + NEWLINE;
    addInCodeSegment(code);
}

void returnFunction()
{
    string code = "";
    code += "\t\tPOP AX" + string("\t; Popped return value and saved it in AX") + NEWLINE;
    addInCodeSegment(code);
    endProcedure(currentFunc->getName(), currentFunc->getFuncRetType());
}

void writePrintNumProc() {
    string code = "";
    code = "\t;printf(n)\n";
    code += 
    "\tPRINT_NUM PROC NEAR\n\
        PUSH BP             ;Saving BP\n\
        MOV BP, SP          ;BP points to the top of the stack\n\
        MOV BX, [BP + 4]    ;The number to be printed\n\
        ;if(BX < -1) then the number is positive\n\
        CMP BX, 0\n\
        JGE POSITIVE\n\
        ;else, the number is negative\n\
        MOV AH, 2           \n\
        MOV DL, '-'         ;Print a '-' sign\n\
        INT 21H\n\
        NEG BX              ;make BX positive\n\
        POSITIVE:\n\
        MOV AX, BX\n\
        MOV CX, 0        ;Initialize character count\n\
        PUSH_WHILE:\n\
            XOR DX, DX  ;clear DX\n\
            MOV BX, 10  ;BX has the divisor //// AX has the dividend\n\
            DIV BX\n\
            ;quotient is in AX and remainder is in DX\n\
            PUSH DX     ;Division by 10 will have a remainder less than 8 bits\n\
            INC CX       ;CX++\n\
            ;if(AX == 0) then break the loop\n\
            CMP AX, 0\n\
            JE END_PUSH_WHILE\n\
            ;else continue\n\
            JMP PUSH_WHILE\n\
        END_PUSH_WHILE:\n\
        MOV AH, 2\n\
        POP_WHILE:\n\
            POP DX      ;Division by 10 will have a remaainder less than 8 bits\n\
            ADD DL, '0'\n\
            INT 21H     ;So DL will have the desired character\n\
            DEC CX       ;CX--\n\
            ;if(CX <= 0) then end loop\n\
            CMP CX, 0\n\
            JLE END_POP_WHILE\n\
            ;else continue\n\
            JMP POP_WHILE\n\
        END_POP_WHILE:\n\
        ;Print newline\n\
        MOV DL, 0DH\n\
        INT 21H\n\
        MOV DL, 0AH\n\
        INT 21H\n\
        POP BP          ; Restore BP\n\
        RET 2\n\
    PRINT_NUM ENDP";
    
    addInCodeSegment(code);
}

void endAssemblyCode()
{
    string code = "";
    code += "END MAIN" + NEWLINE;
    addInCodeSegment(code);
}