#include "SymbolTable.h"
#include<bits/stdc++.h>

#define DEFINE_WORD " DW "
#define ARRAY_DUP " DUP(0)"
#define NEWLINE string("\r\n")

string codesegment,datasegment;
ofstream asmFile;

int tempCount=0,maxTempCount=-1;

ScopeTable* scope = nullptr; // Have to declare it in ICG header file
SymbolTable* symbolTable = new SymbolTable(); // Have to declare it in ICG header file

string ruleName[] = {"init_asm_model", "data_segment", "code_segment",
                         "init_data","error"};

enum NONTERMINAL_TYPE {
	init_asm_model = 0, data_segment, code_segment, init_data,error
};

class NonTerminalHandler
{
private:
	stack<string> nonterminals[NONTERMINAL_TYPE::error +1];
public:
	string getValue(NONTERMINAL_TYPE nonterminal){
		if(nonterminals[nonterminal].empty()){
			return "";
		}
		return nonterminals[nonterminal].top();
	}
	string popValue(NONTERMINAL_TYPE nonterminal){
		if(nonterminals[nonterminal].empty()){
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

void setValue(NONTERMINAL_TYPE nonterminal, string value){
	nonTerminalHandler.setValue(nonterminal,value);
}

string popValue(NONTERMINAL_TYPE nonterminal) {
	string val = nonTerminalHandler.popValue(nonterminal);
	return val;
}


bool replaceAll(string &source, string toReplace, string replaceBy) {
	if (source.find(toReplace, 0) == string::npos){
		return false;
	}
	for (string::size_type i = 0; (i = source.find(toReplace, i)) != string::npos;) {
		source.replace(i, toReplace.length(), replaceBy);
		i += replaceBy.length();
	}
	return true;
}

void replaceByNewLine(string &str){
    replaceAll(str,"\r","");
    replaceAll(str,"\n","\r\n");
    if(str[str.size()-1]!='\n'){
        str+="\r\n";
    }
}

uint32_t hashValue(string str){
    uint32_t hash = 0;
    for(int c:str)
        hash = c + (hash * 64) + (hash *65536) - hash;
    return hash;
}

void writeInDataSegment(){
    asmFile.close();
    asmFile.open("code.asm");
    asmFile << popValue(init_asm_model);
    asmFile << popValue(data_segment);
    asmFile << popValue(code_segment);
}

void writeInCodeSegment(string str){
    asmFile << str;
}

string GET_ASM_VAR_NAME(string var){
    SymbolInfo* temp = symbolTable->lookUp(var,(int)(hashValue(var)%NoOfBuckets));
    if(temp==nullptr || temp->getType()!="ID"){
        return var;
    }
    return var + to_string(temp->getScopeID());
}

void addVarInDataSegment(string var){
    string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + "?";
    replaceByNewLine(code);
    //datasegment+=code;
    setValue(popValue(data_segment)+code);
    writeInDataSegment();
}

void addArrInDataSegment(string var){
    SymbolInfo* temp = symbolTable->lookUp(var,(int)(hashValue(var)%NoOfBuckets));
    string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + to_string(temp->getArrSize()) + ARRAY_DUP;
    replaceByNewLine(code);
    //datasegment+=code;
    setValue(popValue(data_segment)+code);
    writeInDataSegment();
}

void addInCodeSegment(string code){
    string str = code;
    replaceByNewLine(str);
    //codesegment+=str;
    setValue(popValue(code_segment)+str);
    writeInCodeSegment(str);
}

string getLabelForFunction(string func_name){
    return "_" + func_name;
}

void init_model(){
    string code = ".MODEL SMALL\r\n";
    code += "\r\n.STACk 100H\r\n";
    setValue(init_asm_model,code);
    setValue(data_segment, "\r\n.DATA \r\n");
    setValue(code_segment, "\r\n.CODE\r\n");
    setValue(init_data, "\r\nMOV AX,@DATA\r\n
                            MOV DS,AX\r\n");
}

string newTemp(){
    string temp = "t";
    char c[3];
    sprintf(c,"%d", tempCount);
    temp += c;
    if(tempCount>maxTempCount){
        maxTempCount++;
        addVarInDataSegment(temp);
    }
    tempCount++;
    return temp;
}

string memoryToReg(string command, string reg, string memory){
    return command + " " + reg + ", " + GET_ASM_VAR_NAME(memory) + NEWLINE;
}

string operatorToReg(string command, string reg, SymbolInfo* sym){
    if(sym->isArray()){

    }
    else{
        return memoryToReg(command, reg, sym->getName());
    }
}

string addAddOpAsmCode(string op, string tempVar, SymbolInfo* left, SymbolInfo* right){
    if(op=="+"){
        op = "ADD";
    }
    else if(op=="-"){
        op = "SUB";
    }

    string code = "";
    string temp = GET_ASM_VAR_NAME(tempVar);
    code += operatorToReg("MOV", "AX", left);
    code += operatorToReg(op, "AX", right);
    code += "MOV" + temp + ", AX" + NEWLINE;
    return code;
}