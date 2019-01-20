#include "builtins.h"

// Compute the sum of attributes
//TODO: Add es template support
void FunctionsLibrary::AttribSum(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string dst_name, std::string src_name) {
	Attribute *attributes = TrySearchAmong(_world, _room, arguments, src_name);
	Attribute *destination = TrySearchAmong(_world, _room, arguments, dst_name);
	Attributes *attribs = attributes->getAsAttributes();
	NumberAttribute *number = destination->getAsNumber();
	if (attribs == nullptr || number == nullptr) return;

	NumberAttribute *addee = nullptr;
	number->setData(0);
	//auto itr = attribs->begin();
	for (auto itr : (*attribs)) {
		addee = itr.second->getAsNumber();
		if (addee != nullptr)
			*number += *addee;
	}
}

void FunctionsLibrary::WorldScanner(World *_world, UNUSED AstrayAttribute *_room, Attributes *arguments, std::string outputName) {
	IGNORE(_room);
	std::string str = _world->getNextCmd();
	while (str.size() == 0)
	{
		str = _world->getNextCmd();
	}
	arguments->add(outputName, new StringAttribute(str));
}

void FunctionsLibrary::Activator(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string activate_type,
		std::string dst_name, std::string failure_callback, std::string success_callback) {
	std::stringstream ss;
	__ESTemplateString2Normal(_world, _room, arguments, dst_name, ss);
	Attributes *dstNode = TrySearchAttributesInverse(_world, _room, arguments, ss.str());
	ExecutableAttribute *failureNode = TrySearchExecutable(_world, _room, arguments, failure_callback);
	ExecutableAttribute *successNode = TrySearchExecutable(_world, _room, arguments, success_callback);
	// ss.str("");
	// __ESTemplateString2Normal(_world, _room, arguments, activate_type, ss);
	if (dstNode == nullptr) {
		if (failureNode != nullptr)
			failureNode->execute(_room, arguments);
		return;
	}
	_world->lazyActivate(activate_type, dstNode);
	if (successNode != nullptr) {
		successNode->execute(_room, arguments);
	}
}

void FunctionsLibrary::DeActivator(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string activate_type,
		std::string failure_callback, std::string success_callback) {
	TODO_IGNORE(failure_callback);
	// ExecutableAttribute *failureNode = TrySearchExecutable(_world, _room, arguments, failure_callback);
	ExecutableAttribute *successNode = TrySearchExecutable(_world, _room, arguments, success_callback);
	_world->deactivate(activate_type);
	if (successNode != nullptr) {
		successNode->execute(_room, arguments);
	}
}

void FunctionsLibrary::AttributeBinOp(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string lhs, int op, std::string rhs) {
		std::stringstream ss;
	std::string name;
	__ESTemplateString2Normal(_world, _room, arguments, lhs, ss);
	lhs = ss.str();
	ss.str("");
	__ESTemplateString2Normal(_world, _room, arguments, rhs, ss);
	rhs = ss.str();
	Attribute *_lhs_ = nullptr;
	if (op == Token::tok_assign_update) {
		auto pos = lhs.find_last_of('.');
		
		if (pos < lhs.size()) {
			_lhs_ = TrySearchAmong(_world, _room, arguments, lhs.substr(0, pos));
			name = lhs.substr(++pos);
		}
		else {
			_lhs_ = TrySearchAmong(_world, _room, arguments, lhs);
		}
	}
	else
		_lhs_ = TrySearchAmong(_world, _room, arguments, lhs);

	Attribute *_rhs_ = TrySearchAmong(_world, _room, arguments, rhs);
	if (_rhs_ == nullptr && rhs != "null" && op != Token::tok_assign_update) return;
	if (_lhs_ == nullptr && op != Token::tok_assign_update) {
		auto pos = lhs.find_last_of('.');
		if (pos < lhs.size()) {
			_lhs_ = TrySearchAmong(_world, _room, arguments, lhs.substr(0, pos));
			if (_lhs_ != nullptr) {
				auto _attribs_ = _lhs_->getAsAttributes();
				if (_attribs_) {
					name = lhs.substr(++pos);
					_lhs_ = _attribs_->addNReturn(name, _rhs_->duplicate());
				}
			}

		}
	}
	if (_lhs_ == nullptr) return;
	Attributes *attribs = nullptr;
	Attributes *rparent = nullptr;
	switch (op)
	{
	case Token::tok_assign:
		_lhs_->assign(_rhs_);
		break;
	case Token::tok_assign_add:
		_lhs_->assign_add(_rhs_, name);
		break;
	case Token::tok_assign_update:
		if(_rhs_ != nullptr)
			rparent = _rhs_->getParent(); // we need to record it's parent
		attribs = _lhs_->getAsAttributes();
		if (rparent == _lhs_)
			break;
		if (attribs == nullptr)
			break;
		_lhs_->assign_add(_rhs_, name);
		if (rparent)
			rparent->remove(_rhs_);
		break;
	case Token::tok_assign_sub:
		if (_rhs_ == nullptr || _lhs_ == nullptr || _lhs_->getAsNumber() == nullptr)
			break;
		_lhs_->getAsNumber()->assign_sub(_rhs_);
	default:
		break;
	}
}

