#include "SymbolTable.h"
#include<bits/stdc++.h>

#define DEFINE_WORD " DW "
#define ARRAY_DUP " DUP(0)"

string codesegment,datasegment;

ScopeTable* scope = nullptr; // Have to declare it in ICG header file
SymbolTable* symbolTable = new SymbolTable(); // Have to declare it in ICG header file

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

string GET_ASM_VAR_NAME(string var){
    SymbolInfo* temp = symbolTable->lookUp(var,(int)(hashValue(var)%NoOfBuckets));
    if(temp==nullptr || temp->getType()!="ID"){
        return var;
    }
    return var + to_string(temp->getScopeID());
}

void addVarInDataSegment(string var){
    string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + "0";
    replaceByNewLine(code);
    datasegment+=code;
}

void addArrInDataSegment(string var){
    SymbolInfo* temp = symbolTable->lookUp(var,(int)(hashValue(var)%NoOfBuckets));
    string code = GET_ASM_VAR_NAME(var) + DEFINE_WORD + to_string(temp->getArrSize()) + ARRAY_DUP;
    replaceByNewLine(code);
    datasegment+=code;
}

void addInCodeSegment(string code){
    string str = code;
    replaceByNewLine(str);
    codesegment+=str;
}

string getLabelForFunction(string func_name){
    return "_" + func_name;
}