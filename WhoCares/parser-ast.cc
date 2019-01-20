#include "parser-ast.h"
#include <fstream>
#include "token.h"



static std::string IdentifierStr;
static double NumVal;              // Filled in if tok_number
// this stores string or regex string
static std::string StringStr;
static DebugInfo DBG;
int isquotation(const int Ch) {
	return (Ch == '\"' || Ch == '\'' || Ch == '`');
}
enum {
	gettok_default_return = -1,
	gettok_return_last_char = -2,
	gettok_peek = -3,
};
//TODO: This seems a bit messed up
static int gettok(std::stringstream& ss, int return_what) {
	static int LastChar = ' ';
	static std::map<std::string, int> keywords = {
		{"include", tok_include },
		{"if", tok_if},
		{"else", tok_else},
	};
	static std::map<std::string, int> operators = {
#define MAP_OPS(n, s, p) \
	 {s, tok_##n},
		TOKEN_LIST(MAP_OPS, MAP_OPS)
#undef MAP_OPS
	};
	if (return_what == gettok_return_last_char) {
		return LastChar;
	}
	if (return_what >= 0) {
		LastChar = return_what;
		return LastChar;
	}
	// Skip any whitespace.
	while (isspace(LastChar)) {
		if (LastChar == '\n' && return_what != gettok_peek)
			DBG.inc_line();
		LastChar = ss.get();
	}

	if (isalpha(LastChar) || LastChar == '_' || LastChar == '$') { // identifier: [a-zA-Z$][a-zA-Z0-9$]*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = ss.get())) || LastChar == '_' || LastChar == '$')
			IdentifierStr += LastChar;
		auto keyw = keywords.find(IdentifierStr);
		if (keyw != keywords.end())
			return keyw->second;
		return tok_identifier;
	}
	//TODO: Bug, this also accept something like 1.2.3.4
	if (isdigit(LastChar) || (LastChar == '.' && isdigit(ss.peek()))) {   // Number: [0-9.]+
		std::string NumStr;
		do {
			NumStr += LastChar;
			LastChar = ss.get();
		} while (isdigit(LastChar) || LastChar == '.');

		NumVal = strtod(NumStr.c_str(), 0);
		return tok_number;
	}
	if (LastChar == '/' && (ss.peek() == '/')) { //Comment: '//'
		// Comment until end of line.
		do LastChar = ss.get();
		while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF)
			return gettok(ss, return_what);
	}
	// we have check it is not a comment ^^^up here^^^
	if (isquotation(LastChar) || LastChar == '/') {
		int end_quote = LastChar;
		LastChar = ss.get(); //eat first ' or " or `
		StringStr = LastChar;
		while (LastChar != end_quote)
		{
			LastChar = ss.get();
			StringStr += LastChar;
		}
		StringStr.pop_back();
		LastChar = ss.get();
		if (end_quote == '/') return tok_regex;
		return tok_string;
	}
	if (LastChar == '<') {
		auto last = ss.tellg();
		int next = ss.get();
		while (next == ' ') {
			next = ss.get();
		}
		if (isalpha(next) || next == '$' || next == '_')
		{
			while (isalnum(next) || next == '$' || next == '_')
				next = ss.get();
			while (next == ' ')
				next = ss.get();
			if (next == '>') {
				ss.seekg(last);
				LastChar = ss.get();
				return tok_template_start;
			}
		}
		ss.seekg(last);
	}
	// we have to be greedy and check 
	// operator consists of 2 character first
	std::string PotentialOperators;
	PotentialOperators = LastChar;
	PotentialOperators += ss.peek();
	auto itr = operators.find(PotentialOperators);
	if (itr != operators.end()) {
		ss.get();
		LastChar = ss.get();
		return itr->second;
	}
	PotentialOperators = LastChar;
	itr = operators.find(PotentialOperators);
	if (itr != operators.end()) {
		LastChar = ss.get();
		return itr->second;
	}
	// Check for end of file.  Don't eat the EOF.
	if (LastChar == EOF) {
		LastChar = ss.get();
		return tok_eof;
	}
	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = ss.get();
	return ThisChar;
}

static int peektok(std::stringstream& ss) {
	std::streampos p_orig = ss.tellg();
	std::string last_indentifier = IdentifierStr;
	std::string last_string = StringStr;
	int last_char = gettok(ss, gettok_return_last_char);
	int tok = gettok(ss, gettok_peek);
	ss.clear();
	ss.seekg(p_orig);
	IdentifierStr = last_indentifier;
	StringStr = last_string;
	gettok(ss, last_char);
	return tok;
}

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

