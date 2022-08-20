#include "SymbolTable.h"
#include <bits/stdc++.h>

#define DEFINE_WORD " DW "
#define ARRAY_DUP " DUP(0)"
#define NEWLINE string("\r\n")

string codesegment, datasegment;
ofstream asmFile;

int tempCount = 0, maxTempCount = -1;
int labelCount = 0;

ScopeTable *scope = nullptr;                  // Have to declare it in ICG header file
SymbolTable *symbolTable = new SymbolTable(); // Have to declare it in ICG header file

SymbolInfo *currentFunc = nullptr;

string ruleName[] = {"init_asm_model", "data_segment", "code_segment",
                     "init_data", "error"};

enum NONTERMINAL_TYPE
{
    init_asm_model = 0,
    data_segment,
    code_segment,
    init_data,
    error
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

void writeInDataSegment()
{
    asmFile.close();
    asmFile.open("code.asm");
    // asmFile << popValue(init_asm_model);
    asmFile << popValue(data_segment);
    asmFile << popValue(code_segment);
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
    return var + to_string(temp->getScopeID());
}

void addGlobalVarInDataSegment(string var, int arrsize = 0, bool isArray = false)
{
    // string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + "?";
    if (!isArray)
    {
        string code = "\t" + var + DEFINE_WORD + "?";
    }
    else
    {
        string code = "\t" + var + DEFINE_WORD + to_string(arrsize) + ARRAY_DUP;
    }
    replaceByNewLine(code);
    // datasegment+=code;
    setValue(data_segment, popValue(data_segment) + code);
    writeInDataSegment();
}

// void addVarInDataSegment(string var){
//     //string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + "?";
//     string code = "\t" + var + DEFINE_WORD + "?";
//     replaceByNewLine(code);
//     //datasegment+=code;
//     setValue(data_segment, popValue(data_segment)+code);
//     writeInDataSegment();
// }

// void addArrInDataSegment(string var, int arrsize){
//     //SymbolInfo* temp = symbolTable->lookUp(var,(int)(hashValue(var)%NoOfBuckets));
//     string code = "\t" + var + DEFINE_WORD + to_string(arrsize) + ARRAY_DUP;
//     replaceByNewLine(code);
//     //datasegment+=code;
//     setValue(data_segment, popValue(data_segment)+code);
//     writeInDataSegment();
// }

void addInCodeSegment(string code)
{
    if ((error_count + syntax_error_count) == 0)
    {
        string str = code;
        replaceByNewLine(str);
        // codesegment+=str;
        setValue(code_segment, popValue(code_segment) + str);
        writeInCodeSegment(str);
    }
}

string getLabelForFunction(string func_name)
{
    return func_name + "_EXIT";
}

void init_model()
{
    string code = ".MODEL SMALL\r\n";
    code += "\r\n.STACk 100H\r\n";
    setValue(init_asm_model, code);
    setValue(data_segment, "\r\n.DATA \r\n");
    setValue(code_segment, "\r\n.CODE\r\n");
    setValue(init_data, "\r\nMOV AX,@DATA\r\n
                            MOV DS,AX\r\n");
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
    code += "\t\tPOP BX\t; " + right->getName() + " popped from stack\n";
    code += "\t\tPOP AX\t; " + left->getName() + " popped from stack\n";
    code += "\t\t" + op + " AX, BX\n";
    code += "\t\tPUSH AX\t; Pushed evaluated value of " + left->getName() + op + right->getName() + " in the stack\n";
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
    code += "\t\tPOP BX" + "\t; Popped out " + right->getName() + " from stack\n";
    code += "\t\tPOP AX" + "\t; Popped out " + left->getName() + " from stack\n";
    code += "\t\tCMP AX, BX" + "\t; Comparing " + left->getName() + " with " + right->getName() + "\n";
    code += conditionalJump(op, labelIfTrue); // If true then do conditional jump
    code += jumpInstant(labelIfFalse);        // If false then do long jump
    code += declareLabel(labelIfTrue, true);
    code += declareLabel(labelIfFalse, false);
    addInCodeSegment(code);
}

void addLogicOpAsmCode(string op, SymbolInfo *left, SymbolInfo *right)
{
    string labelForRightCheck = newLabel();
    string endLabel = newLabel();
    string resultLabel = newLabel();

    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": " + left->getName() + op + right->getName() + NEWLINE;
    code += "\t\tPOP BX" + "\t; " + right->getName() + " popped from stack\n";
    code += "\t\tPOP AX" + "\t; " + left->getName() + " popped from stack\n";
    code += "\t\tCMP AX, 0" + "\t; Comparing " + left->getName() + " and 0\n";
    // If operator is &&, then getting a 0 will cause result to be 0
    // and if the operator is ||, then ggetting a 1 will cause result
    // to be 1
    if (op == "&&")
    {
        code += "\t\t; If " + left->getName() + " is not 0, then check " + right->getName() + ". So, jump to " + labelForRightCheck + "\n";
        code += conditionalJump("JNE", labelForRightCheck);
    }
    else
    {
        code += "\t\t; if " + left->getName() + " is 0, check " + right->getName() + ". So, jump to " + labelForRightCheck + "\n";
        code += conditionalJump("JE", labelForRightCheck);
    }
    code += jumpInstant(endLabel);
    code += "\t\t" + labelForRightCheck + ":\n";
    code += "\t\tCMP BX, 0" + "\t; Comparing " + right->getName() + " and 0\n";
    // If operator is &&, then getting 0 in right will cause result to be 0
    // and if the operator is ||, then ggetting a 1 in right will cause result
    // to be 1
    if (op == "&&")
    {
        code += "\t\t; If " + right->getName() + " is not 0, the whole expression is true. So, jump to " + resultLabel + NEWLINE;
        code += conditionalJump("JNE", resultLabel);
    }
    else
    {
        code += "\t\t; If " + right->getName() + " is 0, the whole expression is false. So, jump to " + resultLabel + NEWLINE;
        code += conditionalJump("JE", resultLabel);
    }
    code += jumpInstant(endLabel);
    if (op == "&&")
    {
        code += declareLabel(resultLabel, true);
        code += declareLabel(endLabel, false);
    }
    else
    {
        code += declareLabel(resultLabel, false);
        code += declareLabel(endLabel, true);
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
    code += "\t\t; DATA SEGMENT INITIALIZATION\n";
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
    code += "\t" + name + " PROC\n";
    code += "\t\t; Function with name " + name + " started";
    code += "\t\tPUSH BP\n";
    code += "\t\tMOV BP, SP\t; All the offsets of a function depends on the value of BP\n";
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
    code += "\t\t" + getLabelForFunction(name) + ":\n";
    code += "\t\tMOV SP, BP\t; Restoring SP at the end of function\n";
    code += "\t\tPOP BP\t; Restoring BP at the end of function\n";

    if (name == "main")
    {
        code += "\t\t; Setting interrupt for function end\n";
        code += "\t\tMOV AH, 4CH\n\t\tINT 21H\n";
    }
    if (currentFunc->getFuncRetType() == VOID_TYPE)
    {
        code += "\t\tRET 0\n"
    }
    else
    {
        vector<string> v = currentFunc->getparamType();
        code += "\t\tRET " + to_string(2 * v.size()) + "\n";
    }

    code += "\t" + name + " ENDP\n";
    addInCodeSegment(code);
}

void popArrayFromStack(string reg, SymbolInfo *sym)
{
    // If after "variable: Id" or "variable: Array" rule matches and next rule doesn't match with INCOP or DECOP,
    // and goes to "factor : variable" rule then we no longer need the index of the previously accessed array
    if (sym->getDecType() == ARRAY)
    {
        addInCodeSegment("\t\tPOP BX" + "\t; Array index popped because it is no longer required");
    }
}

void evaluateArrayVariable(SymbolInfo *var, string index)
{
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": getting value of " + var->getName() + "[" + index + "]\n";
    code += "\t\tPOP BX" + "\t; Getting index of the array\n";
    code += "\t\tSHL BX, 1" + "\t; Multiplying index by 2 to match size of a word\n";
    if (var->isGlobal)
    {
        code += "\t\tPUSH BP" + "\t; Saving value of BP in stack, so that we can restore it's value later" + NEWLINE;
        code += "\t\tMOV BP, BX" + "\t; Saving address of the array index in BP to access array from stack" + NEWLINE;
        code += "\t\tMOV AX, " + var->getName() + "[BP]" + "\t; Getting the value of the array from index BP\n";
    }
    else
    {
        code += "\t\tSUB BX, " + to_string(var->getOffset()) + "\t; Adding the offset of the array to get the offset of array element\n";
        code += "\t\tADD BX, BP" + "\t; Adding BP to BX to get the address of the array\n";
        code += "\t\tPUSH BP" + "\t; Saving value of BP in stack, so that we can restore it's value later" + NEWLINE;
        code += "\t\tMOV BP, BX" + "\t; Saving address of the array index in BP to access array from stack" + NEWLINE;
        code += "\t\tMOV AX, [BP]" + "\t; Getting the value of the array at address BP\n";
    }
    code += "\t\tPOP BP" + "\t; Restoring value of BP" + NEWLINE;
    // Pushing the index and value of the array element on the stack
    // This will allow the ASSIGNOP and INCOP to use it later
    code += "\t\tPUSH AX" + "\t; Pushing the value of the array element at index " + index->getName() + NEWLINE;
    code += "\t\tPUSH BX" + "\t; Pushing the index of the array\n";
    addInCodeSegment(code);
}