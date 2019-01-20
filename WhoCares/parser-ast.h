#ifndef _PARSER_AST_H_
#define _PARSER_AST_H_
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <experimental/filesystem>
typedef void Code; //TODO: Generate Actual code
namespace fs = std::experimental::filesystem;

#define K_INPUT std::stringstream& ss
#define K_INPUT_PASSON ss

#define AST_LIST(V)     \
	V(NumberExprAST)    \
	V(StringExprAST)    \
    V(VariableExprAST)  \
	V(BinaryExprAST)    \
	V(CallExprAST)      \
	V(BlockExprAST)     \
	V(LabelExprAST)     \
	V(LambdaExprAST)    \
	V(CommaExprAST)     \
	V(AccessExprAST)    \
	V(RegexExprAST)     \
	V(IfExprAST)        \

#define DEF_AST(AST) class AST;
AST_LIST(DEF_AST)
#undef DEF_AST

//TODO: Add debug info to ExprAST
class DebugInfo {
public:
	typedef std::shared_ptr<std::string> StrPtr;
private:
	StrPtr raw_;
	size_t pos;
	StrPtr file_name_;
	int line_;
	int column_;
public:
	DebugInfo() {
		column_ = 1;
		line_ = 1;
	}
	void inc_line() { 
		pos = raw_->find('\n', pos);
		++line_; column_ = 1; 
	};
	void init(StrPtr&& file_name, StrPtr&& raw) {
		reset();
		file_name_ = std::move(file_name);
		raw_ = std::move(raw);
	}
	int line() { return line_; }
	void reset() {
		pos = 0;
		column_ = 1;
		line_ = 1;
	}
	const std::string& filename() const { return *file_name_; }
};
//TODO: distinguish between lvalue & rvalue
class Visitor;
//TODO: delete ExprAST in the function name. kinda verbose
class ExprAST {
public:
	virtual ~ExprAST() {}
	
	virtual NumberExprAST *getAsNumberExprAST() { return nullptr; }
	virtual StringExprAST *getAsStringExprAST() { return nullptr; }
	virtual VariableExprAST *getAsVariableExprAST() { return nullptr; }
	virtual BinaryExprAST *getAsBinaryExprAST() { return nullptr; }
	virtual CallExprAST *getAsCallExprAST()     { return nullptr; }
	virtual BlockExprAST *getAsBlockExprAST()   { return nullptr; }
	virtual LabelExprAST *getAsLabelExprAST()   { return nullptr; }
	virtual LambdaExprAST *getAsLambdaExprAST() { return nullptr; }
	virtual CommaExprAST *getAsCommaExprAST()   { return nullptr; }
	virtual AccessExprAST *getAsAccessExprAST() { return nullptr; }
	virtual RegexExprAST *getAsRegexExprAST()   { return nullptr; }
	virtual IfExprAST    *getAsIfExprAST()      { return nullptr; }

	virtual void accept(Visitor *visitor) = 0;
	ExprAST(DebugInfo& dbg) :dbg_(dbg) {}
	const DebugInfo& dbg() const { return dbg_; }
private:
	DebugInfo dbg_;
};

class NumberExprAST : public ExprAST {
	double val_;
public:
	NumberExprAST(DebugInfo& dbg, double val) :ExprAST(dbg), val_(val) {}
	virtual NumberExprAST *getAsNumberExprAST() override { return this; }
	double val() { return val_; }
	virtual void accept(Visitor *visitor);
};

class StringExprAST : public ExprAST {
	std::string str_;
public:
	StringExprAST(DebugInfo& dbg, std::string str) :ExprAST(dbg), str_(str) {}
	virtual StringExprAST *getAsStringExprAST() override { return this; }
	std::string str() { return str_; }
	virtual void accept(Visitor *visitor);
};

class RegexExprAST : public ExprAST {
	std::string raw_regex_;
public:
	RegexExprAST(DebugInfo& dbg, std::string raw_regex) :ExprAST(dbg), raw_regex_(raw_regex) {}
	virtual RegexExprAST *getAsRegexExprAST() override { return this; }
	virtual void accept(Visitor *visitor);
	std::string raw_regex() { return raw_regex_; }
	// it is said that optimize will take 3x time to compile regex, but 20% down when searching
	// since in world there is heavy searching job to do, we really want to improve match speed
	std::regex std_regex() { return std::move(std::regex(raw_regex_, std::regex::optimize)); }
};

/// idnetifiers: abc
class VariableExprAST : public ExprAST {
	std::string name_;
	std::string template_;
	std::vector<std::string> tags_;
public:
	VariableExprAST(DebugInfo& dbg, std::string name, std::string template_name, std::vector<std::string> tags) :ExprAST(dbg), name_(name), template_(template_name), tags_(tags) {}
	virtual VariableExprAST *getAsVariableExprAST() override { return this; }
	std::string name() { return name_; }
	std::string template_name() { return template_; }
	std::vector<std::string> tags() { return tags_; }
	bool hasValidName() { return name_.size(); }
	bool hasValidTemplate() { return template_.size(); }
	bool hasTags() { return tags_.size(); }
	bool isVarDecl() { return hasValidName(); }
	bool isTemplDecl() { return !hasValidName() && hasValidTemplate(); }
	virtual void accept(Visitor *visitor);
};

