#ifndef _LISTENERS_H_
#define _LISTENERS_H_
#include "attribute.h"
#include <vector>
#if 0
#include <algorithm> // std::set_intersection()
#include <iterator> // std::inserter()
class WordList{
public:
	typedef std::set<std::string> data_type;
	typedef data_type::iterator iterator;
	WordList() {}

	int count(std::string _element) {
		return data_.count(_element);
	}

	void insert(std::string _element) {
		data_.insert(_element);
	}
	WordList& operator,(std::string rhs) {
		insert(rhs);
	}
	iterator begin() { return data_.begin(); }
	iterator end() { return data_.end(); }
	bool operator==(WordList& rhs) {
		data_type intersect;
		std::set_intersection(begin(), end(), rhs.begin(), rhs.end(),
			std::inserter(intersect, intersect.begin()));
		if (intersect.size() == 0) return false;
		return true;
	}

private:
	data_type data_;
};
#endif

class Listeners {
public:
	//TODO: Check whether std::set is better
	typedef std::vector<Attribute *> Interestee;
	typedef std::map<std::string, Interestee> data_type;
	typedef data_type::iterator iterator;
	typedef std::function<void(std::string&, ExecutableAttribute *)> EventDealerType;

	int addListener(std::string _event, Attribute * _attribute) {
		if (!_attribute->getAsExecutable()) return -1;
		auto itr = find(_event);
		if (itr == end()) {
			data_.insert(std::pair<std::string, Interestee>(_event, Interestee()));
			itr = find(_event);
		}
		itr->second.push_back(_attribute);
		return 0;
	}
	
	Listeners& operator+=(std::string _event) {
		auto itr = find(_event);
		if (itr == end())
			data_.insert(std::pair<std::string, Interestee>(_event, Interestee()));
		return *this;
	}

	Listeners& operator,(std::string _event) { return operator+=(_event); }
	
	iterator find(std::string& _event) { return data_.find(_event); }
	iterator begin() { return data_.begin(); }
	iterator end() { return data_.end(); }

	void fireEvent(std::string _event, UNUSED AstrayAttribute *_room, EventDealerType _event_dealer) {
		IGNORE(_room);
		auto itr = find(_event);
		if (itr != end()) {
			Interestee& interestees = itr->second;
			ExecutableAttribute *exe = nullptr;
			auto interestee = interestees.begin();
			for(;interestee != interestees.end(); interestee++)
			{
				// we guranteed it is an executable when we insert it.
				exe = (*interestee)->getAsExecutable();
				_event_dealer(_event, exe);
			}
		}
	}
private:
	data_type data_;
};
#endif // !_LITENERS_H_
