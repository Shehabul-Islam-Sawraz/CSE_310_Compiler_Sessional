#include<iostream>
#include<bits/stdc++.h>

using namespace std;

//FILE *logout, *tokenout;
extern FILE *logout, *errorout, *parserout;
extern char *yytext;

char makeSpecialChar(char *ch) {
    if(ch[1] != '\\'){
        return ch[1];
    }
    else if(ch[1] == '\\'){
        if(ch[2] == '0'){
            return (char)0;
        }
        if(ch[2] == 'a'){
            return (char)7;
        }
        if(ch[2] == 'b'){
            return (char)8;
        }
        if(ch[2] == 't'){
            return (char)9;
        }
        if(ch[2] == 'n'){
            return (char)10;
        }
        if(ch[2] == 'v'){
            return (char)11;
        }
        if(ch[2] == 'f'){
            return (char)12;
        }
        if(ch[2] == 'r'){
            return (char)13;
        }
        if(ch[2] == '\"'){
            return (char)34;
        }
        if(ch[2] == '\''){
            return (char)39;
        }
        if(ch[2] == '\\'){
            return (char)92;
        }		
    }
}

vector<string> split(string str, char del){
    string temp = "";
    vector<string> words;
    for(int i=0; i<str.size(); i++){
        if(str[i] != del){
            temp += str[i];
        }
        else{
            if(temp.compare("")==0){
                continue;
            }
            words.push_back(temp);
            temp = "";
        }
    }
    return words;
}

int stringToInt(string s){
    stringstream ss(s);
    int x = 0;
    ss >> x;
    return x;
}

uint32_t hashValue(string &str){
    uint32_t hash = 0;
    for(int c:str)
        hash = c + (hash * 64) + (hash *65536) - hash;
    return hash;
}

class SymbolInfo
{
private:
    SymbolInfo* previous;
    SymbolInfo* next;
    string name, type;
    string decType; // Stores which one is declared!! FUNCTION, VARIABLE, ARRAY
    string varType; // Stores variable type!! INTEGER, FLOAT, DOUBLE, VOID
    bool isFuncDeclared = false;
    string funcRetType; // Stores return type of function
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

    void setDecType(string type){
        this->decType = type;
    }

    string getDecType(){
        return this->decType;
    }

    void setVarType(string type){
        this->varType = type;
    }

    string getVarType(){
        return this->varType;
    }

    void setFuncRetType(string type){
        this->funcRetType = type;
    }

