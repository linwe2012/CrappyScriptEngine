#include "worldgen.h"
#include "compile-debugger.h"
#include "wrapper.h"
using namespace CompileDebugger;

std::string WorldGen::argument_tail_ = "__ARG__";
//TODO: Whole Implement is fd up, rewrite
void WorldGen::compile(std::stringstream& ss, fs::path current_dir, std::string file_name) {
	BlockExprAST* block = ParseALL(ss, current_dir, file_name);
	initLogging();
	if (block != nullptr)
		block->accept(this);
	endLogging();
}

void WorldGen::visitNumberExprAST(NumberExprAST* node) {
	current_->castFrom(Factory::newNumber(context_, node->val()));
	refreshEnvironment();
}

void WorldGen::visitStringExprAST(StringExprAST* node) {
	current_->castFrom(Factory::newString(context_, node->str()));
	refreshEnvironment();
}

void WorldGen::setVarUnknow(VariableExprAST *node) {
	std::vector<std::string> temp = path_;
	if (global_vars_.find(node->name()) != global_vars_.end()) {
		relocates_[node->name()] = std::move(temp);
	}
	else {
		auto itr = undetermined_.find(node->name());
		if (itr != undetermined_.end())
			itr->second.push_back(std::move(temp));
		else {
			std::vector< std::vector<std::string> > temp_vec;
			temp_vec.push_back(std::move(temp));
			undetermined_[node->name()] = std::move(temp_vec);
		}
	}
}
void WorldGen::bindVar(Attributes *dst, Attributes *src, std::vector<std::string>& path_of_dst, std::string name_of_src,  ExprAST* node) {
	TODO_IGNORE(name_of_src);
	static std::string here = "WorldGen::bindVar";
	Attribute *next;
	int cnt = 0;
	int last = path_of_dst.size() - 1;
	for (auto path : path_of_dst) {
		if (dst == nullptr || cnt == last) break;
		if (!cnt++) continue;
		next = dst->lookup(path);
		if (next == nullptr) next = dst->addNReturn(path, new Attributes());
		else if (next->getAsAttributes() == nullptr) {
			logError(node, here, E_Variable_Error | E_Unable_To_Match);
		}
		dst = next->getAsAttributes();
		if (dst == nullptr || cnt == last) break;
	}
	if (dst)
		dst->add(path_of_dst[last], src);
}

void WorldGen::visitLabelExprAST(LabelExprAST* node) {
	static std::string here = "WorldGen::visitLabelExprAST";
	if (current_ == nullptr) {
		logError(node, here, E_Misplaced | E_Label_Error);
		refreshEnvironment();
		return;
	}
	auto check = current_->checkForChild(node->label_name());
	auto last = current_;
	node->label()->accept(this);
	enterScope(node->label()->name());
	if (auto var = node->target()->getAsVariableExprAST()) {
		setVarUnknow(var);
	}
	else {
		//TDOD: Add Label supporting Template & Error
		if (check == nullptr) {
			(*current_) += VarProxy::newPlaceholder(context_, last);
			current_ = current_->checkForChild(context_);
		}
		else
			current_ = check;
		node->target()->accept(this);
	}
	leaveScope();
	refreshEnvironment();
}