class BinaryExprAST : public ExprAST {
	char op_;
	ExprAST *lhs_, *rhs_;
public:
	BinaryExprAST(DebugInfo& dbg, char op, ExprAST *lhs, ExprAST *rhs)
		:ExprAST(dbg), op_(op), lhs_(lhs), rhs_(rhs) {}
	virtual BinaryExprAST *getAsBinaryExprAST() override { return this; }
	char op() { return op_; }
	ExprAST *lhs() { return lhs_; }
	ExprAST *rhs() { return rhs_; }
	virtual void accept(Visitor *visitor) override;
};


class BlockExprAST : public ExprAST {
	std::vector<ExprAST *> exprs_;
public:
	typedef std::vector<ExprAST *>::iterator iterator;
	BlockExprAST(DebugInfo& dbg, const std::vector<ExprAST *>& exprs)
		:ExprAST(dbg), exprs_(exprs) {}
	virtual BlockExprAST *getAsBlockExprAST() override { return this; }
	std::vector<ExprAST *>& exprs() { return exprs_; }
	iterator begin() { return exprs_.begin(); }
	iterator end() { return exprs_.end(); }
	virtual void accept(Visitor *visitor) override;
};


class LabelExprAST : public ExprAST {
	VariableExprAST* label_;
	ExprAST* target_;
public:
	virtual LabelExprAST *getAsLabelExprAST() override { return this; }
	LabelExprAST(DebugInfo& dbg, VariableExprAST* _label, ExprAST *target) :ExprAST(dbg), label_(_label), target_(target){}
	std::string label_name() { return label_->name(); }
	VariableExprAST* label() { return label_; }
	ExprAST* target() { return target_; }
	virtual void accept(Visitor *visitor) override;
};

class LambdaExprAST : public ExprAST {
	BlockExprAST *body_;
	std::vector<ExprAST *> args_;
public:
	virtual LambdaExprAST *getAsLambdaExprAST() override { return this; }
	LambdaExprAST(DebugInfo& dbg, BlockExprAST *body, std::vector<ExprAST *>& args) :ExprAST(dbg), body_(body), args_(args) {}
	BlockExprAST *body() { return body_; }
	std::vector<ExprAST *>& args() { return args_; }
	int args_size() { return args_.size(); }
	virtual void accept(Visitor *visitor) override;
};

class CommaExprAST : public ExprAST {
	std::vector<ExprAST *> exprs_;
public:
	virtual CommaExprAST *getAsCommaExprAST() override { return this; }
	CommaExprAST(DebugInfo& dbg, std::vector<ExprAST *>& exprs) :ExprAST(dbg), exprs_(exprs) {}
	std::vector<ExprAST *>& args() { return exprs_; }
	virtual void accept(Visitor *visitor) override;
};

class AccessExprAST : public ExprAST{
	typedef std::vector<ExprAST *>::iterator iterator;
	std::vector<ExprAST *> exprs_;
public:
	virtual AccessExprAST *getAsAccessExprAST() override { return this; }
	AccessExprAST(DebugInfo& dbg, std::vector<ExprAST *>& exprs) :ExprAST(dbg), exprs_(exprs) {}
	std::vector<ExprAST *>& args() { return exprs_; }
	iterator begin() { return exprs_.begin(); }
	iterator end() { return exprs_.end(); }
	virtual void accept(Visitor *visitor) override;
};

class IfExprAST : public ExprAST {
	ExprAST *cond_;
	ExprAST *then_;
	ExprAST *otherwise_;
public:
	virtual IfExprAST *getAsIfExprAST() override { return this; }
	IfExprAST(DebugInfo& dbg, ExprAST* cond, ExprAST* then, ExprAST* otherwise) :ExprAST(dbg), cond_(cond), then_(then), otherwise_(otherwise) {}
	ExprAST *cond() { return cond_; }
	ExprAST *then() { return then_; }
	ExprAST *otherwise() { return otherwise_; }
	virtual void accept(Visitor *visitor) override;
};

class CallExprAST :public ExprAST {
	std::string callee_;
	std::vector<ExprAST *> args_;
public:
	CallExprAST(DebugInfo& dbg, const std::string& callee,
		std::vector<ExprAST *>& args)
		:ExprAST(dbg), callee_(callee), args_(args) {}
	virtual CallExprAST *getAsCallExprAST() override { return this; }
	std::string callee() { return callee_; }
	std::vector<ExprAST *> args() { return args_; }
	int args_size() { return args_.size(); }
	virtual void accept(Visitor *visitor) override;
};

BlockExprAST* ParseALL(K_INPUT, fs::path current_dir, std::string current_file);


//TODO: obsolete or useless currently

class FunctionProtoAST{
	std::string name_;
	std::vector<std::string> args_;
	ExprAST *body_;
public:
	FunctionProtoAST(const std::string name, 
		const std::vector<std::string>& args,
		ExprAST *body)
		:name_(name), args_(args), body_(body){}
};
/*
class StatemenExprAST : public ExprAST {
	ExprAST * statement_;
public:
	StatemenExprAST(ExprAST * statement) : statement_(statement) {}
	virtual void accept(Visitor *visitor);
};
*/

// abstract class for traverse AST
class Visitor {
public:
#define DEF_VISIT(type) virtual void visit##type(type *node) = 0;
	AST_LIST(DEF_VISIT)
#undef DEF_VISIT
};

#endif // !_PARSER_AST_H_