void FunctionsLibrary::FindAttributeAndAddToArguments(World *_world, AstrayAttribute *_room, Attributes *arguments,
	std::string searchee, std::string dst_name, std::string failure_callback, std::string success_callback) {
	TODO_IGNORE(failure_callback);
	TODO_IGNORE(success_callback);
	std::stringstream ss;
	__ESTemplateString2Normal(_world, _room, arguments, searchee, ss);
	Attribute *result = TrySearchAmong(_world, _room, arguments, ss.str());
	arguments->addProtectParent(dst_name, result);

	//TODO: Adding fail& success call back
}

void FunctionsLibrary::SelectRandomFromAttributes(World *_world, AstrayAttribute *_room, Attributes *arguments,
	std::string search_within, std::string arg_name) {
	std::stringstream ss;
	__ESTemplateString2Normal(_world, _room, arguments, search_within, ss);
	search_within = ss.str();
	Attributes *result = TrySearchAttributes(_world, _room, arguments, search_within);
	if (result == nullptr) return;
	if (result->size() == 0) return;
	auto which_one = rand() % result->size();
	Attribute *ret = nullptr;
	int cnt = 0;
	for (auto itr : (*result)) {
		if (cnt++ != static_cast<int>(which_one)) continue;
		ret = itr.second;
		break;
	}
	arguments->addProtectParent(arg_name, ret);
}
// e.g.`test${abc.efg}test\n` -> left = 4, right = 10(starting from left)
//TODO: Add nest support
void FunctionsLibrary::loacteLeftAndRight(int *left, int *right, const std::string& str, std::string& left_delim, std::string& right_delim, int start) {
	*left = str.find(left_delim, start);
	*right = str.find(right_delim, *left);
}
void FunctionsLibrary::ESTemplateStringSeperator(int *left, int *right, const std::string& str, int start) {
	static std::string left_delim = "${";
	static std::string right_delim = "}";
	loacteLeftAndRight(left, right, str, left_delim, right_delim, start);
}

