#ifndef _ATTRIBUTE_H_
#define _ATTRIBUTE_H_
#include <cmath>
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <bitset>
#include "macro-utils.h"

class PlaceholderAttribute;
class StringAttribute;
class NumberAttribute;
class ExecutableAttribute;
class Attributes;
class AstrayAttribute;

//Inheritance hierarchy (Attribute postfix omitted for readabilty sake.)
// Attribute
//   - Placeholder
//   - Executable
//   - template<T> Base
//      - Base<std::string> String
//      - Base<double>      Number
//      - Base<std::map>    Attributes
//          - Astray
//          - Templates               //TODO: design flaw?


class Attribute
{
public:
	enum {
		kFALSE,
		kTRUE,
		kINCOMPARABLE, 
		kIS_LARGER,
		kIS_EQUAL,
		kIS_SAMLLER,
		kIS_NOT_EUQAL,
	};
	virtual StringAttribute * getAsString() { return nullptr; }
	virtual NumberAttribute * getAsNumber() { return nullptr; }
	virtual ExecutableAttribute * getAsExecutable() { return nullptr; }
	virtual Attributes * getAsAttributes() { return nullptr; }
	virtual PlaceholderAttribute * getAsPlaceholder() { return nullptr; }
	virtual AstrayAttribute *getAsAstray() { return nullptr; } //TODO: Room or Room Attribute?
	virtual Attribute *duplicate() = 0; //de facto copy ctor

	virtual void setParent(Attributes *attributes) { parent_ = attributes; }
	virtual Attributes* getParent() { return parent_; }
	virtual int compare(Attribute *rhs) = 0;
	virtual int istrue() = 0;
	virtual int assign_add(Attribute *rhs, std::string& name) = 0;
	virtual int assign(Attribute *rhs) = 0;
	virtual ~Attribute() {}
protected:
	Attributes *parent_;
};

struct NULLPTR
{
	void* operator() () const {
		return nullptr;
	}
};


class PlaceholderAttribute : public Attribute {
public:
	PlaceholderAttribute(Attributes *parent, std::string name) : name_(name) { parent_ = parent; }
	PlaceholderAttribute * getAsPlaceholder() override { return this; }
	Attribute *duplicate() override {
		return new PlaceholderAttribute(parent_, name_);
	}
	int compare(UNUSED Attribute *rhs) override { IGNORE(rhs); return kINCOMPARABLE; }
	int istrue() override { return kFALSE; }
	virtual int assign_add(UNUSED Attribute *rhs, UNUSED std::string& name) override { IGNORE(rhs); IGNORE(name); return -1; }
	int assign(UNUSED Attribute *rhs) { IGNORE(rhs); return -1; }
	std::string name_;
};


class ExecutableAttribute :public Attribute {
public:
	typedef std::function<void(AstrayAttribute *, Attributes *)> ExecutorType;

	ExecutableAttribute() { setParent(nullptr); }
	ExecutableAttribute(std::vector<std::string>& arguments, ExecutorType executor, int activate_depth)
		:arguments_(arguments), executor_(executor), activate_depth_(activate_depth){ setParent(nullptr); }
	ExecutableAttribute *getAsExecutable() override { return this; }
	Attribute *duplicate() {
		return new ExecutableAttribute(arguments_, executor_, activate_depth_);
	}
	std::vector<std::string>& arguments() { return arguments_; }
	void execute(AstrayAttribute *room, Attributes *attribute) { 
		executor_(room, attribute); 
	}
	void setExecutor(ExecutorType executor) {
		executor_ = executor;
	}
	int compare(UNUSED Attribute *rhs) override { IGNORE(rhs); return kINCOMPARABLE; }
	int istrue() override { return kFALSE; }
	int assign_add(UNUSED Attribute *rhs, UNUSED std::string& name) { IGNORE(rhs); IGNORE(name);return -1; }
	int assign(UNUSED Attribute *rhs) { IGNORE(rhs); return -1; }
	int activate_depth() { return activate_depth_; }
protected:
	// use string so that we can dynamically bind arguments
	std::vector<std::string> arguments_;
	ExecutorType executor_;
	int activate_depth_;
};

// base attrib is an abstraction of how attribute handles data
// since we may use `void *data` or just `int data`
// They are just STL or POD wrapper
template<class T, class ClassName>
class BaseAttribute : public Attribute {
public:
	typedef T data_type;
	typedef ClassName self_type;
	// it is better use apis to retrieve data, since I haven't decide
	// if data is a void * or just a type itself
	T data() { return *ptrData(); }
	T data() const { return *ptrData(); }