void WorldGen::visitVariableExprAST(VariableExprAST* node) {
	static std::string here = "WorldGen::visitVariableExprAST";
	static std::regex reg("([^\\$]+)");
	std::string name = node->name();
	std::string templ = node->template_name();
	if (node->isTemplDecl()) {
		if (Factory::checkForTemplate(templ)) {
			logWarning(node, here, E_Template_Error | E_Declaration | E_Duplicated | S_Ignore);
		}
		if (templ[0] == '$') {
			templ = templ.substr(1);
			auto samp = Factory::newAstrayTemplate(templ);
			samp.addCategory(templ);
			current_->castFrom(std::move(samp));
		}
		else {
			auto samp = Factory::newAttributes(templ);
			samp.addCategory(templ);
			current_->castFrom(std::move(samp));
		}
		
		
	}
	else if (node->hasValidName()) {
		if (!node->hasValidTemplate() && node->hasTags()) {
			current_->castFrom(Factory::newAttributes(context_));
		}
		else if (node->hasValidTemplate()) {
			if (Factory::checkForTemplate(templ)) {
				current_->castFrom(Factory::getIsolatedTemplate(templ));
			}
			else {
				logError(node, "WorldGen::visitVariableExprAST", E_Template_Error | E_Not_Found | S_Ignore);
			}
		}
	}
	else
		logError(node, here, E_Variable_Error | E_Declaration | E_Invalid);
}
void  WorldGen::executeEqual(BinaryExprAST* node, std::string& _where) {
	std::string here = _where + "::executeEqual";
	VariableExprAST* var = node->lhs()->getAsVariableExprAST();
	if (!var) {
		logError(node, here, Expected_Variable_Identifier | E_Declaration);
		return;
	}
	VarProxy result = VarProxy::newPlaceholder(var->name(), nullptr);
	current_ = &result;
	var->accept(this);
	enterScope(var->name());
	node->rhs()->accept(this);
	leaveScope();
	if (var->hasValidName() && !result.isEmpty()) {
		Attributes *res = Factory::installTemplate(result, world_, var->name())->getAsAttributes();
		//TODO: Check Template NULL
		global_vars_[var->name()] = res;
		auto undetermine = undetermined_.find(var->name());
		if (undetermine != undetermined_.end()) {
			for (auto single_undetermined : undetermine->second) {
				Attributes*dst = global_vars_[single_undetermined[0]];
				bindVar(dst, res, single_undetermined, var->name(), node);
			}
			undetermined_.erase(undetermine);
		}
		for (auto pos : relocates_) {
			Attributes *src = global_vars_[pos.first];
			bindVar(res, src, pos.second, pos.first, node);
		}
		relocates_.clear();
	}
	if (var->isTemplDecl()) {
		Factory::saveTemplate(result);
	}
	refreshEnvironment();
}
void WorldGen::visitBinaryExprAST(BinaryExprAST* node) {
	static std::string here = "WorldGen::visitBinaryExprAST";
	switch (node->op())
	{
	case Token::tok_assign:
		executeEqual(node, here); break;
	default:
		break;
	}
}

void WorldGen::visitBlockExprAST(BlockExprAST* node) {
	if (current_ != nullptr && current_->isEmpty()) {
		if (auto check = current_->checkForChild(context_)) {
			//TODO: Add security check -- [current_] may not be attributes
			current_ = check;
		}
		else
			current_->castFrom(Factory::newAttributes(context_));
	}
	auto last = current_;
	for (auto statement : node->exprs()) {
		statement->accept(this);
		// rewind pointer as [current_] always points to block's scope
		// STL won't move [*last] as it is not modified
		current_ = last;
	}
	refreshEnvironment();
}

void WorldGen::visitCommaExprAST(CommaExprAST* node) {
	if(current_->isEmpty())
		current_->castFrom(Factory::newAttributes(context_));
	for (auto sub : node->args()) {
		if (auto var = sub->getAsVariableExprAST()) {
			enterScope(var->name());
			setVarUnknow(var);
			leaveScope();
		}
	}
}

void WorldGen::generateIfElse(std::string& invoker, FunctionsLibrary::Sequence& sequence, IfExprAST *node) {
	std::string lhs;
	std::string rhs;
	int op = 0;
	int dummy = 0;
	ExprAST *cond = node->cond();
	if (auto bin = cond->getAsBinaryExprAST()) {
		lhs = AccessiableExprAST2String(sequence, bin->lhs());
		rhs = AccessiableExprAST2String(sequence, bin->rhs());
		op = bin->op();
	}
	else
		lhs = AccessiableExprAST2String(sequence, cond);
	auto IF = FunctionsLibrary::newSequence();
	auto ELSE = FunctionsLibrary::newSequence();
	if (auto block = node->then()->getAsBlockExprAST()) {
		for (auto statement : (*block))
			genenrateFunction(dummy, invoker, IF, statement);
	}
	if (node->otherwise() != nullptr) {
		if (auto block = node->otherwise()->getAsBlockExprAST()) {
			for (auto statement : (*block))
				genenrateFunction(dummy, invoker, ELSE, statement);
		}
		else if (auto nested = node->otherwise()->getAsIfExprAST()) {
			generateIfElse(invoker, ELSE, nested);
		}
	}
	sequence << FunctionsLibrary::newControlFlow(lhs, op, rhs, FunctionsLibrary::newSequenceExe(IF), FunctionsLibrary::newSequenceExe(ELSE));
}

