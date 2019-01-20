#ifndef _BOOTSTRAPPER_H_
#define _BOOTSTRAPPER_H_
#include <vector>
#include <string>
#ifdef _MSC_VER
#include <filesystem>
#else
#include <experimental/filesystem>
#endif // _MSVC_VER
namespace fs = std::experimental::filesystem;
#include "templates_library.h"
#include "game.h"
#include "worldgen.h"
class GameDescript {
	fs::path directory_;
	std::string game_name_;
	std::string descript_;
	std::string entry_;
	std::string usage_;
public:
	fs::path directory() { return directory_; };
	std::string game_name() { return game_name_; }
	std::string descript() { return descript_; }
	std::string entry() { return entry_; }
	std::string usage() { return usage_; }
	GameDescript(fs::path directory, std::string game_name, std::string descript, std::string entry, std::string usage)
		:directory_(directory), game_name_(game_name), descript_(descript), entry_(entry), usage_(usage){}
	// This one is for vector
	GameDescript() {}
};


class GameLoader {
public:
	void scanFiles();
	void printFiles();
	void loadFile(std::stringstream& ss);
	fs::path getCurrentFileDir();
	GameDescript& getCurrentGameInfo();
	int checkValid() {return active_id_ >= 0;}
	GameLoader();
private:
	std::string where_i_am_;
	std::vector<GameDescript> game_files_;
	int active_id_;
};

class Gensis {
public:

	static Game* bigbang(UNUSED int test) {
		IGNORE(test);
		static GameLoader gl;
		if (current_game_ != nullptr)
			delete current_game_;
		if (temp_lib_ != nullptr)
			delete temp_lib_;
		current_game_ = new Game();

		World *world = new World();
		auto templs = new Templates();
		temp_lib_ = new TemplatesLibrary(world, templs);
		gl.scanFiles();
		gl.printFiles();
		std::stringstream ss;
		gl.loadFile(ss);
		if(!gl.checkValid())
			return nullptr;
		Factory::bindTemplates(templs);
		compileWorld(world, ss, gl);
		auto game_desc = gl.getCurrentGameInfo();
		current_game_->addWorld(game_desc.game_name(), world);
		current_game_->setCurrentWorld(game_desc.game_name());
		return current_game_;
	}
	static void compileWorld(World *world, std::stringstream& ss, GameLoader& gl) {
		WorldGen worldgen(world);
		auto game_desc = gl.getCurrentGameInfo();
		worldgen.compile(ss, gl.getCurrentFileDir(), game_desc.entry());
	}
private:
	static TemplatesLibrary *temp_lib_;
	static Game* current_game_;
};

#endif // !_BOOTSTRAPPER_H_
