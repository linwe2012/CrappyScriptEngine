#pragma once
#include "parser-ast.h"
#define COMPILE_ERROR_LIST(V, S)                 \
     /* Error node Type */            \
	V(Template_Error)                 \
	V(Variable_Error)                 \
	V(Variable_Template_Error)        \
	V(Variable_Tag_Error)             \
    V(Argument_Error)                 \
	V(Label_Error)                    \
		                              \
	/* Error extra info */            \
	V(Declaration)                    \
	V(nUsage)                         \
	V(Not_Found)                      \
                                      \
    /* Error specify */               \
	V(Duplicated)                     \
	V(Invalid)                        \
	V(Wrong_Type)                     \
	V(Ambiguous)                      \
	V(Empty)                          \
	V(Misplaced)                      \
	V(Unable_To_Match)                \
	V(Too_Few_Arguments)              \
                                      \
    /* default solution */            \
	S(Ignore)                         \
	S(Override)                       







namespace CompileDebugger {
	typedef int ErrorType;
	enum
	{
#define K_ENUM(e) kE_HIDE_FROM_OTHERS_PLEASE_DONT_USE_THIS_##e,
		COMPILE_ERROR_LIST(K_ENUM, K_ENUM)
#undef K_ENUM
	};
	enum ErrId {
#define EID_ENUM(e) E_##e = 1 << kE_HIDE_FROM_OTHERS_PLEASE_DONT_USE_THIS_##e,
#define SID_ENUM(s) S_##s = 1 << kE_HIDE_FROM_OTHERS_PLEASE_DONT_USE_THIS_##s,
		COMPILE_ERROR_LIST(EID_ENUM, SID_ENUM)
#undef SID_ENUM
#undef EID_ENUM
		Expected_Variable_Identifier = 1 << 31,
	};
	/*
	enum ErrId {
		// Error node Type
		E_Template_Error = 1 << 1,
		E_Variable_Error = 1 << 2,
		E_Variable_Template_Error = 1 << 3,
		E_Variable_Tag_Error = 1 << 4,
		E_Argument_Error = 1 << 5,
		E_Label_Error = 1 << 6,

		// Error extra info
		E_Declaration = 1 << 9,
		E_nUsage = 1 << 10,
		E_Not_Found = 1 << 11,

		// Error specify
		E_Duplicated = 1 << 15,
		E_Invalid = 1 << 16,
		E_Wrong_Type = 1 << 17,
		E_Ambiguous = 1 << 18,
		E_Empty = 1 << 19,
		E_Misplaced = 1 << 20,
		E_Unable_To_Match = 1 << 21,
		E_Too_Few_Arguments = 1 << 22,

		// default solution
		S_Ignore = 1 << 30,
		S_Override = 1 << 31,

		Expected_Variable_Identifier = 1 << 25,
	};
	*/
	void initLogging();
	void endLogging();
	//TODO: Disable extra info default, forcing compiler write extra info
	void logError(ExprAST *error_point, std::string initiator, ErrorType what_error, std::string extra_info="");
	void logWarning(ExprAST *error_point, std::string initiator, ErrorType what_error, std::string extra_info="");
}