void FunctionsLibrary::__ESTemplateString2Normal(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string str, std::ostream& output) {
	int pos = 0;
	int left, right;
	while (1) {
		ESTemplateStringSeperator(&left, &right, str, pos);
		if (left < 0 || right < 0) {
			output << str.substr(pos);
			break;
		}
		output << str.substr(pos, left - pos);
		left += 2; // eat "${"
		StringAttribute *variable = TrySearchString(_world, _room, arguments, str.substr(left, right - left));
		if (variable != nullptr)
			output << variable->data();
		pos = right + 1; // eat '}'
	}
}
void FunctionsLibrary::__PrintStringFImplement(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string str) {
	int pos = 0;
	int left, right;
	std::string object_format;
	while (1) {
		ESTemplateStringSeperator(&left, &right, str, pos);
		if (left < 0 || right < 0) {
			Console::out << str.substr(pos);
			break;
		}
		Console::out << str.substr(pos, left - pos);
		left += 2; // eat "${"
		Attribute *variable = TrySearchAmong(_world, _room, arguments, str.substr(left, right - left)); // str.substr(left, right - left), ".");
		if (variable && variable->getAsAttributes()) {
			if (str[right + 1] == '[') {
				int new_right = right;
				new_right = str.find(']', right);
				if (new_right > 0) {
					left = right + 1;
					right = new_right;
					object_format = str.substr(left + 1, right - left - 1);
				}
				else
					object_format = "";
			}
		}
		PrinterHelper(_world, _room, arguments, variable, 0, object_format);
		if (right < 0)
			break;
		pos = right + 1; // eat '}'
	}
}

void FunctionsLibrary::PrintStringF(World *_world, AstrayAttribute *_room, Attributes *arguments, StringAttribute *fmt) {
	__PrintStringFImplement(_world, _room, arguments, fmt->data());
}

void FunctionsLibrary::PrintNumber(UNUSED World *_world, UNUSED AstrayAttribute *_room, UNUSED Attributes *arguments, NumberAttribute *number) {
	IGNORE(_world);
	IGNORE(_room);
	IGNORE(arguments);
	Console::out << number->data();
}
/*
	static void PrintObject(World *_world, AstrayAttribute *_room, Attributes *arguments, Attributes *object, int tabs) {
		int max_width = 20;
		++tabs;
		for (auto itr : (*object)) {
			auto name = itr.first;
			Console::PrintF("%s:%c", name.c_str(), name.size() > max_width ? '\n':' ');
			// PrinterHelper(_world, _room, arguments, itr->second, tabs);
		}
	}
*/
void FunctionsLibrary::FormatPrintObject(World *_world, AstrayAttribute *_room, Attributes *arguments, Attributes *object, std::string format) {
	static std::string left_delim = ">";
	static std::string right_delim = "<";
	if (format.empty()) return;
	if (object->size() == 0) return;
	int left, right;
	std::string inner;
	loacteLeftAndRight(&left, &right, format, left_delim, right_delim, 0);
	if (left >= 0 && right > 0) {
		inner = format.substr(left + 1, right - left - 1);
		__PrintStringFImplement(_world, _room, arguments, format.substr(0, left));
	}
	else
		inner = format;
	int ending_cnt = inner.find('>');
	std::string ending;
	if (ending_cnt > 0) {
		ending = inner.substr(ending_cnt + 1);
		inner = inner.substr(0, ending_cnt);
	}
	int cnt = object->size();
	for (auto itr : (*object)) {
		--cnt;
		arguments->add("$name$", new StringAttribute(itr.first));
		arguments->addProtectParent("$node$", itr.second);
		__PrintStringFImplement(_world, _room, arguments, inner);
		if (cnt) __PrintStringFImplement(_world, _room, arguments, ending);
	}

	if (right > 0)
		__PrintStringFImplement(_world, _room, arguments, format.substr(right + 1));
}

void FunctionsLibrary::PrinterHelper(World *_world, AstrayAttribute *_room, Attributes *arguments, Attribute *printee, int tabs, std::string &extra_fmt) {
	for (int i = 0; i < tabs; i++)
		Console::out << "\t";
	if (printee == nullptr) Console::out << "( null )";
	else if (printee->getAsString()) PrintStringF(_world, _room, arguments, printee->getAsString());
	else if (printee->getAsNumber()) PrintNumber(_world, _room, arguments, printee->getAsNumber());
	else if (printee->getAsAttributes()) FormatPrintObject(_world, _room, arguments, printee->getAsAttributes(), extra_fmt);
}
/*
	// seems it becomes useless, Ooops...
	static void Printer(World *_world, AstrayAttribute *_room, Attributes *arguments, std::vector<std::string>printee_name) {
		static std::string str = "";
		for (auto itr = printee_name.begin(); itr != printee_name.end(); itr++) {
			auto printee = arguments->lookupPath(*itr, ".");
			PrinterHelper(_world, _room, arguments, printee, 0, str);
		}
	}
*/
void FunctionsLibrary::RandomString(UNUSED World *_world, UNUSED AstrayAttribute *_room, Attributes *arguments, std::string name_of_chosen_one, std::vector<std::string>string_set) {
	IGNORE(_world);
	IGNORE(_room);
	int select = rand() % string_set.size();
	arguments->add(name_of_chosen_one, new StringAttribute(string_set[select]));
}

