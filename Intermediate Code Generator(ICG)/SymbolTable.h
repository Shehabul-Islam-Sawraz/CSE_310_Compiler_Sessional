#include<iostream>
#include<bits/stdc++.h>

using namespace std;

//FILE *logout, *tokenout;

#define NoOfBuckets 7
#define ARRAY "ARR"
#define VARIABLE "VAR"
#define FUNCTION "FUNC"
#define INT_TYPE "INT"
#define FLOAT_TYPE "FLOAT"
#define VOID_TYPE "VOID"
#define CHAR_TYPE "CHAR"
#define TEMPORARY_TYPE "TEMP"

class SymbolInfo
{
private:
    SymbolInfo* previous;
    SymbolInfo* next;
    string name, type;
    string decType; // Stores which one is declared!! FUNCTION, VARIABLE, ARRAY
    string varType; // Stores variable type!! INTEGER, FLOAT, DOUBLE, VOID
    bool isFuncDeclared = false; // Is a function have a body or not
    string funcRetType; // Stores return type of function
    vector<string> paramType; // Stores the types of the parameters
    size_t arrSize; // Stores size of the array
    //size_t arrIndex; // Stores which index we are accessing
    // int defInt = 0; // Set default value to 0 to a INT_TYPE variable until some value is assigned into it
    // float defFloat = 0.0; // Set default value to 0.0 to a FLOAT_TYPE variable until some value is assigned into it
    int scopeId = -1;
    string funcRetLabel = "";
    string arrIndex = "";
    int offset;

public:
    //vector<int> intValues; // Stores array values if array is INT_TYPE. Default value is set to 0
    //vector<float> floatValues; // Stores array values if array is FLOAT_TYPE. Default value is set to 0.0
    string code;
    bool isGlobal;
    SymbolInfo(string name,string type){
        this->name = name;
        this->type = type;
        this->next = nullptr;
        this->previous = nullptr;
        this->decType = "";
        this->varType = "";
        this->isFuncDeclared = false;
        this->funcRetType = "";
        this->isGlobal = false;
        this->offset = -1;
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

    void setparamType(vector<string> paramType){
        this->paramType = paramType;
    }

    vector<string> getparamType(){
        return this->paramType;
    }

    void setDecType(string type){
        this->decType = type;
    }

    string getDecType(){
        return this->decType;
    }

    void setVarType(string type){
        this->varType = type;
        // if (type == INT_TYPE){
        //     intValues.push_back(0);
        // }   
		// else if (type == FLOAT_TYPE){
        //     floatValues.push_back(0);
        // }
    }

    string getVarType(){
        return this->varType;
    }

    void setFuncRetType(string type){
        this->funcRetType = type;
        setVarType(type);
    }

    string getFuncRetType(){
        return this->funcRetType;
    }

    void setPrevious(SymbolInfo* previous){
        this->previous = previous;
    }

    bool getIsFuncDeclared(){
        return this->isFuncDeclared;
    }

    void setIsFuncDeclared(bool isFuncDeclared){
        this->isFuncDeclared = isFuncDeclared;
    }

    size_t getArrSize() {
		return arrSize;
	}

	void setArrSize(size_t arrSize) {
		if (!isArray()){
            return;
        }
		this->arrSize = arrSize;
	}

    string getScopeID() {
		return to_string(this->scopeId);
	}

	void setScopeID(int scopeId) {
		this->scopeId = scopeId;
	}

    void setFuncRetLabel(string label){
        this->funcRetLabel = label;
    }

    string getFuncRetLabel(){
        return this->funcRetLabel;
    }

    // size_t getArrIndex() {
	// 	return isArray() ? arrIndex : 0;
	// }

	// void setArrIndex(size_t arrIndex) {
	// 	if (decType!=ARRAY){
    //         return;
    //     }
	// 	this->arrIndex = arrIndex;
	// }

    void setArrIndex(string index){
        this->arrIndex = index;
    }

    string getArrIndex(){
        return this->arrIndex;
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

    void setCode(string code){
        this->code = code;
    }

    string getCode(){
        return this->code;
    }

    bool isVariable(){
        return decType==VARIABLE;
    }

    bool isFunction(){
        return decType==FUNCTION;
    }

    bool isArray(){
        return decType==ARRAY;
    }

    bool isVoidFunc(){
        return isFunction() && getFuncRetType()==VOID_TYPE;
    }

    void setOffset(int offset){
        this->offset = offset;
    }

    int getOffset(){
        return this->offset;
    }

    // int setVarValue(int val) { // As default value was set to 0 now we change the value with the defined value
	// 	if (intValues.empty()){
    //         intValues.push_back(val);
    //     }
	// 	else{
    //         intValues[0] = val;
    //     }
	// 	return intValues[0];
	// }

	// float setVarValue(float val) {
	// 	if (floatValues.empty()){
    //         floatValues.push_back(val);
    //     }
	// 	else{
    //         floatValues[0] = val;
    //     }
	// 	return floatValues[0];
	// }

    // int &intValue(){ // Setting default value 0 to a variable initially 
    //     if(decType==VARIABLE && varType==INT_TYPE){
    //         if(!intValues.size()){
    //             intValues.push_back(0);
    //         }
    //         return intValues[0];
    //     }
    //     return defInt;
    // }

    // float &fltValue(){
    //     if(decType==VARIABLE && varType==FLOAT_TYPE){
    //         if(!floatValues.size()){
    //             floatValues.push_back(0);
    //         }
    //         return floatValues[0];
    //     }
    //     return defFloat;
    // }

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
    int id;
public:
    ScopeTable(int noOfBuckets){
        this->scopeTable = new SymbolInfo*[noOfBuckets];
        for(int i=0;i<noOfBuckets;i++){
            scopeTable[i] = nullptr;
        }
        this->noOfChild = 0;
        this->parentScope = nullptr;
        this->noOfBuckets = noOfBuckets;
        this->id = 0;
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

    void setId(int id){
        this->id = id;
    }

    void setId(){
        if(this->parentScope==nullptr){
            this->id = 1;
        }
        else{
            this->id = this->parentScope->getId()+1;
        }
    }

    int getId(){
        return this->id;
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

    bool insertSymbol(string name,string type,int index, string varType, string decType){
        if(lookUpScope(name,index)!=nullptr){
            //cout << "<" << name << ", " << type << "> already exists in current ScopeTable" << endl;
            //fprintf(logout,"< %s : %s > already exists in current ScopeTable\n",name.data(),type.data());
            return false;
        }
        SymbolInfo* symbol = new SymbolInfo(name,type);
        int x=0;
        if(scopeTable[index]==nullptr){
            symbol->setNext(nullptr);
            symbol->setPrevious(nullptr);
            symbol->setDecType(decType);
            symbol->setVarType(varType);
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
            symbol->setDecType(decType);
            symbol->setVarType(varType);
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

    void printScope(FILE* logout){
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
            scope->setId();
        }
        else{
            scope->setParentScope(this->currentScope);
            string sc = (this->currentScope->getScopeName()).append(".");
            scope->setScopeName(sc.append(to_string(this->currentScope->getChildNo() + 1)));
            this->currentScope->setChildNo(this->currentScope->getChildNo() + 1);
            this->currentScope = scope;
            scope->setId();
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

    SymbolInfo* lookUpCurrentScope(string name,int index){
        SymbolInfo* si = nullptr;
        ScopeTable* scope = this->currentScope;
        if(scope!=nullptr){
            si = scope->lookUpScope(name,index);
            if(si!=nullptr){
                return si;
            }
        }
        return nullptr;
    }

    void printCurrentScope(FILE* logout){
        if(this->currentScope!=nullptr){
            this->currentScope->printScope(logout);
        }
    }

    void printAllScope(FILE* logout){
        ScopeTable* scope = this->currentScope;
        while(scope!=nullptr){
            scope->printScope(logout);
            //cout << endl;
            fprintf(logout,"\n\n");
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