static ExprAST* ParseBinOpRHS(int ExprPrec, ExprAST* LHS, K_INPUT);
static ExprAST* ParseExpression(K_INPUT);
static ExprAST *ParsePrimary(K_INPUT);
/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken(K_INPUT) {
	return CurTok = gettok(K_INPUT_PASSON, gettok_default_return);
}

/// Error* - These are little helper functions for error handling.
ExprAST *LogError(const char *Str) { fprintf(stderr, "line[%d] Error: %s\n", DBG.line(), Str); return 0; }
VariableExprAST *LogVariableError(const char *Str) { fprintf(stderr, "Error: %s\n", Str); return 0; }
/// numberexpr ::= number
static ExprAST *ParseNumberExpr(K_INPUT) {
	double flag = 1;
	if (CurTok == Token::tok_sub) {
		getNextToken(K_INPUT_PASSON); // consume '-'
		flag = -flag;
	}
	if (CurTok != Token::tok_number)
		return LogError("Expected number after unary '-' operator.");
	auto Result = new NumberExprAST(DBG, flag * NumVal);
	getNextToken(K_INPUT_PASSON); // consume the number
	return Result;
}

/// stringexpr ::= string
static ExprAST *ParseStringExpr(K_INPUT) {
	auto Result = new StringExprAST(DBG, StringStr);
	getNextToken(K_INPUT_PASSON);
	return Result;
}

static ExprAST *ParseRegexExpr(K_INPUT) {
	auto Result = new RegexExprAST(DBG, StringStr);
	getNextToken(K_INPUT_PASSON);
	return Result;
}

/// Arguments (A, B, C)
static ExprAST *__ParseArguments(std::vector<ExprAST*>& Args, K_INPUT) {
	getNextToken(K_INPUT_PASSON);  // eat (
	if (CurTok != ')') {
		while (1) {
			ExprAST *Arg = ParsePrimary(K_INPUT_PASSON);
			if (!Arg) return nullptr;
			Args.push_back(Arg);

			if (CurTok == ')') break;

			if (CurTok != ',')
				return LogError("Expected ')' or ',' in argument list");
			getNextToken(K_INPUT_PASSON);
		}
	}
	// Eat the ')'.
	getNextToken(K_INPUT_PASSON);
	return nullptr;
}

static BlockExprAST *ParseBlockExpr(K_INPUT) {
	std::vector<ExprAST *> Expressions;
	getNextToken(K_INPUT_PASSON); // Eat the '{'
	if (CurTok != tok_rbrace) {
		while (1) {
			ExprAST *Expr = ParseExpression(K_INPUT_PASSON);
			if (!Expr) continue;
			Expressions.push_back(Expr);

			if (CurTok == tok_rbrace) break;
		}
	}
	// Eat the '}'
	getNextToken(K_INPUT_PASSON);
	return new BlockExprAST(DBG, Expressions);
}

static CommaExprAST *ParseCommaExprAST(K_INPUT) {
	std::vector<ExprAST *> Expressions;
	while (1) {
		auto primary = ParsePrimary(K_INPUT_PASSON);
		if(primary)
			Expressions.push_back(primary);

		if (CurTok != ',') break;

		getNextToken(K_INPUT_PASSON); // eat ','
	}
	return new CommaExprAST(DBG, Expressions);
}
/*
static ExprAST *ParseAccessExprAST(K_INPUT) {
	std::vector<ExprAST *> Expressions;
	while (1) {
		auto primary = ParsePrimary(K_INPUT_PASSON);
		if (primary)
			Expressions.push_back(primary);

		if (CurTok != '.') break;

		getNextToken(K_INPUT_PASSON); // eat '.'
	}
	return new AccessExprAST(DBG, Expressions);
}
*/
// something like this () => {}
static ExprAST *ParseLambdaExprAST(K_INPUT) {
	std::vector<ExprAST*> Args;

	__ParseArguments(Args, K_INPUT_PASSON);

	if (CurTok != tok_lambda_goes_to)
		return LogError("Expected '=>' for lambda");

	getNextToken(K_INPUT_PASSON);

	return new LambdaExprAST(DBG, ParseBlockExpr(K_INPUT_PASSON), Args);
}
/// IfExpr
///   ::= if(...) ...
///   ::= if(...) ... else ...
static ExprAST *ParseIfExprAST(K_INPUT) {
	getNextToken(K_INPUT_PASSON); // eat if
	if (CurTok != '(')
		return LogError("Expected '(' to indicate condition for if statement");
	getNextToken(K_INPUT_PASSON); // eat (

	ExprAST *Cond = ParseExpression(K_INPUT_PASSON);
	if (CurTok != ')')
		return LogError("Expected ')' to end the condition expression");
	getNextToken(K_INPUT_PASSON); // eat )

	ExprAST *Then = ParseExpression(K_INPUT_PASSON);
	if(Then == nullptr)
		return LogError("Expected valid statement for if");

	ExprAST *Else = nullptr;
	if (CurTok == Token::tok_else) {
		getNextToken(K_INPUT_PASSON); // eat else
		Else = ParseExpression(K_INPUT_PASSON);
	}

	return new IfExprAST(DBG, Cond, Then, Else);
}

