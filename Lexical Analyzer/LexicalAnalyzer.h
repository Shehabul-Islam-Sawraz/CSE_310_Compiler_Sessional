#include "SymbolTable.h"
#include<cstring>

#define NoOfBuckets 7

int line_count = 1;
int word_count = 0;

ScopeTable* scope = nullptr;
SymbolTable* symbolTable = new SymbolTable();

string toUpper(string token){
    string temp = "";
    int len  = token.length();
    for(int i=0;i<len;i++){
        temp+=toupper(token[i]);
    }
    return temp;
}

void insertIntoHashTable(string token, string lexeme){
    if(scope==nullptr){
        scope = symbolTable->createScopeTable(NoOfBuckets);
    }
    scope->insertSymbol(lexeme, token,(int)(hashValue(lexeme)%NoOfBuckets));
    symbolTable->printAllScope();
}

void printLogData(int noOfLine, string token){
    fprintf(logout,"Line no %d: TOKEN <%s> Lexeme %s found\n",noOfLine, token.data(), yytext);
}

void printToken(string token){
    fprintf(tokenout, "<%s>", token.data());
    printLogData(line_count, token);
}

void printTokenWithSymbol(string token){
    fprintf(tokenout, "<%s, %s>", token.data(), yytext);
    printLogData(line_count, token);
}

void addKeywords(){
    printToken(toUpper(yytext));
}

void addConstInt(string token){
    printTokenWithSymbol(token);
    insertIntoHashTable(token,yytext);
}

void addConstFloat(string token){
    printTokenWithSymbol(token);
    insertIntoHashTable(token,yytext);
}

void addIdentifier(string token){
    printTokenWithSymbol(token);
    insertIntoHashTable(token,yytext);
}