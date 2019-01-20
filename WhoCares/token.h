#ifndef _TOKEN_H_
#define _TOKEN_H_
// name, symbol, priority
#define TOKEN_LIST(BINOP, V)              \
	BINOP(equal, "==", 10)                \
	BINOP(not_equal, "!=", 10)            \
	BINOP(lower_than, "<", 15)            \
	BINOP(greater_than, ">", 15)          \
	BINOP(lower_or_euqal, "<=", 15)       \
	BINOP(greater_or_euqal, ">=", 15)     \
	BINOP(assign, "=", 5)                 \
	BINOP(assign_add, "+=", 5)            \
	BINOP(assign_update, ":=", 5)         \
	BINOP(assign_sub, "-=",    5)         \
	BINOP(sub, "-", 50)                   \
	V(lbrace, "{", 0)                     \
	V(rbrace, "}", 0)                     \
	V(lambda_goes_to, "=>", 0)          
	                
//TODO: stuff tokens into class Token
enum PrimaryToken {
	tok_eof = -1,

	// commands & keywords
	tok_import = -2, //TODO: resuerved, as import is kinda hard to code
	tok_include = -3,
	// primary
	tok_identifier = -4,
	tok_number = -5, tok_string = -7,
	tok_regex = -11, tok_if = -12, tok_else = -13,
	tok_template_start = -14,
	tok_operatator_start = -100,
#define ENUM_OPERATOR(n, s, p) \
	tok_##n,
	TOKEN_LIST(ENUM_OPERATOR, ENUM_OPERATOR)
#undef ENUM_OPERATOR
};

class Token {
public:
	enum {
		tok_eof = -1,

		// commands & keywords
		tok_import = -2, //TODO: resuerved, as import is kinda hard to code
		tok_include = -3,
		// primary
		tok_identifier = -4,
		tok_number = -5, tok_string = -7,
		tok_regex = -11, tok_if = -12, tok_else = -13,
		tok_operatator_start = -100,
#define ENUM_OPERATOR(n, s, p) \
	tok_##n,
		TOKEN_LIST(ENUM_OPERATOR, ENUM_OPERATOR)
#undef ENUM_OPERATOR
	};

	static bool isCompare(int tok) {
		return tok <= tok_greater_or_euqal && tok >= tok_equal;
	}
	static bool isUnaryOp(int tok) {
		return tok == tok_sub;
	}
};
#endif