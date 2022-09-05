#include <bits/stdc++.h>

#define NEWLINE string("\r\n")

ofstream asmFile, optimizedFile;
ifstream asmForOptimizeFile;

bool isNumber(string str)
{
    for (char c : str) {
        if (isdigit(c) == 0){
            return false;
        }
    }
    return true;
}

int strToInt(string str)
{
    stringstream ss; 
    int num;
    ss << str;
    ss >> num;
    return num;
}

bool isJump(string s)
{
    if (s == "JMP" || s == "JE" || s == "JL" || s == "JLE" || s == "JG" || s == "JGE" ||
        s == "JNE" || s == "JNL" || s == "JNLE" || s == "JNG" || s == "JNGE")
    {
        return true;
    }
    return false;
}

vector<string> split(string s){
    vector<string> tokens;
    string token = "";
    string str = "";
    for(int i=0;i<s.length();i++){
        if(s[i]!='\t'){
            str += s[i];
        }
    }
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == ' ' || str[i] == ',' || str[i] == '\t') {
            if (token != "") {
                tokens.push_back(token);
                token = "";
            }
        }
        else {
            token += str[i];
        }
    }

    if (token != "") {
        tokens.push_back(token);
    }

    return tokens;
}

void optimizeCodeSegment(int pass)
{
    string line, nextLine;
	vector<string> portions, nextPortions;
   if(pass == 1){
        asmForOptimizeFile.open("code.asm");
    }
    else{
        asmForOptimizeFile.open("optimized_code.asm");
    }
    vector<string> lineVector;
    while (getline(asmForOptimizeFile,line)){
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
        if(line[0] == ';'){
            continue;
        }
        if((split(line).size()==0)){
            continue;
        }
        lineVector.push_back(line);
    }
		
    for (int i = 0; i < lineVector.size()-1; i++)
    {
        if((split(lineVector[i]).size()==0)){
            continue;
        }
        
        line=lineVector[i];
        nextLine=lineVector[i+1];
        portions=split(line);
        nextPortions=split(nextLine);
        if(portions[0]=="PUSH"){
            if(nextPortions[0]=="POP"){ 
                if(portions[1]==nextPortions[1]){ // PUSH AX ; POP AX
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";" + lineVector[i];
                    lineVector[i+1]=";"+lineVector[i+1];
                }
            }
        }
        if(portions[0]=="POP"){
            if(nextPortions[0]=="PUSH"){
                if(portions[1]==nextPortions[1]){ // POP AX ; PUSH AX
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";" + lineVector[i];
                    lineVector[i+1]=";"+lineVector[i+1];
                }
            }
        }
        if(portions[0]=="MOV"){
            if(portions[1]==portions[2]){ // MOV AX, AX
                lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
            }
            if(nextPortions[0]=="MOV"){ 
                if((portions[1]==nextPortions[2]) && (portions[2]==nextPortions[1])){ // MOV AX, BX ; MOV BX, AX
                    lineVector[i+1]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i+1];
                }
                if(portions[1]==nextPortions[1]){ // MOV AX, BX ; MOV AX, CX or MOV AX, BX ; MOV AX, BX
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                }
            }
        }
        if(nextPortions[0]=="MOV"){
            if(nextPortions[1]==nextPortions[2]){ // MOV AX, AX
                lineVector[i+1]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i+1];
            }
        }
        if(portions[0] == "ADD" || portions[0] == "SUB"){
            if(isNumber(portions[2])){
                if(strToInt(portions[2]) == 0){
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                }
            }
        }
        if(portions[0] == "IMUL" || portions[0] == "IDIV"){
            if(isNumber(portions[2])){
                if(strToInt(portions[2]) == 1){
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                }
            }
        }
        if((portions[0] == "ADD" && nextPortions[0] == "ADD") || (portions[0] == "SUB" && nextPortions[0] == "SUB")){
            if(isNumber(portions[2]) && isNumber(nextPortions[2])){
                if(portions[1]==nextPortions[1]){
                    lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
                    int x = strToInt(portions[2]) + strToInt(nextPortions[2]);
                    lineVector[i+1]=portions[0] + " " + portions[1] + ", " + to_string(x);
                }
            }
        }
        if(isJump(portions[0]) && portions[1] == getLabel(nextPortions[0])){
            lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
        }
        if(portions[0] == "CMP" && !isJump((nextPortions[0]))){
            lineVector[i]=";----Optimized Code----" + NEWLINE + ";"+lineVector[i];
        }
    }
    for (int i = 0; i < lineVector.size(); i++){
        optimizedFile << "\t\t" + lineVector[i] << endl;
    }
    optimizedFile << "END MAIN" + NEWLINE;
    asmForOptimizeFile.close();
}
