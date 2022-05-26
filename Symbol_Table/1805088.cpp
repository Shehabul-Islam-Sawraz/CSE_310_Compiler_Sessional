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

    void setChildNo(int noOfChild){
        this->noOfChild = noOfChild;
    }

    string getScopeName(){
        return this->scopeName;
    }

    void setScopeName(string scopeName){
        this->scopeName = scopeName;
    }

    SymbolInfo* lookUpScope(string name){
        for(int i=0;i<noOfBuckets;i++){
            SymbolInfo* si = scopeTable[i];
            if(si!=nullptr){
                int x=0;
                while(si!=nullptr){
                    if(si->getName().compare(name)==0){
                        cout << "Found in ScopeTable #" << this->scopeName << " at position " << i << ", " << x << endl;
                        return si;
                    }
                    si = si->getNext();
                    x++;
                }
            }
        }
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
        cout << "Not found" << endl;
        return true;
    }

    bool deleteSymbol(string name){
        SymbolInfo* si = lookUpScope(name);
        if(si!=nullptr){
            SymbolInfo* prev = si->getPrevious();
            SymbolInfo* next = si->getNext();
            if(next==nullptr){
                prev->setNext(nullptr);
            }
            else{
                prev->setNext(next);
                next->setPrevious(prev);
            }
            cout << "Deleted Entry from current ScopeTable" << endl;
            return true;
        }
        return false;
    }

    void printScope(){
        cout << "ScopeTable #" << this->scopeName << endl;
        for(int i=0;i<noOfBuckets;i++){
            SymbolInfo* si = scopeTable[i];
            cout << i << " --> ";
            if(si==nullptr){
                cout << endl;
                continue;
            }
            while(si!=nullptr){
                cout << "< " << si->getName() << " : " << si->getType() << " >" << " ";
                si = si->getNext();
            }
            cout << endl;
        }
    }
};

class SymbolTable
{
private:
    stack<ScopeTable*> scopes;
    ScopeTable* currentScope;
    int noOfChild;
public:
    SymbolTable(){
        this->currentScope = nullptr;
        this->noOfChild = 0;
    }
    ScopeTable* getCurrentScope(){
        return this->currentScope;
    }

    stack<ScopeTable*> getScopeTables(){
        return this->scopes;
    }

    ScopeTable* createScopeTable(int noOfBuckets){
        ScopeTable* scope = new ScopeTable(noOfBuckets);
        if(currentScope==nullptr){
            this->currentScope = scope;
            scope->setParentScope(nullptr);
            scope->setScopeName(to_string(this->noOfChild + 1));
            this->noOfChild=this->noOfChild+1;
        }
        else{
            scope->setParentScope(this->currentScope);
            scope->setScopeName((this->currentScope->getScopeName()).append(to_string(this->currentScope->getChildNo() + 1)));
            this->currentScope->setChildNo(this->currentScope->getChildNo() + 1);
            this->currentScope = scope;
        }
        cout << "New ScopeTable with id " << this->currentScope->getScopeName() << " created" << endl;
        this->scopes.push(scope);
        return scope;
    }

    void exitScope(){
        if(this->currentScope==nullptr){
            cout << "No Current Scope" << endl;
            return;
        }
        cout << "ScopeTable with id " << this->currentScope->getScopeName() << " removed" << endl;
        if(this->currentScope->getParentScope()==nullptr){
            cout << "Destroying the First Scope" << endl;
        }
        this->currentScope = this->currentScope->getParentScope();
        this->scopes.pop();
    }
};

int main()
{
    SymbolTable* symbolTable = new SymbolTable();
    ScopeTable * scope =  symbolTable->createScopeTable(7);
    scope->insertSymbol("A","a",0);
    scope->insertSymbol("B","b",0);
    scope->insertSymbol("C","c",1);
    scope->insertSymbol("D","d",2);
    scope->insertSymbol("E","e",2);
    scope->insertSymbol("F","f",2);
    scope->insertSymbol("G","g",0);
    scope->insertSymbol("H","h",0);
    scope->insertSymbol("I","i",1);
    scope->insertSymbol("J","j",2);
    scope->insertSymbol("K","k",3);
    scope->printScope();
    scope->lookUpScope("B");
    scope->lookUpScope("O");
    scope->lookUpScope("J");
    scope->lookUpScope("k");
    scope->deleteSymbol("E");
    scope->deleteSymbol("G");
    scope->printScope();
    scope->insertSymbol("D","d",0);
    scope->insertSymbol("E","e",4);
    scope->printScope();
    symbolTable->exitScope();
    symbolTable->exitScope();
    return 0;
}