static LabelExprAST *__ParseLabelExprAST(VariableExprAST *label_, K_INPUT) {
	getNextToken(K_INPUT_PASSON); // eat ':'
	return new LabelExprAST(DBG, label_, ParseExpression(K_INPUT_PASSON));
}


/// Variable Expr
///    accept: id<template>#tag1#tag2
///            id<template>
///            id#tag1
///            <template>#tag1
static VariableExprAST *__ParseVariableExprAST(std::string IdName, K_INPUT) {
	std::string template_ = "";
	std::vector<std::string> tags_;
	if (CurTok == tok_template_start) {
		getNextToken(K_INPUT_PASSON); // eat '<'
		template_ = IdentifierStr;
		getNextToken(K_INPUT_PASSON); // eat identifier
		if (CurTok != tok_greater_than) return LogVariableError("Expected '>' for template ending.");
		getNextToken(K_INPUT_PASSON); // eat '>'
	}
	if (template_.size() == 0 && IdName.size() == 0) {
		return LogVariableError("Cannot Determine Variable.");
	}
	while (CurTok == '#') {
		getNextToken(K_INPUT_PASSON); // eat '#'
		if(CurTok != tok_identifier) return LogVariableError("Expected identifier after for '#'.");
		tags_.push_back(IdentifierStr);
		getNextToken(K_INPUT_PASSON); // eat identifier
	}
	return new VariableExprAST(DBG, IdName, template_, tags_);
}


static CallExprAST* __ParseCallExprAST(std::string& IdName, K_INPUT) {
	std::vector<ExprAST*> Args;
	__ParseArguments(Args, K_INPUT_PASSON);
	return new CallExprAST(DBG, IdName, Args);
}
/// identifierexpr
///   ::= call expr
///   ::= identifier with template
///   ::= labeled expr
static ExprAST *ParseIdentifierExpr(K_INPUT) {
	std::string IdName = "";
	std::vector<ExprAST *> exprs;
	
	while (1)
	{
		if (CurTok != tok_template_start) {
			IdName = IdentifierStr;
			getNextToken(K_INPUT_PASSON);  // eat identifier
		}
		// Call.
		if (CurTok == '(') {
			exprs.push_back(__ParseCallExprAST(IdName, K_INPUT_PASSON));
		}
		// this is a variable/template/label
		else {
			
			// eat all stuff like id<template>#tag1#tag2
			auto var_ = __ParseVariableExprAST(IdName, K_INPUT_PASSON);
			// check if it is a label
			if (CurTok == ':')
				return __ParseLabelExprAST(var_, K_INPUT_PASSON);
			exprs.push_back(var_);
		}
		if (CurTok != '.') break; 
		getNextToken(K_INPUT_PASSON); // eat .
	}
	
	// getNextToken(K_INPUT_PASSON); //eat whatever is left
	if (exprs.size() == 1) return exprs[0];
	return new AccessExprAST(DBG, exprs);
	
}


