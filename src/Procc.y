%{
#include "Node.h"
#include <iostream>
#include <string>
extern int yylex();
extern void yyerror(const char* s);

// Global for the program
NProgram* gProgram = nullptr;

// Disable the warnings that Bison creates
#pragma warning(disable: 4065)
%}

/* You should not need to change the union */
%union {
    Node* node;
	NProgram* program;
	NData* data;
	NDecl* decl;
	NBlock* block;
	NStatement* statement;
	NNumeric* numeric;
	NExpr* expr;
	NComparison* comparison;
	std::string* string;
	int token;
}

%error-verbose

/* Tokens/Terminal symbols */
%token <token> TDATA TMAIN TLBRACE TRBRACE TSEMI TLPAREN TRPAREN
%token <token> TLBRACKET TRBRACKET TINC TDEC TEQUALS
%token <token> TADD TSUB TMUL TDIV
%token <token> TLESS TISEQUAL
%token <token> TVAR TARRAY
%token <token> TIF TELSE TWHILE
%token <token> TCOMMA TPENUP TPENDOWN TSETPOS TSETCOLOR TFWD TBACK TROT
%token <string> TINTEGER TIDENTIFIER

/* Types/non-terminal symbols */
%type <program> program
%type <block> main
%type <data> data decls
%type <decl> decl;
%type <numeric> numeric

/* Operator precedence */
%left TADD TSUB
%left TMUL TDIV

%%

program		: data main 
				{ 
					std::cout << "Program\n";
					gProgram = new NProgram($1, $2);
				}
;

data		: TDATA TLBRACE TRBRACE
				{
					std::cout << "Data (no decls)\n";
					$$ = new NData();
				}
			| TDATA TLBRACE decls TRBRACE
				{
					std::cout << "Data\n";
				}
;

decls		: decl 
				{
					std::cout << "Single decl\n";
				}
			| decls decl
				{
					std::cout << "Multiple decls\n";
				}
;

decl		: TVAR TIDENTIFIER TSEMI
				{
					std::cout << "Var declaration " << *($2) << '\n';
				}
			| TVAR TIDENTIFIER TEQUALS expr TSEMI
				{
					std::cout << "Var declaration " << *($2) << " with equals\n";
				}
			| TARRAY TIDENTIFIER TLBRACKET numeric TRBRACKET TSEMI
				{
					std::cout << "Array declaration " << *($2) << '\n';
				}
;

main		: TMAIN TLBRACE TRBRACE
				{
					std::cout << "Main (no stmts)\n";
					$$ = new NBlock();
				}
			| TMAIN TLBRACE block TRBRACE 
				{
					std::cout << "Main\n";
				}
;

block		: statement
				{
					std::cout << "Single statement\n";
				}
			| block statement
				{
					std::cout << "Multiple statements\n";
				}
;

statement	: TIDENTIFIER TEQUALS expr TSEMI 
				{
					std::cout << "Assignment to variable\n";
				}
			| TIDENTIFIER TLBRACKET expr TRBRACKET TEQUALS expr TSEMI
				{
					std::cout << "Assignment to array index\n";
				}
			| TINC TIDENTIFIER TSEMI
				{
					std::cout << "Pre-increment\n";
				}
			| TDEC TIDENTIFIER TSEMI
				{
					std::cout << "Pre-decrement\n";
				}
			| TIDENTIFIER TINC TSEMI
				{
					std::cout << "Post-increment\n";
				}
			| TIDENTIFIER TDEC TSEMI
				{
					std::cout << "Post-decrement\n";
				}
			| TIF comparison TLBRACE block TRBRACE TELSE TLBRACE block TRBRACE
				{
					std::cout << "if-else statement\n";
				}
			| TIF comparison TLBRACE block TRBRACE 
				{
					std::cout << "if statement\n";
				}
			| TWHILE comparison TLBRACE block TRBRACE
				{
					std::cout << "while loop\n";
				}
			| TPENUP TLPAREN TRPAREN TSEMI
				{
					std::cout << "penUp();\n";
				}
			| TPENDOWN TLPAREN TRPAREN TSEMI
				{
					std::cout << "penDown();\n";
				}
			| TSETPOS TLPAREN expr TCOMMA expr TRPAREN TSEMI
				{
					std::cout << "setPosition(int x, int y);\n";
				}
			| TSETCOLOR TLPAREN expr TRPAREN TSEMI
				{
					std::cout << "setColor(int color);\n";
				}
			| TFWD TLPAREN expr TRPAREN TSEMI
				{
					std::cout << "forward(int x);\n";
				}
			| TBACK TLPAREN expr TRPAREN TSEMI
				{
					std::cout << "back(int x);\n";
				}
			| TROT TLPAREN expr TRPAREN TSEMI
				{
					std::cout << "rotate(int x);\n";
				}
;

comparison	: expr TISEQUAL expr 
				{
					std::cout << "Comparison equals\n";
				}
			| expr TLESS expr 
				{
					std::cout << "Comparison less than\n";
				}
;

expr		: expr TADD expr 
				{ 
					std::cout << "+\n"; 
				}
			| expr TSUB expr 
				{ 
					std::cout << "-\n";
				}
			| expr TMUL expr 
				{ 
					std::cout << "*\n";
				}
			| expr TDIV expr 
				{ 
					std::cout << "/\n";
				}
			| TLPAREN expr TRPAREN 
				{ 
					std::cout << "()\n";
				}
			| TIDENTIFIER TLBRACKET expr TRBRACKET
				{
					std::cout << "Value in array\n";
				}
			| TIDENTIFIER
				{
					std::cout << "Identifier " << *($1) << '\n';
				}
			| numeric
				{
					std::cout << "Numeric expression\n";
				}
;

numeric		: TINTEGER
				{
					std::cout << "Numeric value of " << *($1) << '\n';
				}
;

%%
