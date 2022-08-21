#include "SymbolTable.h"
#include <bits/stdc++.h>

#define DEFINE_WORD " DW "
#define ARRAY_DUP " DUP(0)"
#define NEWLINE string("\r\n")

string codesegment, datasegment;
ofstream asmFile;

int tempCount = 0, maxTempCount = -1;
int labelCount = 0;
string forLoopIncDecCode = "";
string forLoopExpressionCode = "";
bool isForLoop = false;
string forLoopStartLabel = "", forLoopEndLabel = "", elseBlockEndLabel = "";
string whileLoopStartLabel = "", whileLoopEndLabel = "";

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
    asmFile << popValue(init_asm_model);
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
            forLoopIncDecCode = code;
        }
    }
}

string getLabelForFunction(string func_name)
{
    return func_name + "_EXIT";
}

void init_model()
{
    string code = ".MODEL SMALL\r\n";
    code += "\r\n.STACk 800H\r\n";
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

void addIncDecAsmCode(SymbolInfo *sym, string op, string type)
{
    string code = "";
    if(type == "post"){
        code += "\t\t; At line no " + to_string(line_count) + ": Evaluating postfix " + op + " of " + sym->getName() + NEWLINE;
        if(sym->getDecType() == ARRAY){
            code += "\t\tPOP BX" + "\t; Array index popped from stack" + NEWLINE;
            code += "\t\tPUSH BP" + "\t; Saving value of BP in stack" + NEWLINE;
            code += "\t\tMOV BP, BX" + "\t; Saving value of array index in BP" + NEWLINE;
            code += "\t\tMOV AX, [BP]" + "\t; Saving value of " + sym->getName() + " in AX" + NEWLINE;
            code += "\t\tPOP BP" + "\t; Resetting value of BP" + NEWLINE;
        }
        else{
            code += "\t\tPOP AX" + "\t; Saving value of " + sym->getName() + " in AX" + NEWLINE;
            code += "\t\tPUSH AX" + "\t; Pushing the value of " + sym->getName() + " back to stack" + NEWLINE;
        }

        if(op == "++"){
            code += "\t\tINC AX" + "\t; Incrementing " + sym->getName() + NEWLINE;
        }
        else{
            code += "\t\tDEC AX" + "\t; Decrementing " + sym->getName() + NEWLINE;
        }
        
        if(sym->isGlobal) {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + "\t; Saving value of BP in stack" + NEWLINE;
                code += "\t\tMOV BP, BX" + "\t; Saving value of array index in BP" + NEWLINE;
                code += "\t\tMOV " + sym->getName() + "[BP], AX" + "\t; Saving the result in stack" + NEWLINE;
                code += "\t\tPOP BP" + "\t; Resetting value of BP" + NEWLINE;
            }
            else{
                code += "\t\tMOV " + sym->getName() + ", AX" + "\t; Saving the result in stack" + NEWLINE;
            }
        } 
        else {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + "\t; Saving value of BP in stack" + NEWLINE;
                code += "\t\tMOV BP, BX" + "\t; Saving value of array index in BP" + NEWLINE;
                code += "\t\tMOV [BP], AX" + "\t; Saving the result in stack" + NEWLINE;
                code += "\t\tPOP BP" + "\t; Resetting value of BP" + NEWLINE;
            }
            else{
                code += "\t\tMOV [BP + " + to_string(-1 * sym->getOffset()) + "], AX" + "\t; Saving result in stack" + NEWLINE;
            }
        }
    }
    else{
        code += "\t\t; At line no " + to_string(line_count) + ": Evaluating prefix " + op + " of " + sym->getName() + NEWLINE;
        if(sym->getDecType() == ARRAY){
            code += "\t\tPOP BX" + "\t; Array index popped from stack" + NEWLINE;
            // code += "\t\tPUSH BP" + "\t; Saving value of BP in stack" + NEWLINE;
            // code += "\t\tMOV BP, BX" + "\t; Saving value of array index in BP" + NEWLINE;
            // code += "\t\tMOV AX, [BP]" + "\t; Saving value of " + sym->getName() + " in AX" + NEWLINE;
            // code += "\t\tPOP BP" + "\t; Resetting value of BP" + NEWLINE;
        }
        // else{
        //     code += "\t\tPOP AX" + "\t; Saving value of " + sym->getName() + " in AX" + NEWLINE;
        //     code += "\t\tPUSH AX" + "\t; Pushing the value of " + sym->getName() + " back to stack" + NEWLINE;
        // }
        code += "\t\tPOP AX" + "\t; Saving value of " + sym->getName() + " in AX" + NEWLINE;

        if(op == "++"){
            code += "\t\tINC AX" + "\t; Incrementing " + sym->getName() + NEWLINE;
        }
        else{
            code += "\t\tDEC AX" + "\t; Decrementing " + sym->getName() + NEWLINE;
        }
        
        code += "\t\tPUSH AX" + "\t; Pushing the value of " + sym->getName() + " back to stack" + NEWLINE;

        if(sym->isGlobal) {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + "\t; Saving value of BP in stack" + NEWLINE;
                code += "\t\tMOV BP, BX" + "\t; Saving value of array index in BP" + NEWLINE;
                code += "\t\tMOV " + sym->getName() + "[BP], AX" + "\t; Saving the result in stack" + NEWLINE;
                code += "\t\tPOP BP" + "\t; Resetting value of BP" + NEWLINE;
            }
            else{
                code += "\t\tMOV " + sym->getName() + ", AX" + "\t; Saving the result in stack" + NEWLINE;
            }
        } 
        else {
            if(sym->getDecType() == ARRAY){
                code += "\t\tPUSH BP" + "\t; Saving value of BP in stack" + NEWLINE;
                code += "\t\tMOV BP, BX" + "\t; Saving value of array index in BP" + NEWLINE;
                code += "\t\tMOV [BP], AX" + "\t; Saving the result in stack" + NEWLINE;
                code += "\t\tPOP BP" + "\t; Resetting value of BP" + NEWLINE;
            }
            else{
                code += "\t\tMOV [BP + " + to_string(-1 * sym->getOffset()) + "], AX" + "\t; Saving result in stack" + NEWLINE;
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

void handleExtraExpressionPush(string name)
{
    // There is always an extra push after every expression
    // So, we need to pop it out after we get a semicolon after an expression
    string code = "";
    code += "\t\tPOP AX" + "\t; Popped out " + name + NEWLINE;
    addInCodeSegment(code);
}

void forLoopStart()
{
    string startLabel = newLabel();
    forLoopStartLabel = startLabel;
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": Starting for loop" + NEWLINE;
    code += "\t\t" + startLabel + ":\t; For loop start label" + NEWLINE;
    addInCodeSegment(code);
}

void forLoopConditionCheck(){
    string labelIfTrue = newLabel();
    string endLabel = newLabel();
    forLoopEndLabel = endLabel;
    string code = "";
    // We have already popped AX from stack after getting expression_statement. This AX contains
    // the result of the condition inside for loop
    code += "\t\tCMP AX, 0" + "\t; Checking if the condition is true or false" + NEWLINE;
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
    code += "\t\tPOP AX" + "\t; Popped " + var + " from stack" + NEWLINE;
    code += "\t\tJMP " + forLoopStartLabel + "\t; Jump back to for loop" + NEWLINE;
    code += "\t\t; End label of for loop" + NEWLINE;
    code += "\t\t" + forLoopEndLabel + ":" + NEWLINE;
    forLoopExpressionCode = code;
}

void endForLoop()
{
    addInCodeSegment(forLoopIncDecCode);
    addInCodeSegment(forLoopExpressionCode);
    isForLoop = false;
}

SymbolInfo* createIfBlock()
{
    string labelIfTrue = newLabel();
    string labelIfFalse = newLabel();
    SymbolInfo *sym = new SymbolInfo(labelIfFalse, TEMPORARY_TYPE);
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": Evaluationg if statement" + NEWLINE;
    code += "\t\tPOP AX" + "\t; Popped expression value from stack" + NEWLINE;
    code += "\t\tCMP AX, 0" + "\t; Checking whether expression is true or false" + NEWLINE;
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
    elseBlockEndLabel = endLabel;
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
    code += "\t\t; End label for if else statement" + NEWLINE;
    code += "\t\t" + elseBlockEndLabel + ":" + NEWLINE;
    addInCodeSegment(code);
}

void whileLoopStart()
{
    string startLabel = newLabel();
    whileLoopStartLabel = startLabel;
    string code = "";
    code += "\t\t; At line no " + to_string(line_count) + ": Starting while loop" + NEWLINE;
    code += "\t\t" + startLabel + ":\t; While loop start label" + NEWLINE;
    addInCodeSegment(code);
}

void whileLoopConditionCheck(string var){
    string labelIfTrue = newLabel();
    string endLabel = newLabel();
    whileLoopEndLabel = endLabel;
    string code = "";
    code += "\t\tPOP AX" + "\t; Popped " + var + " from stack" + NEWLINE;
    code += "\t\tCMP AX, 0" + "\t; Checking if the condition is true or false" + NEWLINE;
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
    code += "\t\tJMP " + whileLoopStartLabel + "\t; Jump back to while loop" + NEWLINE;
    code += "\t\t; End label of while loop" + NEWLINE;
    code += "\t\t" + whileLoopEndLabel + ":" + NEWLINE;
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
        code += "\t\tPUSH [BP+" + to_string(-1 * sym->getOffset()) + "]" + "\t; Passing " + sym->getName() + " to PRINT_NUM for printing it" + NEWLINE;
    }
    code += "\t\tCALL PRINT_NUM" + NEWLINE;
    addInCodeSegment(code);
}

void returnFunction()
{
    string code = "";
    code += "\t\tPOP AX" + "\t; Popped return value and saved it in AX" + NEWLINE;
    addInCodeSegment(code);
}

void writeProcPrintln() {
    string code = "";
    code = "\t;println(n)\n";
    code += 
    "\tPRINT_INTEGER PROC NEAR\n\
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
    PRINT_INTEGER ENDP";
    
    addInCodeSegment(code);
}

void endAssemblyCode()
{
    string code = "";
    code += "END MAIN" + NEWLINE;
    addInCodeSegment(code);
}