/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static ExprAST *ParsePrimary(K_INPUT) {
	switch (CurTok) {
	default:
		return LogError("unknown token when expecting an expression");
	case tok_template_start: // fall through
	case tok_identifier: return ParseIdentifierExpr(K_INPUT_PASSON);
	case tok_sub: // fall through
	case tok_number:     return ParseNumberExpr(K_INPUT_PASSON);
	case tok_string:     return ParseStringExpr(K_INPUT_PASSON);
	case tok_lbrace:     return ParseBlockExpr(K_INPUT_PASSON);
	case tok_regex:      return ParseRegexExpr(K_INPUT_PASSON);
	case '(':            return ParseLambdaExprAST(K_INPUT_PASSON);
	case tok_if:         return ParseIfExprAST(K_INPUT_PASSON); // tok_else is expected be be consumed in ParseIfExprAST()
	}
}

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static std::map<int, int> BinopPrecedence = {
#define PREC(n, s, p) {tok_##n, p},
#define DISCARD(...) 
	TOKEN_LIST(PREC, DISCARD)
#undef PREC
#undef DISCARD
};

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
	//if (!isascii(CurTok))
	//	return -1;
	if (BinopPrecedence.find(CurTok) == BinopPrecedence.end())
		return -1;
	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[CurTok];
	if (TokPrec <= 0) return -1;
	return TokPrec;
}

/// binoprhs
///   ::= ('+' primary)*
static ExprAST* ParseBinOpRHS(int ExprPrec, ExprAST* LHS, K_INPUT) {
	// If this is a binop, find its precedence.
	while (1) {
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec)
			return LHS;

		// Okay, we know this is a binop.
		int BinOp = CurTok;
		getNextToken(K_INPUT_PASSON);  // eat binop

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimary(K_INPUT_PASSON);
		if (!RHS)
			return nullptr;
		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS), K_INPUT_PASSON);
			if (!RHS)
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS = new BinaryExprAST(DBG, BinOp, std::move(LHS), std::move(RHS));
	}
}

/// expression
///   ::= primary binoprhs
///
static ExprAST* ParseExpression(K_INPUT) {
	int next = peektok(K_INPUT_PASSON);
	if (next == ',')
		return ParseCommaExprAST(K_INPUT_PASSON);
	// ExprAST* LHS = nullptr;
	// if (next == '.')
		//LHS = ParseAccessExprAST(K_INPUT_PASSON);
	//else 
	auto LHS = ParsePrimary(K_INPUT_PASSON);
	if (!LHS) {
		getNextToken(K_INPUT_PASSON); // recover
		return nullptr;
	}
	return ParseBinOpRHS(0, std::move(LHS), K_INPUT_PASSON);
}

void IncludeHelper(K_INPUT, std::vector<ExprAST *>&Expressions, fs::path& current_dir);
void ParseALLHelper(K_INPUT, std::vector<ExprAST *>&Expressions, fs::path& current_dir, std::string& current_file) {
	DBG.init(
		std::move(DebugInfo::StrPtr(std::make_shared<std::string>((current_dir / current_file).string()))),
		std::move(DebugInfo::StrPtr(std::make_shared<std::string>(ss.str())))
	);
	getNextToken(K_INPUT_PASSON);
	while (CurTok != tok_eof)
	{
		if (CurTok == tok_include) 
			IncludeHelper(K_INPUT_PASSON, Expressions, current_dir);
		auto expr = ParseExpression(K_INPUT_PASSON);
		if (expr)
			Expressions.push_back(expr);
	}
}
// this import is just like 
//TODO: prevent recursive import
void IncludeHelper(K_INPUT, std::vector<ExprAST *>&Expressions, fs::path& current_dir) {
	getNextToken(K_INPUT_PASSON);
	if (CurTok != tok_string) {
		LogError("Expected String for file path.");
		return;
	}
	
	auto anotherpath = current_dir / StringStr;
	std::ifstream is(current_dir / StringStr);
	if (!is.good()) {
		LogError("Unable to find file to import");
		return;
	}
	std::stringstream buf;
	buf << is.rdbuf();
	DebugInfo dbg(DBG);
	is.close();
	ParseALLHelper(buf, Expressions, current_dir, StringStr);
	getNextToken(K_INPUT_PASSON); // restore from eof
	DBG = dbg;
	getNextToken(K_INPUT_PASSON); // eat next token
}

BlockExprAST* ParseALL(K_INPUT, fs::path current_dir, std::string current_file) {
	std::vector<ExprAST *>Expressions;
	ParseALLHelper(K_INPUT_PASSON, Expressions, current_dir, current_file);
	return new BlockExprAST(DBG, Expressions);
}

#define DECL_ACCEPT(type)        \
void type::accept(Visitor *v) {  \
	v->visit##type(this);        \
}
AST_LIST(DECL_ACCEPT)
#undef DECL_ACCEPT
