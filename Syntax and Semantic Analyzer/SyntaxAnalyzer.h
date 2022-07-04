#include "SymbolTable.h"
#include<stack>

#define NoOfBuckets 7
#define ARRAY "ARR"
#define VARIABLE "VAR"
#define FUNCTION "FUNC"
#define INT_TYPE "INT"
#define FLOAT_TYPE "FLOAT"
#define VOID_TYPE "VOID"
#define CHAR_TYPE "CHAR"

FILE *logout, *errorout, *parserout;
extern int line_count;
extern int error_count;
extern FILE *yyin;
extern char *yytext;
string variableType;

SymbolTable* symbolTable = new SymbolTable();

int yyparse();
int yylex();

string ruleName[] = {"start", "program", "unit", "func_declaration", "func_definition", "parameter_list",
                           "compound_statement", "var_declaration", "type_specifier", "declaration_list", "statements",
                           "statement", "expression_statement", "variable", "expression", "logic_expression",
                           "rel_expression", "simple_expression", "term", "unary_expression", "factor", "argument_list",
                           "arguments"};

enum NONTERMINAL_TYPE {
	start = 0, program, unit, func_declaration, func_definition,
	parameter_list, compound_statement, var_declaration, type_specifier,
	declaration_list, statements, statement, expression_statement,
	variable, expression, logic_expression, rel_expression, simple_expression,
	term, unary_expression, factor, argument_list, arguments, error
};

bool replaceAll(string source, string toReplace, string replaceBy) {
	if (source.find(toReplace, 0) == string::npos){
		return false;
	}
	for (string::size_type i = 0; (i = source.find(toReplace, i)) != string::npos;) {
		source.replace(i, toReplace.length(), replaceBy);
		i += replaceBy.length();
	}
	return true;
}

string formatCode(string code){
	string formattedCode = code;
	while (replaceAll(formattedCode, " ;", ";"));
	replaceAll(formattedCode, ";", ";\n");
	replaceAll(formattedCode, ";\n\n", ";\n");
	while (replaceAll(formattedCode, "( ", "("));
	while (replaceAll(formattedCode, " (", "("));
	while (replaceAll(formattedCode, ") ", ")"));
	while (replaceAll(formattedCode, " )", ")"));
	while (replaceAll(formattedCode, "{ ", "{"));
	while (replaceAll(formattedCode, " {", "{"));
	while (replaceAll(formattedCode, "} ", "}"));
	while (replaceAll(formattedCode, " }", "}"));
	replaceAll(formattedCode, "{", "\n{\n");
	replaceAll(formattedCode, "}", "\n}\n");
	while (replaceAll(formattedCode, "\n\n}", "\n}"));
	while (replaceAll(formattedCode, "}\n\n", "}\n"));
	while (replaceAll(formattedCode, "  = ", " = "));
	while (replaceAll(formattedCode, " =  ", " = "));
	while (replaceAll(formattedCode, "  == ", " == "));
	while (replaceAll(formattedCode, " ==  ", " == "));
	replaceAll(formattedCode, "(", " ( ");
	replaceAll(formattedCode, ")", " ) ");
	while (replaceAll(formattedCode, " ++", "++"));
	while (replaceAll(formattedCode, " --", "--"));
	while (replaceAll(formattedCode, "\n ", "\n"));
	while (replaceAll(formattedCode, " \n", "\n"));
	while (replaceAll(formattedCode, "  ", " "));
	return formattedCode;
}

class NonTerminalHandler
{
private:
	stack<string> nonterminals[NONTERMINAL_TYPE::error +1];
public:
	string getValue(NONTERMINAL_TYPE nonterminal){
		return nonterminals[nonterminal].top();
	}
	void setValue(NONTERMINAL_TYPE nonterminal, string value){
		nonterminals[nonterminal].push(value);
	}
};

NonTerminalHandler nonTerminalHandler;

void yyerror(const char *s) {
	cout << "Error at line: " << line_count << endl;
}

SymbolInfo* getSymbolInfoOfType(string type){
	variableType = type;
	return new SymbolInfo(variableType,variableType);
}

void setValue(NONTERMINAL_TYPE nonterminal, string value){
	nonTerminalHandler.setValue(nonterminal,value);
}

void printRuleAndCode(NONTERMINAL_TYPE nonterminal, string rule){
	fprintf(logout,"Line %d: %s : %s\n",line_count, ruleName[nonterminal].data(), rule.data());
	fprintf(logout,"%s\n", formatCode(nonTerminalHandler.getValue(nonterminal)).data());
}