std::string  WorldGen::AccessiableExprAST2String(FunctionsLibrary::Sequence& sequence, ExprAST *node) {
	if (auto str = node->getAsStringExprAST()) return str->str();
	if (auto var = node->getAsVariableExprAST()) return var->name();
	if (auto acc = node->getAsAccessExprAST()) {
		std::string res = AccessiableExprAST2String(sequence, acc->args()[0]);
		int cnt = 0;
		for (auto itr : (*acc)) {
			if (!cnt++) continue;
			res += "." + AccessiableExprAST2String(sequence, itr);
		}
		return std::move(res);
	}
	if (auto call = node->getAsCallExprAST()) {
		int last_id_ = argument_id_;
		std::string dummmy;
		std::string right;

		int cnt = 0;
		genenrateFunction(cnt, dummmy, sequence, call);
		return std::to_string(last_id_) + argument_tail_;
	}
	return std::move(std::string(""));
}
void WorldGen::genenrateFunction(int& activate_depth, std::string& invoker, FunctionsLibrary::Sequence& sequence, ExprAST *node) {
	static std::vector<std::string> bind_name;
	static std::string here = "WorldGen::genenrateFunction";
	typedef void (*FUNC_RT_Wrapper) (FunctionsLibrary::Sequence&, CallExprAST *, WorldGen&);
	// run time function wrappers
	static std::map<std::string, FUNC_RT_Wrapper> rtfuncs = {
		{"$print", newPrinter_wrapper},
		{"$activate", newActivator_wrapper},
		{"$call", newCallExecuable_wrapper},
		{"$setflag", newSetFlag_wrapper},
		{"$fire", newFireEvent_wrapper},
	};

	if (auto call = node->getAsCallExprAST()) {
		if (call->callee() == "$invoker")
		{
			for (auto arg : call->args()) {
				//TODO: Add var support
				if (auto str = arg->getAsStringExprAST())
					invoker = str->getAsStringExprAST()->str();
				else if (auto num = arg->getAsNumberExprAST())
					activate_depth = num->val();
			}
		}
		else if (call->callee() == "$random") {
			if (call->args_size() != 1)
				return logError(node, here, E_Argument_Error, "Expected 1 argument for $random");
			auto str = call->args()[0]->getAsStringExprAST();
			if (str == nullptr) 
				logError(node, here, E_Argument_Error, "Expected string as argument for $random");
			if(bind_name.size() == 0)
				sequence << FunctionsLibrary::newRandomAttributes(str->str(), std::to_string(argument_id_) + argument_tail_);
			else
				sequence << FunctionsLibrary::newRandomAttributes(str->str(), bind_name.back());
			argument_id_++;
		}
		else {
			auto func = rtfuncs.find(call->callee());
			if (func != rtfuncs.end()) {
				func->second(sequence, call, *this);
			}

		}
	}
	else if (auto ifelse = node->getAsIfExprAST())
		generateIfElse(invoker, sequence, ifelse);
	else if (auto bin = node->getAsBinaryExprAST()) {
		std::string lhs = AccessiableExprAST2String(sequence, bin->lhs());
		std::string rhs;
		if (auto call = bin->rhs()->getAsCallExprAST()) {
			if (call->callee() == "$read") {
				sequence << FunctionsLibrary::newWorldScanner(lhs);
				return;
			}
			if (bin->op() == Token::tok_assign) {
				// int last_id_ = argument_id_;
				bind_name.push_back(lhs);
				genenrateFunction(activate_depth, invoker, sequence, call);
				bind_name.pop_back();
			}
		}
		else {
			
			rhs = AccessiableExprAST2String(sequence, bin->rhs());

			sequence << FunctionsLibrary::newAttributeBinOp(lhs, bin->op(), rhs);
		}
	}

}
void WorldGen::visitLambdaExprAST(LambdaExprAST* node) {
	std::string invoker;
	int activate_depth = 3;
	auto sequence = FunctionsLibrary::newSequence();
	argument_id_ = 0;
	for (auto statement : node->body()->exprs()) {
		genenrateFunction(activate_depth, invoker, sequence, statement);
	}
	current_->castFrom(Factory::newExecutable(context_, FunctionsLibrary::newSequenceExe(sequence), invoker, activate_depth));
}


