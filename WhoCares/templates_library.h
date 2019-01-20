#ifndef _TEMPLATES_LIBRARY_H_
#define _TEMPLATES_LIBRARY_H_
#include "templates.h"
#include "builtins.h"
#include "factory.h"
/* A built-in templates
* require bultin function to work
* Use Factory's API to build template
*/
class TemplatesLibrary {
public:
	TemplatesLibrary(World *world, Templates *templates_) :world_(world){
		Factory::bindTemplates(templates_);
		buildHitPoint();
		buildAttack();
		buildHero();
		buildRoom();
		buildNPC();
	}
	void installTemplate(std::string template_name, std::string new_name) {
		using F = Factory;
		Attribute *result = F::installTemplate(template_name, world_, new_name);
		Attributes *result_map = result->getAsAttributes();
		if (result_map != nullptr)
			result_map->add("name", new StringAttribute(new_name));
	}
protected:
	//TODO: add description for all builtin templates
	// HitPoint hierarchy:
	// - addon
	//    - basic
	//    - others...
	// - max
	// - min
	// - hp
	// - function sum_hp => {hp = addon.basic + addon.others.. }
	void buildHitPoint() {
		using F = Factory;
		auto Base = F::newTemplate("HitPoint");
		Base += F::newAttributes("addon"), F::newNumber("max", 2000), F::newNumber("min", 0), F::newNumber("hp", 0);
		Base += F::newExecutable("sum_hp", FunctionsLibrary::newAttribSum("hp", "addon"), "$onactive");
		Base += F::newString("description", "This specifies how many hit point a hero or npc can withstand.");
		Base.lookup("addon") += F::newNumber("basic", 1000);
		F::saveTemplate(Base);
	}
	
	void buildAttack() {
		using F = Factory;
		auto Base = F::newTemplate("Attack");
		Base += F::newAttributes("addon");
		Base.lookup("addon") += F::newNumber("basic", 200);
		F::saveTemplate(Base);
	}

	void buildNPC() {
		using F = Factory;
		auto Base = F::duplicateTemplate("Hero", "NPC");
		Base.addCategory("$npc");
		// auto Base = F::newTemplate("NPC");
		// Base += F::findTemplate("HitPoint"), F::findTemplate("Attack");
		// Base.addCategory("$npc");
		F::saveTemplate(Base);
	}

	void buildHero() {
		using F = Factory;
		auto Base = F::newTemplate("Hero");
		Base += F::findTemplate("HitPoint"), F::findTemplate("Attack");
		Base.addCategory("$player");
		F::saveTemplate(Base);
	}
	void buildRoom() {
		using F = Factory;
		using L = FunctionsLibrary;
		std::vector<std::string> greetings_room = {
			"Hey, welcome to ${$this.name}.",
			"U R in ${$this.name}.",
			"Enjoy your stay in ${$this.name}.",
		};

		std::vector<std::string> greetings_exits = {
			"There are ${$this.exits}[exit to the ${$name$}(${$node$.name})>, ]\n",
			"You see ${$this.exits}[exit to the ${$name$}(${$node$.name})>, ]\n",
		};

		std::vector<std::string> greetings_npc = {
			"${$this.npc}[Also, you are accompnied by >${$node$.name}>, <.\n]",
			"${$this.npc}[You can also see >${$node$.name}>, <.\n]",
			"${$this.npc}[>${$node$.name}>, < staring at you. Creepy...\n]",
		};

		auto Base = F::newAstrayTemplate("Room");
		Base += F::newAttributes("exits"), F::newAttributes("npc");
		auto Greeting = L::newSequence();
		Greeting << L::newRandomString("greetings_room_temp", greetings_room)
			<< L::newRandomString("greetings_exits_temp", greetings_exits)
			<< L::newRandomString("greetings_npc_temp", greetings_npc)
			<< L::newPrinter("${greetings_room_temp} ${greetings_exits_temp} ${greetings_npc_temp}");
		Base += F::newExecutable("greeting", L::newSequenceExe(Greeting), "$onload");
		// Base += F::newExecutable("greeting", L::newPrinter("Hey, welcome to ${@name}. There are ${exits}[exits to the ${$name$}(${$node$.@name})>, ]\n"
		// "${npc}[Also, you are accompnied by >${$name$}>, <.\n]"), "$onload");
		auto Enter = F::newExecutable("enter_room_fail", L::newPrinter("Ooops, it seems there is no such room.\n"), "$global_callback");
		Enter.addCategory("$global");
		F::saveTemplate(Enter);
		F::installTemplate("enter_room_fail", world_, "enter_room_fail");
		auto ChangeRoom = L::newSequence();
		ChangeRoom << L::newWorldScanner("input_direc")
			<< L::newActivator("$room", "$this.exits.${input_direc}", "$global.enter_room_fail", "$global.enter_room_success");
		Base += F::newExecutable("change_room", L::newSequenceExe(ChangeRoom), "on_goto");
		Base.addCategory("$room");
		F::saveTemplate(Base);
	}

	int setRoomPath(std::string _To, std::string _From, std::string direction) {
		return world_->setRelation(_To, _From, direction, ".");
	}

	int setRoomBidirectionalPath(std::string _To, std::string _From, std::string direction) {
		return setRoomPath(_To, _From, direction) +
			setRoomPath(_From, _To, direction);
	}
private:
	World *world_;
};

#endif // !_BUILTIN_TEMPLATES_
