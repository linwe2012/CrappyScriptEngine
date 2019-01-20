#ifndef _INTERMEDIATE_H_
#define _INTERMEDIATE_H_
#include "attribute.h"
#include "factory.h"
#include <time.h>
class Game {
public:
	void addWorld(std::string name, World* world) {
		worlds_[name] = world;
	}
	int setCurrentWorld(std::string name) {
		auto itr = worlds_.find(name);
		if (itr == worlds_.end()) return -1;
		current_world_ = itr->second;
		return 0;
	}
	int loop() {
		while (1)
		{
			std::string tmp;
			Console::out << Console::Blue.light() << "> ";
			std::getline(std::cin, tmp);
			if (tmp == "exit") {
				return 0;
			}
			// quick &dirty implement of command
			if (tmp[0] == '/') {
				if (tmp == "/srand")
					srand(time(nullptr));
				else if (tmp == "/undead on") {
					allow_exit = 0;
				}
				else if(tmp == "/undead off"){
					allow_exit = 1;
				}
				else if (tmp.find("/set") == 0) {
					std::stringstream ss(tmp);
					ss >> tmp;
					ss >> tmp;
					Attribute *att = current_world_->lookupPath(tmp, ".");
					if (att == nullptr) {
						Console::out << "unable to find attribute" << std::endl;
						continue;
					}
					int id = 0;
					if (auto num = att->getAsNumber()) {
						ss >> id;
						num->setData(id);
					}
					else if (auto str = att->getAsString()) {
						ss >> tmp;
						str->setData(tmp);
					}
					
					else {
						Console::out << "Unable to set data" << std::endl;
					}
				}
				
				else
				{
					std::stringstream ss(tmp);
					ss >> tmp;
					if (tmp == "/show") {
						ss >> tmp;
						Attributes *a = new Attributes();
						FunctionsLibrary::__PrintStringFImplement(current_world_, current_world_->actives(), a, tmp);
						delete a;
					}
					else
						Console::out << "unkown command" << std::endl;
				}
				continue;
			}
			
			Console::out << Console::White << std::endl;
			current_world_->setCmdBuffer(tmp);
			if (shouldExit())
				return 0;
			current_world_->refresh();
			if (shouldExit())
				return 0;
			current_world_->fireEvent("$onactive");
			if (shouldExit())
				return 0;
			current_world_->refresh();
			if (shouldExit())
				return 0;
		}
		return -1;
	}
	int shouldExit() {
		return allow_exit &&
			(current_world_->shouldTerminate() || current_world_->shouldExit());
	}
private:
	std::map<std::string, World *> worlds_;
	World *current_world_;
	int allow_exit = 1;
};

#endif // !_INTERMEDIATE_H_



