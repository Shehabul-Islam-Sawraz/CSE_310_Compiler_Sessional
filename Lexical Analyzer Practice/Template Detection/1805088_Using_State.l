%option yylineno noyywrap

%{
    /* This can't handle all the exceptional cases. But ok to go with!!! */
    #include<iostream>
    #include<bits/stdc++.h>
    #include <algorithm>
    #include <vector>
    using namespace std;

    FILE *logout;
    int angleCount = 0;
    std::string t = "";
    int line_count=1;
    std::vector<std::string> v;
    bool repeat = false;
%}

LETTER      [a-zA-Z]
WHITESPACE  [ \t\f\r\v]+

%x END

%%
{LETTER}+"<"        {   
                        angleCount++; t += yytext; 
                        cout << "Anglecount barse" << endl;
                        std::string str = yytext; 
                        str.pop_back(); 
                        if (std::find(v.begin(), v.end(), str) != v.end()){
                            repeat = true;
                        }   
                        v.push_back(str); 
                    }

{LETTER}+">"        {   
                        BEGIN(END); 
                        angleCount--; 
                        cout << "Anglecount komse" << endl;
                        t += yytext; 
                        std::string str = yytext; 
                        str.pop_back(); 
                        if (std::find(v.begin(), v.end(), str) != v.end()){
                            repeat = true;
                        }
                        v.push_back(str); 
                    }
{WHITESPACE}       {}

<END>">"                {   
                            angleCount--;
                            cout << "Anglecount komse" << endl;     
                            t += yytext; 
                        }

<END>{WHITESPACE}       {}
<END>\n                 |
<END><<EOF>>            |
<END>.                  { 
                            BEGIN(INITIAL); 
                            cout << angleCount << endl;
                            if (repeat == true && angleCount == 0) {
                                fprintf(logout,"Invalid template %s found\n", t.data());
                            }
                            else if (angleCount == 0) {
                                fprintf(logout,"Template %s found\n", t.data());
                            } else {
                                fprintf(logout,"Unfinished template %s found\n", t.data());
                            }
                            t = ""; 
                            repeat = false;
                            angleCount=0;
                            v.clear();
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