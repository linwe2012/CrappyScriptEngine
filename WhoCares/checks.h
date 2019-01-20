#ifndef _CHECKS_H_
#define _CHECKS_H_

#include "macro-utils.h"

#include <stdio.h>
#include <string.h>
#include <string>
#define DEBUG

#define ERROR_TYPES(V)                                                         \
	V(TEMPLATE, "Template Error")                                              \
	V(UNEXPECTED_NULL_PTR, "Unexpected nullptr as function parameter")         \
	V(EXPECTED_GAME_ENTRY, "Expected entry for the game")                      \
	V(GAME_FILE_ENTRY_NOT_FOUND, "The file specified by .game is not found.\n" \
     "Pleace check .game and entry file is under the same directory")

#define ENUM_EID(e, d) kEID_##e,
enum ERROR_IDS {
	kEID_NO_ERROR = 0,
	ERROR_TYPES(ENUM_EID)
};
#undef ENUM_EID


struct ERROR_INTERFACE
{
	virtual const char *error_name() = 0;
	virtual const char *error_description() = 0;
	virtual int error_id() = 0;
	virtual ~ERROR_INTERFACE() {}
};
#define DECL_ERROR_TYPE(e, d)                                                                      \
	struct ERROR_##e : public ERROR_INTERFACE{                                                     \
		const char *error_name() override { constexpr static const char *res = #e; return res; }         \
		const char *error_description() override { constexpr static const char *res = d; return res; }   \
		int error_id() override { return kEID_##e; }                                               \
	};
ERROR_TYPES(DECL_ERROR_TYPE)
#undef DECL_ERROR_TYPE

namespace Debugger {
	void logError(ERROR_INTERFACE* what_error, std::string initiater);
}

#ifdef DEBUG
#define ASSERT(condition, what_error, initiater) do \
if(!(condition)) { \
	fprintf(stderr, "Assertion Failed: %s\n", #condition); \
	fprintf(stderr, "Error(%d): %s\n", what_error.error_id(), what_error.error_name()); \
	fprintf(stderr, "initiater: %s\n", initiater);\
	throw what_error; \
}while (0)
#else
#define ASSERT(...) (void) 0

#endif // DEBUG



#endif // !_CHECKS_H_
