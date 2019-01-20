#include "whocares.h"

#include <regex>
#include <fstream>
#include <sstream>

#include "bootstrapper.h"

using namespace Debugger;

void loadGameDescript(fs::path file, std::vector<GameDescript>& storage);

GameLoader::GameLoader() {
	where_i_am_ = fs::current_path().string();
	active_id_ = -1;
}

// scanFiles -- check file in dir
// accepted valid dir layout:
// where_i_am_.exe
// DirA
//    - bla.game
//    - blah.js
void GameLoader::scanFiles() {
	static std::regex dot_game( ".*\\.game$");
	for (auto &dir : fs::directory_iterator(where_i_am_)) {
		if(!fs::is_directory(dir.status())) continue;
		for (auto &file : fs::directory_iterator(dir.path())) {
			if (fs::is_directory(file)) continue;
			if (!std::regex_match(file.path().filename().string(), dot_game)) continue;
			loadGameDescript(file, game_files_);
			break;
		}
	}
}

void loadGameDescript(fs::path file, std::vector<GameDescript>& storage) {
	static std::regex at_entry("@entry:\\s*(.*)");
	static std::regex at_name("@name:\\s*(.*)");
	//TODO: Cannot match descrpition of that extends to the end of string w/o \n
	static std::regex at_descript("@description:\\s*([^@$]*)");
	static std::regex at_usage("@usage:\\s*([^@$]*)");
	std::string buffer;
	std::ifstream is(file.string());
	std::stringstream ss;
	std::string name = file.filename().string();
	std::string descript = "";
	std::string usage;
	file.remove_filename();
	size_t posfix_pos = name.find_last_of('.');
	name.erase(posfix_pos);

	ss << is.rdbuf();
	is.close();
	buffer = std::move(ss.str());
	std::smatch match_entry;
	std::regex_search(buffer, match_entry, at_entry);
	// https://www.geeksforgeeks.org/smatch-regex-regular-expressions-in-c/
	// The size() member function indicates the 
	// number of capturing groups plus one for the overall match 
	if (!match_entry.size()) {
		logError(new ERROR_EXPECTED_GAME_ENTRY(), "GameLoader::loadGameDescript");
		return;
	}
	std::string entry = match_entry.str(1);
	if (!fs::exists(file / entry)) {
		logError(new ERROR_GAME_FILE_ENTRY_NOT_FOUND(), "GameLoader::loadGameDescript");
	}
	
	std::smatch match_name, match_descript, match_usage;
	std::regex_search(buffer, match_name, at_name);
	std::regex_search(buffer, match_descript, at_descript);
	std::regex_search(buffer, match_usage, at_usage);
	

	if (match_name.size()) name = match_name.str(1);
	if (match_descript.size()) descript = match_descript.str(1);
	if (match_usage.size()) usage = match_usage.str(1);
	posfix_pos = descript.find_last_not_of('\n');
	if(posfix_pos < descript.size())
		descript.erase(++posfix_pos);
	posfix_pos = descript.find('\n');
	while (posfix_pos < descript.size()) {
		descript.replace(posfix_pos, 1, "\n\t");
		posfix_pos = descript.find('\n', posfix_pos + 1);
	}

	storage.push_back(std::move(GameDescript(file, name, descript, entry, usage)));
}


void GameLoader::printFiles() {
	int cnt = 0;
	int max = game_files_.size();
	Console::PrintF("You've got %d game%s:\n", max, max == 1 ? "" : "s");
	for (auto &desc : game_files_) {
		Console::PrintF("[%d] %s\n", cnt++, desc.game_name().c_str());
		Console::out << "\t" << desc.descript() << "\n\n";
	}
}

void GameLoader::loadFile(std::stringstream& ss) {
	int id;
	Console::out << "Select game id to start your journey." << std::endl;
	active_id_ = -1;
	while (true)
	{
		std::cin >> id;
		if (id < 0) { return; }
		if (static_cast<size_t>(id) < game_files_.size()) break;
		Console::PrintError("Invalid game id.\n");
	}
	auto &file = game_files_[id];
	std::ifstream is(file.directory() / file.entry());
	ss << is.rdbuf();
	is.close();
	Console::ClearScreen();
	active_id_ = id;
	Console::out << file.usage();
}

fs::path GameLoader::getCurrentFileDir() {
	if (active_id_ < 0) return (std::move(std::string("")));
	return game_files_[active_id_].directory();
}
GameDescript& GameLoader::getCurrentGameInfo() {
	return game_files_[active_id_];
}

TemplatesLibrary* Gensis::temp_lib_ = nullptr;
Game* Gensis::current_game_ = nullptr;

