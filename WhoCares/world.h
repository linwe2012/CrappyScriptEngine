#ifndef _WORLD_H_
#define _WORLD_H_
#include "listeners.h"
#include "whocares.h"
#include "functional"
#include <sstream>
#include <set>
#include <iostream>
#include <regex>

template<class T>
class FunctionMap {
public:
	FunctionMap() {}
	void insert(std::string function_name, T func) { data_[function_name] = func; }
	T* lookup(std::string function_name) {
		auto itr = data_.find(function_name);
		if (itr == data_.end()) return nullptr;
		return &(itr->second);
	}
private:
	std::map<std::string, T> data_;
};
#define WORLD_COMMAND_LIST(V)   \
	V(Exit)                     \
	V(Terminate)

class World {
private:
	enum FLAG_K
	{
#define ENUM_FLAG_CONSTANTS(f) k##f,
		WORLD_COMMAND_LIST(ENUM_FLAG_CONSTANTS)
#undef ENUM_FLAG_CONSTANTS
	};

public:
	typedef std::function<void(World *, AstrayAttribute *, Attributes *)> ExecutorType;
	typedef std::pair<std::regex, std::string> RegexEventType;
	typedef struct ExecutableAttribInfo
	{
		ExecutorType exe;
		std::vector<std::string> arguments;
		ExecutableAttribInfo() {}
		ExecutableAttribInfo(ExecutorType _exe, std::vector<std::string>& _arguments)
			:exe(std::move(_exe)), arguments(_arguments){}
	}ExecutableAttribInfo;

	World(Listeners *listeners) :listeners_(listeners),
			actives_(new AstrayAttribute()), everything_(new Attributes()), flags_(false){
		installDealer();
	}
	World() : actives_(new AstrayAttribute()),
			everything_(new Attributes()), flags_(false) {
		listeners_ = new Listeners();
		installDealer();
	}
	
	void activate(std::string _name, Attribute *_To);

	void deactivate(std::string _name);

	void lazyActivate(std::string _name, Attributes *_To) {
		ASSERT(_To != nullptr, ERROR_UNEXPECTED_NULL_PTR(), "World::activate");
		lazy_activate_buffer_[_name] = _To;
	}

	void activateAllLazy() {
		auto act = lazy_activate_buffer_.begin();
		for (; act != lazy_activate_buffer_.end();) {
			activate(act->first, act->second);
			lazy_activate_buffer_.erase(act++);
		}
	}

	void refresh() {
		parseCmd();
		activateAllLazy();
		while (!lazy_activate_buffer_.empty()) {
			activateAllLazy();
		}
	}

	void fireEvent(std::string _event) {
		ASSERT(actives_ != nullptr, ERROR_UNEXPECTED_NULL_PTR(), "World::fireEvent");
		listeners_->fireEvent(_event, actives_, dealer_);
	}

	void addListener(ExecutableAttribute *interestee, ExecutorType _exe, std::string _bind_event) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		interestee->setExecutor(std::bind(_exe, this, _1, _2));
		listeners_->addListener(_bind_event, interestee);
	}

	Attributes *generateArguments(ExecutableAttribute *exeAttrib);
	Attribute *parseArgument(ExecutableAttribute *exeAttrib, std::string& arg);
	void setCategory(std::string category, std::string _attrib_name, Attribute *_attribute);

	World& addCategories(std::string category) {
		everything_->add(category, new Attributes());
		return *this;
	}
	

	Attribute* lookupPath(std::string path, std::string delims) {
		auto pos = path.find("$active");
		if (pos == 0) {
			auto next = path.find(delims, 8);
			if (next >= path.size()) return nullptr;
			auto sub = path.substr(8);
			return actives_->lookupPath(sub, delims);
		}
		return everything_->lookupPath(path, delims);
	}

	int setRelation(std::string _parent, std::string _child, std::string _relation_name, std::string path_delims);

	Listeners *getListener() { return listeners_; }

	void setCmdBuffer(std::string _cmd) {
		cmd_buffer_.clear();
		cmd_buffer_ << "\n" << _cmd;
	}

	std::string getNextCmd();
	
	void addcmd2event(std::string _cmd, std::string _event) {
		cmd2event_[_cmd] = _event;
	}

	void addRegexCmd2event(std::regex&& reg, std::string _event);
	void parseCmd();
	bool shouldExit() { return flags_[kExit];  }
	bool shouldTerminate() { return flags_[kTerminate];}
	int setFlag(std::string flag);
	AstrayAttribute *actives() { return actives_; }
private:
	struct case_insensitive_strcmp {
		bool operator()(const std::string& a, const std::string& b) const
		{
			return std::equal(a.begin(), a.end(),
				b.begin(), b.end(),
				[](const char a, const char b) {
				return tolower(a) == tolower(b);
			});
		}
	};
	void eventExecutor(UNUSED std::string& _event, ExecutableAttribute *caller) {
		IGNORE(_event);
		Attribute *parent = caller->getParent();
		int cnt = 0;
		int max_depth = caller->activate_depth();
		while (cnt++, parent && actives_set_.find(parent) == actives_set_.end()) //it is an odinary attribute
			parent = parent->getParent();
		if(max_depth < 0 || (cnt < max_depth && actives_set_.find(parent) != actives_set_.end()))
			caller->execute(actives_, generateArguments(caller));
	}

	void installDealer() {
		using std::placeholders::_1;
		using std::placeholders::_2;
		dealer_ = std::bind(&World::eventExecutor, this, _1, _2);
	}

	Listeners *listeners_;
	AstrayAttribute *actives_;
	std::set<Attribute *> actives_set_;
	Attributes *everything_;
	Listeners::EventDealerType dealer_;
	std::map<std::string, std::string> cmd2event_;
	std::vector<RegexEventType> regex_cmd2event_;
	std::stringstream cmd_buffer_;
	std::map<std::string, Attribute *>lazy_activate_buffer_;
	std::bitset<16> flags_;
	static std::map<std::string, int, case_insensitive_strcmp> flags_name_;
};

#endif