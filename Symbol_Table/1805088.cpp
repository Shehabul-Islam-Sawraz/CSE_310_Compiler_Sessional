#include<iostream>
#include<bits/stdc++.h>

using namespace std;

class SymbolInfo
{
private:
    SymbolInfo* previous;
    SymbolInfo* next;
    string name, type;

public:
    SymbolInfo(string name,string type){
        this->name = name;
        this->type = type;
        this->next = nullptr;
        this->previous = nullptr;
    }

    void setName(string name){
        this->name = name;
    }

    string getName(){
        return this->name;
    }

    void setType(string type){
        this->type = type;
    }

    string getType(){
        return this->type;
    }

    void setPrevious(SymbolInfo* previous){
        this->previous = previous;
    }

    SymbolInfo* getPrevious(){
        return this->previous;
    }

    void setNext(SymbolInfo* next){
        this->next = next;
    }

    SymbolInfo* getNext(){
        return this->next;
    }
};

class ScopeTable
{
private:
    SymbolInfo** scopeTable;
    ScopeTable* parentScope;
    int noOfChild;
    int noOfBuckets;
    string scopeName;
public:
    ScopeTable(int noOfBuckets){
        this->scopeTable = new SymbolInfo*[noOfBuckets];
        for(int i=0;i<noOfBuckets;i++){
            scopeTable[i] = nullptr;
        }
        this->noOfChild = 0;
        this->parentScope = nullptr;
        this->noOfBuckets = noOfBuckets;
    }

    void setParentScope(ScopeTable* parentScope){
        this->parentScope = parentScope;
    }

    ScopeTable* getParentScope(){
        return this->parentScope;
    }

    int getChildNo(){
        return this->noOfChild;
    }

    void setChildNo(){
        this->noOfChild++;
    }

    string getScopeName(){
        return this->scopeName;
    }

    void setScopeName(){
        this->scopeName = scopeName;
    }
};

class SymbolTable
{
private:
    stack<ScopeTable> scopes;
    ScopeTable currentScope;
public:
    ScopeTable getCurrentScope(){
        return this->currentScope;
    }

    void setCurrentScope(){
        this->currentScope = currentScope;
    }

    stack<ScopeTable> getScopeTables(){
        return this->scopes;
    }

};

int main()
{
    return 0;
}