void WorldGen::visitCallExprAST(CallExprAST* node) {
	static std::string here = "WorldGen::visitCallExprAST";
	if (current_ != nullptr) {
		// this is not happening
	}
	if (node->callee() == "$activate") {
		if (node->args().size() < 2) {
			logError(node, here, E_Argument_Error | E_Too_Few_Arguments, "Expected 2 arguments for $active");
			return;
		}
		auto _1st = node->args()[0]->getAsStringExprAST();
		auto _2nd = node->args()[1]->getAsStringExprAST();

		if (_1st && _2nd) {
			Attribute *attirb = world_->lookupPath(_2nd->str(), ".");
			if (attirb == nullptr) return;
			world_->lazyActivate(_1st->str(), attirb->getAsAttributes());
			world_->refresh();
		}

	}
	else if (node->callee() == "$setinvoker") {
		std::string _event;
		if (node->args().size() < 2) {
			logError(node, here, E_Argument_Error | E_Too_Few_Arguments);
			return;
		}
		if (auto str = node->args()[0]->getAsStringExprAST()) {
			_event = str->str();
		}
		else {
			logError(node, here, E_Argument_Error | E_Wrong_Type);
			return;
		}
		int cnt = 0;
		for (auto arg : node->args()) {
			if (!cnt++) continue;
			if (auto str = arg->getAsStringExprAST())
				world_->addcmd2event(str->str(), _event);
			else if (auto reg = arg->getAsRegexExprAST()) {
				world_->addRegexCmd2event(std::move(reg->std_regex()), _event);
			}
			else
				logError(node, here, E_Argument_Error | E_Invalid);
		}
	}
	else if(node->callee() == "$srand")
	{
		srand(time(nullptr));
	}
}


void WorldGen::visitIfExprAST(IfExprAST* node) {
	TODO_IGNORE(node);
	
}

void WorldGen::visitRegexExprAST(RegexExprAST* node) {
	TODO_IGNORE(node);
}

void WorldGen::visitAccessExprAST(AccessExprAST* node) {
	TODO_IGNORE(node);
}

void WorldGen::clear() {
	current_ = nullptr;
	context_ = "";
}





// Code Dumps -- Codes may come to use someday or never

//bool WorldGen::checkAndInstallTemplateToVar() {
//	static std::string here = "WorldGen::checkAndInstallTemplateToVar";
//	if (var_build_required == nullptr) return false;
//	VarProxy* result = nullptr;
//	VariableExprAST* node = var_build_required;
//	if (node->hasValidTemplate() && !Factory::checkForTemplate(node->template_name())) {
//		logError(node, here, E_Variable_Template_Error | E_Declaration | S_Ignore);
//	}
//	else if(node->hasValidTemplate()) {
//		temp_vars.push_back(Factory::findTemplate(node->template_name()));
//		result = &temp_vars.back();
//	}
//	for (auto tag : node->tags()) {
//		if (!Factory::checkForTemplate(tag)) {
//			logError(node, here, E_Variable_Template_Error | E_Declaration | S_Ignore);
//		}
//		else {
//			if (result == false) {
//				temp_vars.push_back(Factory::newAttributes(context_));
//				result = &temp_vars.back();
//			}
//			(*result) += Factory::findTemplate(tag);
//		}	
//	}
//	refreshEnvironment();
//	return result;
//}