	static ClassName* cast(Attribute *another) {
		auto placeholder = another->getAsPlaceholder();
		if (placeholder == nullptr)
			return nullptr;
		return new ClassName();
	}
	void setData(T data) { refData() = data; }
	virtual Attribute *duplicate() override {
		return new ClassName(data_);
	}
protected:
	T& refData() { return *ptrData(); }
	T* ptrData() { return &(data_); }
	
	const T* ptrData() const { return &(data_); }
	// void deleteData() override {}
	// virtual void allocate() override { data_ = new T(); }
private:
	T data_;
};


class NumberAttribute : public BaseAttribute<double, NumberAttribute> {
public:
	NumberAttribute() { setParent(nullptr); }
	NumberAttribute(data_type data) { refData() = data; setParent(nullptr);}
	
	NumberAttribute *getAsNumber() override { return this; }
	NumberAttribute& operator+=(const NumberAttribute & number) {
		refData() += number.data();
		return *this;
	}
	int compare(Attribute *rhs) override { 
		if (rhs == this) return kIS_EQUAL;
		if (rhs == nullptr || rhs->getAsNumber() == nullptr)
			return kINCOMPARABLE;
		int res = refData() - rhs->getAsNumber()->refData();
		if (res < 0)
			return kIS_SAMLLER;
		else if (res == 0)
			return kIS_EQUAL;
		return kIS_LARGER;
	}
	int istrue() override {
		if (refData()) return kTRUE; 
		return kFALSE;
	}
	virtual int assign_add(Attribute *rhs, UNUSED std::string& name) override {
		IGNORE(name);
		auto str = rhs->getAsNumber();
		if (str == nullptr)
			return -1;
		refData() += str->refData();
		return 0;
	}
	virtual int assign_sub(Attribute *rhs) {
		auto num = rhs->getAsNumber();
		if (num == nullptr) return -1;
		refData() -= num->refData();
		return 0;
	}
	virtual int assign_div(Attribute *rhs) {
		auto num = rhs->getAsNumber();
		if (num == nullptr) return -1;
		refData() /= num->refData();
		return 0;
	}
	virtual int assign_mul(Attribute *rhs) {
		auto num = rhs->getAsNumber();
		if (num == nullptr) return -1;
		refData() *= num->refData();
		return 0;
	}
	virtual int assign(Attribute *rhs) override {
		auto str = rhs->getAsNumber();
		if (str == nullptr)
			return -1;
		refData() = str->refData();
		return 0;
	}
};


class StringAttribute : public BaseAttribute<std::string, StringAttribute > {
public:
	StringAttribute() { setParent(nullptr); }
	StringAttribute(data_type data) { refData() = data; setParent(nullptr); }
	StringAttribute *getAsString() override { return this; }
	virtual int compare(Attribute *rhs) override {
		if (rhs == nullptr || rhs->getAsString() == nullptr)
			return kINCOMPARABLE;
		if (rhs->getAsString()->refData() == refData()) {
			return kIS_EQUAL;
		}
		return kIS_NOT_EUQAL;
	}
	virtual int istrue() override { return kTRUE; }
	virtual int assign_add(Attribute *rhs, UNUSED std::string& name) override {
		IGNORE(name);
		auto str = rhs->getAsString();
		if (str == nullptr)
			return -1;
		refData() += str->refData();
		return 0;
	}
	virtual int assign(Attribute *rhs) override {
		auto str = rhs->getAsString();
		if (str == nullptr)
			return -1;
		refData() = str->refData();
		return 0;
	}
};


class Attributes : public BaseAttribute<std::map<std::string, Attribute *>, Attributes > {
public:

	Attributes() { setParent(nullptr);  }
	//TODO: Make it const
	Attributes(data_type& data){
		duplicateData(data);
		setParent(nullptr);
	}

	virtual Attribute *duplicate() override {
		return new Attributes(refData());
	}
	~Attributes() {
		auto itr = begin();
		for (; itr != end(); itr++)
			if(itr->second->getParent() == this)
				delete itr->second;
	}
	virtual Attributes * getAsAttributes() override { return this; }

	Attribute *lookup(std::string attribute_name) {
		if (attribute_name == "$size") {
			return addNReturn("$size", new NumberAttribute(size()));
		}
		if (attribute_name.size() == 0) return this;
		if (attribute_name == "$parent")
			return getParent();
		auto itr = refData().find(attribute_name);
		if (itr == refData().end())
			return nullptr;
		return itr->second;
	}

