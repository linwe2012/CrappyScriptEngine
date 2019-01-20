#pragma once
#include "world.h"
#include "checks.h"
#include <stdlib.h>
#include "token.h"



// if the name of a function capitalized, it means it is 
// the implemenation function
// else it is a wrapper to generate std::function<>
class FunctionsLibrary {
public:
	class Sequence {
	public:
		Sequence() {}
		Sequence(World::ExecutableAttribInfo exeinfo) { operator<<(exeinfo); }
		Sequence& operator<<(World::ExecutableAttribInfo exeinfo) {
			exes.push_back(exeinfo.exe);
			arguments.insert(arguments.end(), exeinfo.arguments.begin(), exeinfo.arguments.end());
			return *this;
		}
		std::vector<World::ExecutorType> exes;
		std::vector<std::string> arguments;
	};

	// Compute the sum of attributes
	//TODO: Add es template support
	static void AttribSum(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string dst_name, std::string src_name);

	static void WorldScanner(World *_world, UNUSED AstrayAttribute *_room, Attributes *arguments, std::string outputName);
	
	static void Activator(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string activate_type,
		std::string dst_name, std::string failure_callback, std::string success_callback);

	static void DeActivator(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string activate_type,
		std::string failure_callback, std::string success_callback);

	static void AttributeBinOp(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string lhs, int op, std::string rhs);

	static void FindAttributeAndAddToArguments(World *_world, AstrayAttribute *_room, Attributes *arguments,
		std::string searchee, std::string dst_name, std::string failure_callback, std::string success_callback);

	static void SelectRandomFromAttributes(World *_world, AstrayAttribute *_room, Attributes *arguments,
		std::string search_within, std::string arg_name);

	static void loacteLeftAndRight(int *left, int *right, const std::string& str, std::string& left_delim, std::string& right_delim, int start);
	static void ESTemplateStringSeperator(int *left, int *right, const std::string& str, int start);

	static void __ESTemplateString2Normal(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string str, std::ostream& output);
	static void __PrintStringFImplement(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string str);

	static void PrintStringF(World *_world, AstrayAttribute *_room, Attributes *arguments, StringAttribute *fmt);

	static void PrintNumber(UNUSED World *_world, UNUSED AstrayAttribute *_room, UNUSED Attributes *arguments, NumberAttribute *number);

	static void FormatPrintObject(World *_world, AstrayAttribute *_room, Attributes *arguments, Attributes *object, std::string format);

	static void PrinterHelper(World *_world, AstrayAttribute *_room, Attributes *arguments, Attribute *printee, int tabs, std::string &extra_fmt);

	static void RandomString(UNUSED World *_world, UNUSED AstrayAttribute *_room, Attributes *arguments,
		std::string name_of_chosen_one, std::vector<std::string>string_set);

	static void CallExecutable(World *_world, AstrayAttribute *_room, Attributes *arguments,
		std::string callee, std::string fail_call, std::string success_call);

	static void ControlFlow(World *_world, AstrayAttribute *_room, Attributes *arguments,
		std::string lhs, int op, std::string rhs, World::ExecutorType _then, World::ExecutorType _else);

	static void SetFlag(World *_world, AstrayAttribute *_room, Attributes *arguments,
		std::string content);

	static World::ExecutableAttribInfo newAttribSum(std::string dst_name, std::string src_name) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = { dst_name, src_name };
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::AttribSum, _1, _2, _3, dst_name, src_name),
			arguments 
		);
	}

	static World::ExecutableAttribInfo newPrinter(std::string PrintWhat) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments;
		int pos = 0;
		int left = 0, right;
		while (1) {
			ESTemplateStringSeperator(&left, &right, PrintWhat, pos);
			if (right < 0) break;
			left += 2;
			arguments.push_back(PrintWhat.substr(left, right - left));
			pos = right + 1; // eat '}'
		}
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::__PrintStringFImplement, _1, _2, _3, PrintWhat),
			arguments
		);
	}

	static void SeqenceExecutor(World *_world, AstrayAttribute *_room, Attributes *arguments, std::vector<World::ExecutorType> exes) {
		for (auto &exe : exes)
			exe(_world, _room, arguments);
	}

	static World::ExecutableAttribInfo newActivator(std::string activate_type,  std::string activatee, std::string failure_call, std::string success_call) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = { activatee , failure_call, success_call };
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::Activator, _1, _2, _3, activate_type, activatee, failure_call, success_call),
			arguments
		);
	}

	static  World::ExecutableAttribInfo newWorldScanner(std::string scanName) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = { scanName };
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::WorldScanner, _1, _2, _3, scanName),
			arguments
		);
	}
	static  World::ExecutableAttribInfo newRandomAttributes(std::string search_within, std::string arg_name) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments;
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::SelectRandomFromAttributes, _1, _2, _3, search_within, arg_name),
			arguments
		);
	}
	static Sequence newSequence() {
		return Sequence();
	}
	static World::ExecutableAttribInfo newSequenceExe(Sequence& sequence) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::SeqenceExecutor, _1, _2, _3, sequence.exes),
			sequence.arguments
		);
	}
	
	static World::ExecutableAttribInfo newAttributeFinder(std::string what_to_find, std::string result_name, std::string failure_call, std::string success_call) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments;
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::FindAttributeAndAddToArguments, _1, _2, _3, what_to_find, result_name, failure_call, success_call),
			arguments
		);
	}
	static World::ExecutableAttribInfo newAttributeBinOp(std::string lhs, int op, std::string rhs) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = { lhs,  rhs };
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::AttributeBinOp, _1, _2, _3, lhs, op, rhs),
			arguments
		);
	}

	static World::ExecutableAttribInfo newRandomString(std::string name_of_chosen_one, std::vector<std::string> string_set) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments;
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::RandomString, _1, _2, _3, name_of_chosen_one, string_set),
			arguments
		);
	}
	static World::ExecutableAttribInfo newControlFlow(std::string lhs, int op, std::string rhs, World::ExecutableAttribInfo _then, World::ExecutableAttribInfo _else) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = _then.arguments;
		arguments.insert(arguments.end(), _else.arguments.begin(), _else.arguments.end());
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::ControlFlow, _1, _2, _3, lhs, op, rhs, _then.exe, _else.exe),
			arguments
		);
	}
	static World::ExecutableAttribInfo newCallExecutable(std::string callee, std::string fail_call, std::string success_call) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = {callee, fail_call, success_call};
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::CallExecutable, _1, _2, _3, callee, fail_call, success_call),
			arguments
		);
	}

	static World::ExecutableAttribInfo newSetFlag(std::string flag_name) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments;
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::SetFlag, _1, _2, _3, flag_name),
			arguments
		);
	}

	static World::ExecutableAttribInfo newDeAactivator(std::string activate_type,
		std::string failure_callback, std::string success_callback) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = { failure_callback , success_callback};
		return World::ExecutableAttribInfo(
			std::bind(FunctionsLibrary::DeActivator, _1, _2, _3, activate_type, failure_callback, success_callback),
			arguments
		);
	}
	static World::ExecutableAttribInfo newFireEvent(std::string _event) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		using std::placeholders::_3;
		std::vector<std::string> arguments = { };
		auto f = [&](World *_world, UNUSED AstrayAttribute *_room, UNUSED Attributes *arguments) { IGNORE(_room); IGNORE(arguments); _world->fireEvent(_event); };
		return World::ExecutableAttribInfo(
			std::bind(f, _1, _2, _3),
			arguments
		);
	}


