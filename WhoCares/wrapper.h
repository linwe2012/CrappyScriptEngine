#ifndef _WRAPPER_H_
#define _WRAPPER_H_
#include "worldgen.h"
#include "compile-debugger.h"


// Wrapper of Runtime function

void newPrinter_wrapper(FunctionsLibrary::Sequence&sequence, CallExprAST *call, UNUSED WorldGen& world_gen) {
	IGNORE(world_gen);
	using namespace CompileDebugger;
	std::stringstream content;
	if (call->args().size() < 1)
		return logWarning(call, "newPrinter_wrapper", E_Argument_Error | E_Empty);
	for (auto arg : call->args()) {
		if (auto str = arg->getAsStringExprAST())
			content << str->str();
		else if (auto var = arg->getAsVariableExprAST())
			content << "${" << var->name() << "}";
		else
			logError(call, "newPrinter_wrapper", E_Argument_Error | E_Wrong_Type);
	}
	content << "\n";
	sequence << FunctionsLibrary::newPrinter(content.str());
	// return FunctionsLibrary::newPrinter(content.str());
}

void newActivator_wrapper(FunctionsLibrary::Sequence&sequence, CallExprAST *call, UNUSED WorldGen& world_gen) {
	IGNORE(world_gen);
	std::string here = "newActivator_wrapper";
	using namespace CompileDebugger;
	std::string activatee;
	std::string catergory;
	std::string failcall;
	std::string successcall;
	if(call->args_size() < 2)
		return logError(call, here, E_Argument_Error | E_Too_Few_Arguments, "Expected 2 arguments of activator");
	auto _1st = call->args()[0];
	auto _2nd = call->args()[1];
	if (_1st == nullptr || _2nd == nullptr) {
		return logError(call, here, E_Argument_Error | E_Too_Few_Arguments, "Expected 2 arguments of activator");

	}
	auto cat = _1st->getAsStringExprAST();
	auto str = _2nd->getAsStringExprAST();

	if (cat == nullptr && str == nullptr) 
		return logError(call, here, E_Argument_Error | E_Wrong_Type);
	activatee = str->str();
	sequence << FunctionsLibrary::newActivator(cat->str(), activatee, failcall, successcall);
	// return FunctionsLibrary::newActivator(cat->str(), activatee, failcall, successcall);
}

std::string StringExprAST2String(ExprAST *node) {
	if (node == nullptr) return std::move(std::string(""));
	auto str_ast = node->getAsStringExprAST();
	if(str_ast == nullptr) return std::move(std::string(""));
	return str_ast->str();
}
void newCallExecuable_wrapper(FunctionsLibrary::Sequence&sequence, CallExprAST *call, UNUSED WorldGen& world_gen) {
	IGNORE(world_gen);
	using namespace CompileDebugger;
	const int kMaxArgs = 3;
	std::string args[kMaxArgs];
	int cnt = 0;
	for (auto arg : call->args()) {
		args[cnt] = StringExprAST2String(arg);
		if (++cnt == kMaxArgs) break;
	}
	sequence << FunctionsLibrary::newCallExecutable(args[0], args[1], args[2]);
	// return FunctionsLibrary::newCallExecutable(args[0], args[1], args[2]);
}

void newSetFlag_wrapper(FunctionsLibrary::Sequence&sequence, CallExprAST *call, UNUSED WorldGen& world_gen) {
	IGNORE(world_gen);
	static std::string here = "newSetFlag_wrapper";
	using namespace CompileDebugger;
	if (call->args_size() < 1)
		return logError(call, here, E_Argument_Error | E_Too_Few_Arguments, "Expected at least one argument for $setflag");
	for (auto arg : call->args()) {
		if (auto str = arg->getAsStringExprAST()) {
			sequence << FunctionsLibrary::newSetFlag(str->str());
		}
		else {
			logError(call, here, E_Argument_Error | E_Wrong_Type, "Expected string for argument");
		}
	}
}

void newFireEvent_wrapper(FunctionsLibrary::Sequence&sequence, CallExprAST *call, UNUSED WorldGen& world_gen) {
	IGNORE(world_gen);
	static std::string here = "newFireEvent_wrapper";
	using namespace CompileDebugger;
	if (call->args_size() < 1)
		return logError(call, here, E_Argument_Error | E_Too_Few_Arguments, "Expected at least one argument for $fire");
	for (auto arg : call->args()) {
		if (auto str = arg->getAsStringExprAST()) {
			sequence << FunctionsLibrary::newFireEvent(str->str());
		}
		else {
			logError(call, here, E_Argument_Error | E_Wrong_Type, "Expected string for argument");
		}
	}
}

// World::ExecutableAttribInfo newEnter_wrapper(CallExprAST *call) {

// }

#endif // !_WRAPPER_H_
