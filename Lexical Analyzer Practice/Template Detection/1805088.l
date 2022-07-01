%option noyywrap

%{
    #include<iostream>
    #include<bits/stdc++.h>
    #include <algorithm>
    #include <vector>
    using namespace std;

    FILE *logout;
    int line_count=1, l_count=0, r_count=0, l_count1=0;
    char prevChar='a';
    string str="", prevStr="";
    bool isInvalid=false;
    std::vector<std::string> v;
%}

WHITESPACE [ \t\f\r\v]+
NEWLINE \r?\n
LETTER [a-zA-Z]
ID {LETTER}+
TEMPLATE_START <
TEMPLATE_END >

%%

{WHITESPACE}+ {}
{NEWLINE} { 
    if(isInvalid){
        fprintf(logout,"Invalid template at line no %d: %s found\n",line_count, str.data());
    }
    else if(r_count!=0 || l_count1!=r_count){
        fprintf(logout,"Unfinished template at line no %d: %s found\n",line_count, str.data());
    }
    else{
        fprintf(logout,"Template detected at line no %d: %s found\n",line_count, str.data());
    }
    line_count++;
    l_count=0, r_count=0, l_count1=0; 
	str="", prevStr="";
    isInvalid = false;
    v.clear();
}
{ID} {
    if(l_count>=1){
        l_count--;
    }
    prevStr = yytext;
    if (std::find(v.begin(), v.end(), prevStr) != v.end()){
        isInvalid = true;
    }   
    v.push_back(prevStr);
    str.append(yytext);
}
{TEMPLATE_START} {
    if(str.compare("")==0){
        isInvalid = true;
    }
    if(l_count!=0){
        isInvalid = true; 
    }
    if(prevChar=='<' || prevChar=='>'){
        isInvalid = true;
    }
    if(!isInvalid){
        l_count++;
        r_count++;
        l_count1++;
    }
    str.append(yytext);
}

{TEMPLATE_END} {
    if(r_count==0){
        isInvalid = true;
    }
    if(prevChar=='<'){
        isInvalid = true;
    }
    if(!isInvalid){
        r_count--;
        l_count1--;
    }
    str.append(yytext);
}

<<EOF>> {
    return 0;
}

%%

int main(int argc, char* argv[]){
    if(argc!=2){
        cout << "Please provide input file name and try again" << endl;
        return 0;
    }
    FILE *fin = fopen(argv[1],"r");
    if(fin==NULL){
        cout << "Can't open specified file!!" << endl;
        return 0;
    }

    logout= fopen("log.txt","w");

    yyin = fin;
    yylex();
    fclose(yyin);
    return 0;
}