private:
	// from left to right, whatever find first
	static Attribute *TrySearchAmong(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string searchee_name) {
		Attribute *result = arguments->lookup(searchee_name);
		if (result != nullptr) return result;
		result = arguments->lookupPath(searchee_name, ".");
		if (result != nullptr) return result;
		if (auto _this_attrib_ = arguments->lookup("$this")) {
			if (auto _this_ = _this_attrib_->getAsAttributes()) {
				result = _this_->lookup(searchee_name);
				if (result != nullptr) return result;
				result = _this_->lookupPath(searchee_name, ".");
				if (result != nullptr) return result;
			}
		}
		TODO_IGNORE(_room);
		if (result != nullptr) return result;
		// result = _room->lookup(searchee_name);
		// if (result != nullptr) return result;
		// result = _room->lookupPath(searchee_name, ".");
		// if (result != nullptr) return result;
		result = _world->lookupPath(searchee_name, ".");
		return result;
	}
	// from right to left, as in some case it is more likely at left
	static Attribute *TrySearchAmongInverse(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string& searchee_name) {
		Attribute *result = _world->lookupPath(searchee_name, "."); 
		if (result != nullptr) return result;
		result = _room->lookupPath(searchee_name, ".");
		if (result != nullptr) return result;
		result = _room->lookup(searchee_name);
		if (result != nullptr) return result;
		result = arguments->lookupPath(searchee_name, ".");
		if (result != nullptr) return result;
		result = arguments->lookup(searchee_name);
		if (result != nullptr) return result;
		if (auto _this_attrib_ = arguments->lookup("$this")) {
			if (auto _this_ = _this_attrib_->getAsAttributes()) {
				result = _this_->lookup(searchee_name);
				if (result != nullptr) return result;
				result = _this_->lookupPath(searchee_name, ".");
				if (result != nullptr) return result;
			}
		}
		return result;
	}
	static Attributes *TrySearchAttributes(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string searchee_name) {
		Attribute *half_way_done = TrySearchAmong(_world, _room, arguments, searchee_name);
		if (half_way_done == nullptr) return nullptr;
		else return half_way_done->getAsAttributes();
	}
	static ExecutableAttribute *TrySearchExecutable(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string searchee_name) {
		Attribute *half_way_done = TrySearchAmong(_world, _room, arguments, searchee_name);
		if (half_way_done == nullptr) return nullptr;
		else return half_way_done->getAsExecutable();
	}
	static StringAttribute *TrySearchString(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string searchee_name) {
		Attribute *half_way_done = TrySearchAmong(_world, _room, arguments, searchee_name);
		if (half_way_done == nullptr) return nullptr;
		else return half_way_done->getAsString();
	}

	static AstrayAttribute *TrySearchAstrayInverse(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string searchee_name) {
		Attribute *half_way_done = TrySearchAmongInverse(_world, _room, arguments, searchee_name);
		if (half_way_done == nullptr) return nullptr;
		else return half_way_done->getAsAstray();
	}
	static Attributes *TrySearchAttributesInverse(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string searchee_name) {
		Attribute *half_way_done = TrySearchAmongInverse(_world, _room, arguments, searchee_name);
		if (half_way_done == nullptr) return nullptr;
		else return half_way_done->getAsAttributes();
	}
};