    string getFuncRetType(){
        return this->funcRetType;
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

    ~SymbolInfo(){

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

    SymbolInfo* lookUpScope(string name, int index){
        SymbolInfo* si = scopeTable[index];
        if(si!=nullptr){
            int x=0;
            while(si!=nullptr){
                if(si->getName().compare(name)==0){
                    //cout << "Found in ScopeTable #" << this->scopeName << " at position " << index << ", " << x << endl;
                    return si;
                }
                si = si->getNext();
                x++;
            }
        }
        return nullptr;
    }

    void deleteFirst(int index){
        scopeTable[index] = nullptr;
    }

    bool insertSymbol(string name,string type,int index){
        if(lookUpScope(name,index)!=nullptr){
            //cout << "<" << name << ", " << type << "> already exists in current ScopeTable" << endl;
            fprintf(logout,"< %s : %s > already exists in current ScopeTable\n",name.data(),type.data());
            return false;
        }
        SymbolInfo* symbol = new SymbolInfo(name,type);
        int x=0;
        if(scopeTable[index]==nullptr){
            symbol->setNext(nullptr);
            symbol->setPrevious(nullptr);
            scopeTable[index] = symbol;
        }
        else{
            SymbolInfo* si = scopeTable[index];
            while(si->getNext()!=nullptr){
                x++;
                si = si->getNext();
            }
            symbol->setNext(nullptr);
            symbol->setPrevious(si);
            x++;
            si->setNext(symbol);
        }
        //cout << "Inserted in ScopeTable #" << this->scopeName << " at position " << index << ", " << x << endl;
        return true;
    }

    bool deleteSymbol(string name, int index){
        SymbolInfo* si = lookUpScope(name,index);
        if(si!=nullptr){
            SymbolInfo* prev = si->getPrevious();
            SymbolInfo* next = si->getNext();
            if(next==nullptr && prev==nullptr){
                deleteFirst(index);
            }
            else if(next==nullptr){
                prev->setNext(nullptr);
            }
            else{
                prev->setNext(next);
                next->setPrevious(prev);
            }
            //cout << "Deleted Entry from current ScopeTable" << endl;
            delete si;
            return true;
        }
        else{
            cout << "Not found" << endl;
        }
        return false;
    }

    void printScope(){
    	fprintf(logout,"ScopeTable #%s\n",this->scopeName.data());
        //cout << "ScopeTable #" << this->scopeName << endl;
        cout << endl;
        SymbolInfo* si = nullptr;
        for(int i=0;i<noOfBuckets;i++){
            si = scopeTable[i];
            if(si==nullptr){
                continue;
            }
            fprintf(logout,"%d --> ", i);
            //cout << i << " --> ";
            while(si!=nullptr){
                //cout << "< " << si->getName() << " : " << si->getType() << " >" << " ";
                fprintf(logout,"< %s : %s > ",si->getName().data(), si->getType().data());
                si = si->getNext();
            }
            //cout << endl;
            fprintf(logout,"\n");
        }
    }

    ~ScopeTable()
    {
        SymbolInfo* temp = nullptr;
        for(int i=0;i<noOfBuckets;i++){
            SymbolInfo* si = scopeTable[i];
            while(si!=nullptr){
                temp = si->getNext();
                delete si;
                si = temp;
            }
        }
        delete[] scopeTable;
        //cout << "Destroying the scope table" << endl;
    }
};

class SymbolTable
{
private:
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

    ScopeTable* createScopeTable(int noOfBuckets){
        ScopeTable* scope = new ScopeTable(noOfBuckets);
        if(currentScope==nullptr){
            this->currentScope = scope;
            scope->setParentScope(nullptr);
            scope->setScopeName(to_string(this->noOfChild + 1));
            this->noOfChild=this->noOfChild;
        }
        else{
            scope->setParentScope(this->currentScope);
            string sc = (this->currentScope->getScopeName()).append(".");
            scope->setScopeName(sc.append(to_string(this->currentScope->getChildNo() + 1)));
            this->currentScope->setChildNo(this->currentScope->getChildNo() + 1);
            this->currentScope = scope;
        }
        //cout << "New ScopeTable with id " << this->currentScope->getScopeName() << " created" << endl;
        return scope;
    }

    ScopeTable* exitScope(){
        if(this->currentScope==nullptr){
            //cout << "No Current Scope" << endl;
            return this->currentScope;
        }
        //cout << "ScopeTable with id " << this->currentScope->getScopeName() << " removed" << endl;
        if(this->currentScope->getParentScope()==nullptr){
            //cout << "Destroying the First Scope" << endl;
        }
        ScopeTable* temp = this->currentScope->getParentScope();
        delete this->currentScope;
        this->currentScope = temp;
        return this->currentScope;
    }

    SymbolInfo* lookUp(string name,int index){
        SymbolInfo* si = nullptr;
        ScopeTable* scope = this->currentScope;
        while(scope!=nullptr){
            si = scope->lookUpScope(name,index);
            if(si!=nullptr){
                return si;
            }
            else{
                scope = scope->getParentScope();
            }
        }
        return nullptr;
    }

    void printCurrentScope(){
        if(this->currentScope!=nullptr){
            this->currentScope->printScope();
        }
    }

    void printAllScope(){
        ScopeTable* scope = this->currentScope;
        while(scope!=nullptr){
            scope->printScope();
            cout << endl;
            scope = scope->getParentScope();
        }
    }

    ~SymbolTable(){
        ScopeTable* temp = this->currentScope;
        ScopeTable *st = nullptr;
        while(temp != NULL){
            st = temp->getParentScope();
            delete temp;
            temp = st;
        }
        //cout << "Destroying the symbol table" << endl;
    }
};
