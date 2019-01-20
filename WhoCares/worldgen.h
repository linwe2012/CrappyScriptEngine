#ifndef _WORLD_GENENRATE_H_
#define _WORLD_GENENRATE_H_
#include "parser-ast.h"
#include "factory.h"
#include "builtins.h"
#include "world.h"
class WorldGen : public Visitor {
public:
	typedef Factory::Sample             VarProxy;
	typedef World::ExecutableAttribInfo FuncProxy;
#define DECL_VISITOR(type) void visit##type(type* node) override;
	AST_LIST(DECL_VISITOR)
#undef DECL_VISITOR
		WorldGen(World *world) :world_(world) { clear(); }
	void compile(std::stringstream& ss, fs::path current_dir, std::string file_name);
	void clear();
private:
	void executeEqual(BinaryExprAST* node, std::string& _where);
	void enterScope(std::string context) {
		context_ = context;
		path_.push_back(context);
	}
	void leaveScope() {
		path_.pop_back();
	}
	std::string tellCurrentPath() {
		std::string path;
		for (auto &str : context_)
			path += "." + str;
		return path;
	}
	std::string AccessiableExprAST2String(FunctionsLibrary::Sequence& sequence, ExprAST *node);
	void generateIfElse(std::string& invoker, FunctionsLibrary::Sequence& sequence, IfExprAST *node);
	void genenrateFunction(int& activate_depth, std::string& invoker, FunctionsLibrary::Sequence& sequence, ExprAST *node);
	void refreshEnvironment() { /*var_build_required = nullptr;*/ }
	bool isItGlobalScope() { return !path_.size(); }
	void bindVar(Attributes *dst, Attributes *src, std::vector<std::string>& path_of_dst, std::string name_of_src, ExprAST* node);
	void setVarUnknow(VariableExprAST *node);
	std::map<std::string, VarProxy> global_templates_;
	std::map<std::string, FuncProxy> global_funcs_;
	std::map<std::string, Attributes*> global_vars_;

	World *world_;
	std::string context_;
	std::vector<std::string> path_;
	VarProxy* current_;
	std::vector<VarProxy> temp_vars;
	std::map < std::string, std::vector<std::vector<std::string> > > undetermined_;
	std::map < std::string, std::vector<std::string> > relocates_;
	int argument_id_;
	static std::string argument_tail_;
};


#endif // _WORLD_GENENRATE_H_
