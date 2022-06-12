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
    bool success = scope->insertSymbol(lexeme, token,(int)(hashValue(lexeme)%NoOfBuckets));
    if(success){
        symbolTable->printAllScope();
    }
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

void printTokenWithSymbol(string token, string lexeme){
    fprintf(tokenout, "<%s, %s>", token.data(), lexeme.data());
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

void addConstChar(string token){
    string ch = yytext;
    ch = ch.substr(1,ch.length()-2);
    //printTokenWithSymbol(token,ch);
    fprintf(tokenout, "<%s, %c>", token.data(), makeSpecialChar(yytext));
    fprintf(logout,"Line no %d: TOKEN <%s> Lexeme %s found --> < %s, %c >\n",line_count, token.data(), yytext, token.data(), makeSpecialChar(yytext));
    insertIntoHashTable(token,yytext);
}

void addIdentifier(string token){
    printTokenWithSymbol(token);
    insertIntoHashTable(token,yytext);
}

void addOperators(string token){
    string ch = yytext;
    if(ch.compare("{")==0){
        scope = symbolTable->createScopeTable(NoOfBuckets);
    }
    printTokenWithSymbol(token);
}