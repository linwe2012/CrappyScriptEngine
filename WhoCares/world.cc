#include "world.h"

std::map<std::string, int, World::case_insensitive_strcmp> World::flags_name_ = {
#define LIST_FLAGS(f) {#f, k##f},
	WORLD_COMMAND_LIST(LIST_FLAGS)
#undef LIST_FLAGS
};



void World::setCategory(std::string category, std::string _attrib_name, Attribute *_attribute) {
	auto exists = everything_->lookup(category);
	if (exists == nullptr) {
		addCategories(category);
		exists = everything_->lookup(category);
	}
	exists->getAsAttributes()->addProtectParent(_attrib_name, _attribute);
}

Attributes* World::generateArguments(ExecutableAttribute *exeAttrib) {
	Attributes *attributes = new Attributes();
	auto arguments = exeAttrib->arguments();
	attributes->addProtectParent("$this", exeAttrib->getParent());
	for (auto arg_name = arguments.begin(); arg_name != arguments.end(); arg_name++)
		attributes->addProtectParent(*arg_name, parseArgument(exeAttrib, *arg_name));
	return attributes;
}


Attribute* World::parseArgument(ExecutableAttribute *exeAttrib, std::string& arg) {
	Attributes *where_to_search = exeAttrib->getParent();
	Attribute *result = where_to_search->lookupPath(arg, ".");
	if (result == nullptr)
		result = lookupPath(arg, ".");
	return result;
}


void World::parseCmd() {
	std::string cmd;
	cmd_buffer_.clear();
	cmd_buffer_ >> cmd;
	auto _event_itr = cmd2event_.find(cmd);
	if (_event_itr != cmd2event_.end()) {
		fireEvent(_event_itr->second);
	}
	else {
		std::smatch result;
		RegexEventType *res = nullptr;
		for (auto &itr : regex_cmd2event_) {
			std::regex_search(cmd, result, itr.first);
			if (!result.size()) continue;
			res = &itr;
			break;
		}
		if (res == nullptr) return;
		fireEvent(res->second);
	}
}


void World::addRegexCmd2event(std::regex&& reg, std::string _event) {
	regex_cmd2event_.push_back(RegexEventType(
		std::move(reg),
		std::move(_event)
	));
}


std::string World::getNextCmd() {
	std::string res;
	cmd_buffer_ >> res;
	if (res.size() == 0) {
		std::cin >> res;
	}
	return res;
}

int World::setRelation(std::string _parent, std::string _child, std::string _relation_name, std::string path_delims) {
	Attribute *parentNode = lookupPath(_parent, path_delims);
	Attribute *childNode = lookupPath(_child, path_delims);
	if (parentNode == nullptr) return -1;
	Attributes *parent = parentNode->getAsAttributes();
	if (parent == nullptr) return -1;
	parent->add(_relation_name, childNode);
	return 0;
}

//TODO: event $onload & $onleave leave others to do
void World::activate(std::string _name, Attribute *_To) {
	ASSERT(_To != nullptr, ERROR_UNEXPECTED_NULL_PTR(), "World::activate");
	auto active_ = actives_->lookup(_name);
	if (active_ != nullptr) {
		fireEvent("$onleave");
		actives_set_.erase(active_);
	}
	actives_->add(_name, _To);
	actives_set_.insert(_To);
	// if (active_ != nullptr)
	fireEvent("$onload");
}

void World::deactivate(std::string _name) {
	auto active_ = actives_->lookup(_name);
	if (active_ == nullptr) return;
	actives_set_.erase(active_);
	actives_->add(_name, nullptr); //delete it
}

int World::setFlag(std::string command) {
	auto itr = flags_name_.find(command);
	if (itr == flags_name_.end())
		return -1;
	flags_.set(itr->second);
	return 0;
}