void FunctionsLibrary::CallExecutable(World *_world, AstrayAttribute *_room, Attributes *arguments, std::string callee, std::string fail_call, std::string success_call) {
	std::stringstream ss;
	__ESTemplateString2Normal(_world, _room, arguments, callee, ss);
	callee = ss.str();
	auto exe = TrySearchExecutable(_world, _room, arguments, callee);
	if (exe != nullptr) {
		// push this
		Attribute *last_this = arguments->lookup("$this");
		arguments->addProtectParent("$this", exe->getParent());
		exe->execute(_room, arguments);
		auto success = TrySearchExecutable(_world, _room, arguments, success_call);
		if (success != nullptr) success->execute(_room, arguments);
		// pop this
		arguments->addProtectParent("$this", last_this);
	}
	else {
		auto fail = TrySearchExecutable(_world, _room, arguments, fail_call);
		if (fail != nullptr) fail->execute(_room, arguments);
	}
}

void FunctionsLibrary::ControlFlow(World *_world, AstrayAttribute *_room, Attributes *arguments,
	std::string lhs, int op, std::string rhs, World::ExecutorType _then, World::ExecutorType _else) {
	std::stringstream ss;
	__ESTemplateString2Normal(_world, _room, arguments, lhs, ss);
	lhs = ss.str();
	Attribute *_lhs_ = TrySearchAmong(_world, _room, arguments, lhs);
	int exe_flag = -1;
	int control_flow = 0;
	if (_lhs_ == nullptr)
		exe_flag = Attribute::kFALSE;
	if (!Token::isCompare(op) && _lhs_) {
		exe_flag = _lhs_->istrue();
		control_flow = (exe_flag == Attribute::kTRUE);
	}
	else {
		ss.str("");
		__ESTemplateString2Normal(_world, _room, arguments, rhs, ss);
		rhs = ss.str();
		Attribute *_rhs_ = TrySearchAmong(_world, _room, arguments, rhs);
		if (_rhs_ == nullptr)
			exe_flag = Attribute::kINCOMPARABLE;
		else if (_lhs_)
			exe_flag = _lhs_->compare(_rhs_);
	}
	switch (op)
	{
	case Token::tok_not_equal: // fall through
	case Token::tok_equal:
		if (Attribute::kIS_EQUAL == exe_flag)
			control_flow = 1;
		if (op == Token::tok_not_equal)
			control_flow = !control_flow;
		break;
	case Token::tok_lower_than: // fall through
	case Token::tok_greater_or_euqal:
		if (Attribute::kIS_LARGER == exe_flag || Attribute::kIS_LARGER == exe_flag)
			control_flow = 1;
		if (op == Token::tok_lower_than)
			control_flow = !control_flow;
		break;
	case Token::tok_lower_or_euqal: // fall through
	case Token::tok_greater_than:
		if (Attribute::kIS_LARGER == exe_flag)
			control_flow = 1;
		if (op == Token::tok_lower_than)
			control_flow = !control_flow;
		break;
	default:
		break;
	}
	if (control_flow)
		_then(_world, _room, arguments);
	else
		_else(_world, _room, arguments);
}

void FunctionsLibrary::SetFlag(World *_world, UNUSED AstrayAttribute *_room, UNUSED Attributes *arguments,
		std::string content) {
	IGNORE(_room);
	IGNORE(arguments);
	_world->setFlag(content);
}