	Attribute *remove(Attribute *_attrib) {
		for (auto itr : refData()) {
			if (itr.second == _attrib) {
				if (itr.second->getParent() == this) {
					delete itr.second;
					refData().erase(itr.first);
					return nullptr;
				}
				return _attrib;
			}
		}
		return _attrib;
	}
	//TODO: Tail Recursion, make it loop
	Attribute *lookupPath(std::string attribute_path, std::string delims) {
		auto temp = refData();
		if (attribute_path.size() == 0) return this;
		int pos = 0, pivot;
		pivot = attribute_path.find(delims);
		Attribute *attribute = lookup(attribute_path.substr(pos, pivot));
		if (attribute == nullptr) return nullptr;
		if (static_cast<size_t>(pivot) > attribute_path.size())
			return attribute;
		Attributes *attributes = attribute->getAsAttributes();
		if (attributes == nullptr) return nullptr;
		return attributes->lookupPath(attribute_path.substr(++pivot), delims);
	}

	void add(std::string attribute_name, Attribute *attribute) {
		addNReturn(attribute_name, attribute);
	}
	// findOrCreateNULL - gurantees a return
	// if not found, create a new one an insert
	Attribute *findOrCreateNULL(std::string attribute_name) {
		Attribute *attribute = lookup(attribute_name);
		if (attribute != nullptr) return attribute;
		return addNReturn(attribute_name, new PlaceholderAttribute(this, attribute_name));
	}

	static Attribute *upgradePlaceholder(Attribute *placeholder) {
		Attributes *attribs = new Attributes();
		Attribute *res = replacePlaceholder(placeholder, attribs);
		if (res == nullptr) delete attribs;
		return res;
	}

	//TODO: use addNReturn API
	static Attribute *replacePlaceholder(Attribute *placeholder, Attribute *replacement) {
		PlaceholderAttribute *ph = placeholder->getAsPlaceholder();
		Attributes *self = ph->getParent();
		if (ph == nullptr)
			return placeholder; //this is not a placeholder
		auto itr = self->refData().find(ph->name_);
		if (itr == self->refData().end())
			return nullptr; // we can't find that
		delete itr->second;
		itr->second = replacement;
		replacement->setParent(self);
		return itr->second;
	}

	typedef data_type::iterator iterator;
	iterator begin() { return refData().begin(); }
	iterator end() { return refData().end(); }
	size_t erase(std::string name) { return refData().erase(name); }
	size_t size() { return refData().size(); }
	// add n' return just added
	Attribute *addNReturn(std::string attribute_name, Attribute *attribute) {
		auto itr = refData().find(attribute_name);
		if (itr != refData().end()) {
			if(itr->second->getParent() == this && attribute != itr->second)
				delete itr->second;
			if (attribute == nullptr)
				erase(attribute_name);
		}
		if (attribute != nullptr) {
			refData()[attribute_name] = attribute;
			attribute->setParent(this);
		}
		return attribute;
	}

	Attribute *addNReturnProtectParent(std::string attribute_name, Attribute *attribute) {
		if (attribute == nullptr) return nullptr;
		auto itr = refData().find(attribute_name);
		if (itr != refData().end() && itr->second->getParent() == this && itr->second != attribute)
			delete itr->second;
		refData()[attribute_name] = attribute;
		return attribute;
	}

	void addProtectParent(std::string attribute_name, Attribute *attribute) {
		addNReturnProtectParent(attribute_name, attribute);
	}
	virtual int compare(Attribute *rhs) override {
		if (rhs == this) return kIS_EQUAL;
		return kINCOMPARABLE;
	}
	virtual int istrue() override { return kTRUE; }
	virtual int assign_add(Attribute *rhs, std::string& name) override {
		add(name, rhs);
		return 0;
	}
	virtual int assign(UNUSED Attribute *rhs) override {
		IGNORE(rhs);
		return -1;
	}
protected:
	void duplicateData(data_type& data) {
		auto itr = data.begin();
		for (; itr != data.end(); itr++) {
			// must use add() API as it can relocate `parent_`
			add(itr->first, itr->second->duplicate());
		}
	}
};

class AstrayAttribute : public Attributes {
public:
	AstrayAttribute() {}
	AstrayAttribute(data_type& data) {
		duplicateData(data);
	}
	AstrayAttribute *getAsAstray() override { return this; }
	Attributes *getParent() override { return nullptr; }
	Attribute *duplicate() override {
		return new AstrayAttribute(refData());
	}

	Attribute *checkAttribute(Attribute *attribute) {
		for (auto attrib : refData())
			if (attrib.second == attribute)
				return attribute;
		return nullptr;
	}
};

#endif // !_ATTRIBUTE_H_
