%option noyywrap

%{
    #include<iostream>
    #include<bits/stdc++.h>
    using namespace std;

    FILE *logout;
    int line_count=1;
%}

LETTER [a-zA-Z]
ID [a-zA-Z_][a-zA-Z0-9_]*
DIGIT [0-9]
W [ \t\f\r\v]+
RELOP "=="|"<"|">"
LOGICOP "&&"|"||"
IDNC {ID}|{DIGIT}+|{LETTER}
TERNARYR {IDNC}{W}?({RELOP}){W}?{IDNC}{W}?"?"{W}?{IDNC}{W}?":"{W}?{IDNC}
TERNARYL {IDNC}{W}?({LOGICOP}){W}?{IDNC}{W}?"?"{W}?{IDNC}{W}?":"{W}?{IDNC}
NEWLINE \r?\n

%%
(({ID}{W}?"="){W})?{IDNC}{W}?({RELOP}|{LOGICOP}){W}?{IDNC}{W}?"?"{W}?({TERNARYR}|{TERNARYL}){W}?":"{W}?({IDNC}) { 
        fprintf(logout,"Nested ternary operator found at line no %d: %s found\n",line_count, yytext);
    }
(({ID}{W}?"="){W})?{IDNC}{W}?({RELOP}|{LOGICOP}){W}?{IDNC}{W}?"?"{W}?({IDNC}){W}?":"{W}?({TERNARYR}|{TERNARYL}) { 
        fprintf(logout,"Nested ternary operator found at line no %d: %s found\n",line_count, yytext);
    }
(({ID}{W}?"="){W})?{IDNC}{W}?({RELOP}|{LOGICOP}){W}?{IDNC}{W}?"?"{W}?({TERNARYR}|{TERNARYL}){W}?":"{W}?({TERNARYR}|{TERNARYL}) { 
        fprintf(logout,"Nested ternary operator found at line no %d: %s found\n",line_count, yytext);; 
    }
({ID}{W}?"=")?{W}?{IDNC}{W}?({RELOP}){W}?{IDNC}{W}?"?"{W}?{IDNC}{W}?":"{W}?{IDNC} { 
        fprintf(logout,"Ternary oprator with RELOP found at line no %d: %s found\n",line_count, yytext);
    }
({ID}{W}?"=")?{W}?{IDNC}{W}?({LOGICOP}){W}?{IDNC}{W}?"?"{W}?{IDNC}{W}?":"{W}?{IDNC} { 
        fprintf(logout,"Ternary oprator with LOGICOP found at line no %d: %s found\n",line_count, yytext);
    }

{W}+ {}
{NEWLINE} {
            line_count++;
        }

";" {}

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