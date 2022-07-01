#include "SymbolTable.h"
#include<cstring>

#define NoOfBuckets 7

int line_count = 1;
int error_count = 0;
string str="",original_str="";
int string_line_no;

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

void printErrorLog(string error){
    fprintf(logout,"Error at line no %d: %s %s found\n",line_count, error.data(), yytext);
    error_count++;
}

void printToken(string token){
    fprintf(tokenout, " <%s> ", token.data());
    printLogData(line_count, token);
}

void printTokenWithSymbol(string token){
    fprintf(tokenout, " <%s, %s> ", token.data(), yytext);
    printLogData(line_count, token);
}

// void printTokenWithSymbol(string token, string lexeme){
//     fprintf(tokenout, "<%s, %s>", token.data(), lexeme.data());
//     printLogData(line_count, token);
// }

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
    else if(ch.compare("}")==0){
        scope = symbolTable->exitScope();
    }
    printTokenWithSymbol(token);
}

void addString(string token, string str){
    fprintf(tokenout, "<%s, %s>", token.data(), yytext);
    fprintf(logout,"Line no %d: TOKEN <%s> Lexeme %s found --> < %s, %s >\n",string_line_no, token.data(), str.data(), token.data(), yytext);
}

void handleSpecialStringCharacters(){
    original_str.append(yytext);
    if(yytext[1] == '\\'){
        str.append(1u, '\\');
    }
    if(yytext[1] == 'n'){
		str.append(1u, '\n');
    }
    if(yytext[1] == 't'){
        str.append(1u, '\t');
    }
    if(yytext[1] == '\''){
        str.append(1u, '\'');
    }
    if(yytext[1] == 'a'){
        str.append(1u, '\a');
    }
    if(yytext[1] == 'f'){
        str.append(1u, '\f');
    }
    if(yytext[1] == 'r'){
        str.append(1u, '\r');
    }
    if(yytext[1] == 'b'){
        str.append(1u, '\b');
    }
    if(yytext[1] == 'v'){
        str.append(1u, '\v');
    }
    if(yytext[1] == '0'){
        str.append(1u, '\0');
    }	
    if(yytext[1] == '\"'){
        str.append(1u, '\"');
    }
}

void clearNewline(){
    int len = strlen(yytext);
    for(int i=0;i<len;i++){
        if(yytext[i]=='\n'){
            line_count++;
        }
    }
}

void addComment(string token){
    printLogData(line_count, token);
    clearNewline();
}