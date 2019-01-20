#ifndef _TEMPLATES_H_
#define _TEMPLATES_H_
#include "attribute.h"
#include "whocares.h" // for ASSERT()
#include "world.h"

//TODO: make it contains Attributes instead inheriting
//TODO: Now I think it is design flaw, eliminate it or find a better place for it to work
class Templates: public Attributes{
public:
	//TODO: Sample implementation and naming is a total disaster
	// fix it !!!!!!!
	class Sample {
	public:
		//TODO: This should never be used by user. try fix it
		Sample() : attribute_(nullptr){} // provide a default ctor otherwise it dosen't work for map

		Sample(Attribute * _attribute, std::string _trace, std::string name, Templates *manufactor)
			:attribute_(_attribute), trace_(_trace), name_(name), manufacture_(manufactor), install_exe(false) {}
		Sample(Attribute * _attribute, std::string _trace, std::string name, Templates *manufactor, World::ExecutorType exe, std::string _event)
			:attribute_(_attribute), trace_(_trace), name_(name), manufacture_(manufactor), install_exe(true),exe_(exe), event_(_event) {}
		Sample duplicateAndRedirect(Attribute *attrib) {
			Sample res;
			res.trace_ = trace_;
			res.name_ = name_;
			res.categories = categories;
			res.exe_ = exe_;
			res.event_ = event_;
			res.install_exe = install_exe;
			res.manufacture_ = manufacture_;
			res.attribute_ = attrib;
			return res;
		}
		static Sample newPlaceholder(std::string name, Sample *parent) {
			Sample sample;
			sample.attribute_ = nullptr;
			sample.name_ = name;
			sample.parent_ = parent;
			return std::move(sample);
		}

		Sample duplicate() {
			Sample sample = duplicateAndRedirect(attribute_->duplicate());
			duplicateImplement(sample, *this);
			return sample;
		}
		static void duplicateImplement(Sample& output, Sample& origin) {
			output.child_nodes_.clear();
			if (output.attribute_ == nullptr) {
				// This should not happen if called by `Sample duplicate()`
			}
			Attributes *attribs = output.attribute_->getAsAttributes();
			if (attribs == nullptr) return;
			for (auto child : origin.child_nodes_) {
				output += child.second.duplicateAndRedirect(attribs->lookup(child.first));
				duplicateImplement(output.lookup(child.first), origin.lookup(child.first));
			}
		}

		bool isEmpty() { return attribute_ == nullptr; }
		bool castFrom(Sample&& rhs) {
			ASSERT(isEmpty(), ERROR_UNEXPECTED_NULL_PTR(), "Sample::castFrom");
			name_ = rhs.name_;
			attribute_   = std::move(rhs.attribute_);
			categories   = std::move(rhs.categories);
			child_nodes_ = std::move(rhs.child_nodes_);
			exe_         = std::move(rhs.exe_);
			event_ = rhs.event_;
			install_exe = rhs.install_exe;
			manufacture_ = rhs.manufacture_;
			if (parent_ != nullptr) {
				manufacture_->appendAttribute(&parent_->attribute_, name_, attribute_);
			}
			return true;
		}
		std::string& getName() { return name_; }
		Attribute *attribute_;
		std::string trace_;
		std::string name_;
		std::vector<std::string> categories;
		std::map<std::string, Sample> child_nodes_;
		Templates *manufacture_;
		bool install_exe;
		World::ExecutorType exe_;
        std::string event_;
		Sample *parent_;
		Sample& addCategory(std::string category) {
			categories.push_back(category);
			return *this;
		}
		Sample& operator+=(Sample child_node) {
			if (child_node.isEmpty()) {
				child_nodes_[child_node.name_] = child_node;
				child_nodes_[child_node.name_].parent_ = this;
				return *this;
			}
			Attribute* result = manufacture_->appendAttribute(&attribute_, child_node.name_, child_node.attribute_);
			if (result == nullptr) {
				//TODO: logError
				return *this;
			}
			if (trace_.size() > 0)
				child_node.trace_ = trace_ + "." + name_;
			else
				child_node.trace_ = name_;
			child_nodes_[child_node.name_] = child_node;
			child_nodes_[child_node.name_].parent_ = this;
			return *this;
		}
		Sample& operator,(Sample child_node) {
			return operator+=(child_node);
		}
		Sample& lookup(std::string path, std::string delims = ".") {
			if (path.size() == 0) return *this;
			int pos = 0, pivot;
			pivot = path.find(delims);
			auto child = child_nodes_.find(path.substr(pos, pivot));
			if (static_cast<size_t>(pivot) > path.size())
				return child->second;
			return child->second.lookup(path.substr(pivot), delims);
		}
		Sample* checkForChild(std::string path, std::string delims = ".") {
			if (path.size() == 0) return this;
			int pos = 0, pivot;
			pivot = path.find(delims);
			auto child = child_nodes_.find(path.substr(pos, pivot));
			if (child == child_nodes_.end()) return nullptr;
			if (static_cast<size_t>(pivot) > path.size())
				return &child->second;
			return child->second.checkForChild(path.substr(pivot), delims);
		}
	};

	// _where must be attributes or placeholder
	Attribute *appendAttribute(Attribute **_where, std::string attribute_name, Attribute *attribute) {
		ASSERT(_where != nullptr, ERROR_UNEXPECTED_NULL_PTR(), "Templates");
		Attributes *attributes = (*_where)->getAsAttributes();
		if (attributes != nullptr)
			return attributes->addNReturn(attribute_name, attribute);

		PlaceholderAttribute *placeholder = (*_where)->getAsPlaceholder();
		if (placeholder != nullptr) {
			attributes = upgradePlaceholder(placeholder)->getAsAttributes();
			*_where = attributes;
			return attributes->addNReturn(attribute_name, attribute);
		}

		return nullptr;
	}

	Attributes *newTemplate(std::string template_name) {
		return addNReturn(template_name, new Attributes())->getAsAttributes();
	}
	Attribute *newAstrayTemplate(std::string template_name) {
		return addNReturn(template_name, new AstrayAttribute());
	}
	void setSample(std::string name, Sample sample) {
		samples_[name] = sample;
	}
	Sample& lookupSample(std::string name) {
		return samples_.find(name)->second;
	}

	bool checkSample(std::string name) {
		if (samples_.find(name) == samples_.end())
			return false;
		return true;
	}

private:
	std::map<std::string, Sample> samples_;

};

#endif // !_TEMPLATES_H_

#if 0

// dig in - dig in one layer from [_where]
// _where is reference to pointer!!!
Attribute *digin(Attribute **_where, std::string attribute_name) {
	ASSERT(_where != nullptr, ERROR_UNEXPECTED_NULL_PTR(), "Templates");

	Attributes *attributes = (*_where)->getAsAttributes();
	if (attributes != nullptr)
		return attributes->findOrCreateNULL(attribute_name);

	PlaceholderAttribute *placeholder = (*_where)->getAsPlaceholder();
	if (placeholder != nullptr)
		return upgradePlaceholder(placeholder);

	return nullptr;
}


Attribute *findAndDuplicate(std::string template_name) {
	Attribute *attribute = lookup(template_name);
	if (attribute == nullptr) return nullptr;
	return attribute->duplicate();
	}

#endif
