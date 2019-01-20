#ifndef _FACTORY_H_
#define _FACTORY_H_
#include "world.h"
#include "templates.h"


class Factory {
public:
	
	typedef Templates::Sample Sample;
	
	// Factory(Templates *templates): templates_(templates), delims_("."){}
	static void bindTemplates(Templates *templates) {
		templates_ = templates;
	}
	static Sample& newTemplate(std::string sample_name) {
		Attribute *attribute = templates_->newTemplate(sample_name);
		templates_->setSample(sample_name,
			Sample(attribute, "", sample_name, templates_)
		);
		return findTemplate(sample_name);
	}

	static Sample& newAstrayTemplate(std::string sample_name) {
		Attribute *attribute = templates_->newAstrayTemplate(sample_name);
		templates_->setSample(sample_name,
			Sample(attribute, "", sample_name, templates_)
		);
		return findTemplate(sample_name);
	}

	static void append(Sample& _To, Sample& _From) {
		_To += _From;
	}

	static Sample newAttributes(std::string attributes_name) {
		return Sample(new Attributes(), "", attributes_name, templates_);
	}
	static Sample newNumber(std::string attribute_name, double d) {
		return Sample(new NumberAttribute(d), "", attribute_name, templates_);
	}
	static Sample newString(std::string attribute_name, std::string content) {
		return Sample(new StringAttribute(content), "", attribute_name, templates_);
	}
	// ExecutableAttribInfo: arguments and exe std::function<void(World *, Room *, Attributes *)>
	static Sample newExecutable(std::string attribute_name, World::ExecutableAttribInfo infos, std::string what_event, int activate_depth = 3) {
		return Sample(new ExecutableAttribute(infos.arguments, dummy_function_, activate_depth), "", attribute_name, templates_, infos.exe, what_event);
	}
	static Sample& findTemplate(std::string name) {
		return templates_->lookupSample(name);
	}
	
	static Sample getIsolatedTemplate(std::string name) {
		auto sample = templates_->lookupSample(name);
		return sample.duplicate();
	}

	static bool checkForTemplate(std::string name) {
		return  templates_->checkSample(name);
	}

	static Attribute* installTemplate(std::string name, World * _where, std::string actual_name) {
		return installTemplate(findTemplate(name), _where, actual_name);
	}
	static void saveTemplate(Sample &what) {
		templates_->setSample(what.getName(), what);
	}
	static Attribute* installTemplate(Sample& what, World *_where, std::string actual_name) {
		Attribute *installee = what.attribute_->duplicate();
		std::string last_name = what.name_; // we will protect the orginal name
		what.name_ = actual_name;
		installImplement(installee, what, _where);
		what.name_ = last_name;
		return installee;
	}

	static Sample& duplicateTemplate(std::string name, std::string new_name) {
		auto Base = findTemplate(name);
		Attribute *attrib = Base.attribute_->duplicate();
		Sample sample = Base.duplicateAndRedirect(attrib);
		sample.name_ = new_name;
		duplicateSample(attrib->getAsAttributes(), sample, Base);
		templates_->setSample(new_name, sample);
		return findTemplate(new_name);
	}

private:
	static void duplicateSample(Attributes *current, Sample& dst, Sample& src) {
		for (auto itr : src.child_nodes_) {
			Attribute *child = current->lookup(itr.first);
			dst += itr.second.duplicateAndRedirect(child);
			if (Attributes *next =  child->getAsAttributes())
				duplicateSample(next, dst.lookup(itr.first), src.lookup(itr.first));
		}
	}
	static void installImplement(Attribute *installee, Sample& what, World *_world) {
		auto itr = what.categories.begin();
		for (; itr != what.categories.end(); itr++)
			_world->setCategory(*itr, what.getName(), installee);
		if (what.install_exe) {
			ExecutableAttribute *exe = installee->getAsExecutable();
			_world->addListener(exe, what.exe_, what.event_);
		}
		auto child = what.child_nodes_.begin();
		if (child == what.child_nodes_.end()) return;
		Attributes *attribs = installee->getAsAttributes();
		for (; child != what.child_nodes_.end(); child++)
			installImplement(attribs->lookup(child->first), child->second, _world);
	}
	static Templates *templates_;
	static ExecutableAttribute::ExecutorType dummy_function_;
};

#endif // !_FACTORY_H_
