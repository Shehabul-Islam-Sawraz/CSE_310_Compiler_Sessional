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

    SymbolInfo* lookUpScope(string name){
        for(int i=0;i<noOfBuckets;i++){
            SymbolInfo* si = scopeTable[i];
            if(si!=nullptr){
                int x=0;
                while(si!=nullptr){
                    if(si->getName().compare(name)==0){
                        cout << "Found in ScopeTable# 1.1.1 at position " << i << ", " << x << endl;
                        return si;
                    }
                    si = si->getNext();
                    x++;
                }
            }
        }
        cout << "Not found" << endl;
        return nullptr;
    }

    bool insertSymbol(string name,string type,int index){
        if(lookUpScope(name)!=nullptr){
            cout << "<" << name << ", " << type << "> already exists in current ScopeTable" << endl;
            return false;
        }
        SymbolInfo* symbol = new SymbolInfo(name,type);
        if(scopeTable[index]==nullptr){
            symbol->setNext(nullptr);
            symbol->setPrevious(nullptr);
            scopeTable[index] = symbol;
        }
        else{
            SymbolInfo* si = scopeTable[index];
            while(si->getNext()!=nullptr){
                si = si->getNext();
            }
            symbol->setNext(nullptr);
            symbol->setPrevious(si);
            si->setNext(symbol);
        }
        return true;
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
