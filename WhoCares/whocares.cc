#include "templates_library.h"
#include "listeners.h"
#include <iostream>
#include <fstream>
#include "worldgen.h"
#include "bootstrapper.h"
void testTemplate();
void testIO();

//
//  █     █░ ██░ ██  ▒█████   ▄████▄   ▄▄▄       ██▀███  ▓█████   ██████
//  ▓█░ █ ░█░▓██░ ██▒▒██▒  ██▒▒██▀ ▀█  ▒████▄    ▓██ ▒ ██▒▓█   ▀ ▒██    ▒
//  ▒█░ █ ░█ ▒██▀▀██░▒██░  ██▒▒▓█    ▄ ▒██  ▀█▄  ▓██ ░▄█ ▒▒███   ░ ▓██▄
//  ░█░ █ ░█ ░▓█ ░██ ▒██   ██░▒▓▓▄ ▄██▒░██▄▄▄▄██ ▒██▀▀█▄  ▒▓█  ▄   ▒   ██▒
//  ░░██▒██▓ ░▓█▒░██▓░ ████▓▒░▒ ▓███▀ ░ ▓█   ▓██▒░██▓ ▒██▒░▒████▒▒██████▒▒
//  ░ ▓░▒ ▒   ▒ ░░▒░▒░ ▒░▒░▒░ ░ ░▒ ▒  ░ ▒▒   ▓▒█░░ ▒▓ ░▒▓░░░ ▒░ ░▒ ▒▓▒ ▒ ░
//  ▒ ░ ░   ▒ ░▒░ ░  ░ ▒ ▒░   ░  ▒     ▒   ▒▒ ░  ░▒ ░ ▒░ ░ ░  ░░ ░▒  ░ ░
//  ░   ░   ░  ░░ ░░ ░ ░ ▒  ░          ░   ▒     ░░   ░    ░   ░  ░  ░
//  ░     ░  ░  ░    ░ ░  ░ ░            ░  ░   ░        ░  ░      ░
//  ░


int main()
{
	

	Console::SetUpConsole();
	PlaceholderAttribute *null = new PlaceholderAttribute(nullptr, "hi");
	auto wtf = NumberAttribute::cast(null);
	std::cout << wtf->data() << std::endl;
	wtf->setData(100);
	std::cout << wtf->data() << std::endl;

	// Console::out << Console::Red << "Testing red" << Console::Blue << std::endl;
	auto game = Gensis::bigbang(0);
	if(game == nullptr){
		Console::out << "Program ended by user" << std::endl;
		return 0;
	}
	game->loop();
	Console::out << "-------Game Terminated-------";
	// GameLoader gl;
	// gl.scanFiles();
	// gl.printFiles();
	// 
	// testTemplate();
	// testIO();
	// Listeners listeners;
	// listeners += "$onload", "$onactive", "$onleave";
	// World westeros(&listeners);
	// Templates templs;
	// TemplatesLibrary templs_lib(&westeros, &templs);

	// templs_lib.installTemplate("Hero", "!@#$%^&");
	// templs_lib.installTemplate("Room", "Square");
	// templs_lib.installTemplate("Room", "Castle Rock");
	// templs_lib.installTemplate("Room", "Kingsland");
	// templs_lib.installTemplate("Hero", "!@#$%^&");
	// templs_lib.installTemplate("NPC", "King Robert");
	// templs_lib.installTemplate("NPC", "Little Finger");
	// templs_lib.installTemplate("NPC", "Cersi Lanister");
	// 
	// 
	// westeros.setRelation("$room.Castle Rock.exits", "$room.Square", "north", ".");
	// westeros.setRelation("$room.Square.exits", "$room.Castle Rock", "south", ".");
	// westeros.setRelation("$room.Kingsland.exits", "$room.Castle Rock", "west", ".");
	// westeros.setRelation("$room.Castle Rock.exits", "$room.Kingsland", "east", ".");
	// westeros.setRelation("$room.Kingsland.npc", "$npc.King Robert", "King Robert", ".");
	// westeros.setRelation("$room.Castle Rock.npc", "$npc.Little Finger", "Little Finger", ".");
	// westeros.setRelation("$room.Castle Rock.npc", "$npc.Cersi Lanister", "Cersi Lanister", ".");
	// 
	// std::stringstream ss;
	// gl.loadFile(ss);
	// std::function<void(std::string)> new_world_call_back;
	// WorldGen w_gen(&westeros);
	// w_gen.compile(ss, gl.getCurrentFileDir());
	// 
	// 
	// westeros.activate("$room", westeros.lookupPath("$room.Square", "."));
	// westeros.activate("$player", westeros.lookupPath("$player.!@#$%^&", "."));
	// westeros.fireEvent("$onload");
	// westeros.fireEvent("$onactive");
	// westeros.addcmd2event("goto", "on_goto");
	// 
	// westeros.setCmdBuffer("goto south"); //Castle Rock
	// westeros.refresh();
	// westeros.setCmdBuffer("goto east"); //Kingsland
	// westeros.refresh();
	// westeros.setCmdBuffer("goto west"); //Castle Rock
	// westeros.refresh();
	// westeros.setCmdBuffer("goto north"); //Square
	// westeros.refresh();
	// westeros.setCmdBuffer("goto north");
	// westeros.refresh();
	// westeros.lazyActivate("$room", westeros.lookupPath("$room.Yunkai", ".")->getAsAttributes());
	// westeros.refresh();
	// westeros.setCmdBuffer("goto north");
	// westeros.refresh();
	// std::string cmd;
	// while (true)
	// {
	// 	std::getline(std::cin, cmd);
	// 	westeros.setCmdBuffer(cmd);
	// 	westeros.refresh();
	// }
	getchar();
	getchar();
	getchar();
	getchar();
}

void testTemplate() {
	// Templates templs;
	// Attribute *hp = templs.newTemplate("hp");
	// Attribute *hp_hp = templs.digin(&hp, "hp");
	// Attribute *hp_hp_prt = templs.digin(&hp_hp, "ptr");
	// 
	// Attribute *hp_addon = templs.digin(&hp, "addon");
	// Attribute *hp_addon_basic = templs.appendAttribute(&hp_addon, "basic", new NumberAttribute(20));
	// Attribute *hp_dup = hp->duplicate();
}


void testIO() {
	//std::string test_utf = u8"Hello this is a Test. 你好，这是测试文字。 Wie geht's, Ich habe nach Östereish gegangen. Tschüss.\n"
	//	"This is some language from hell: महसुस, Greek: αβγδ; German: Übergrößenträger,Ää, Öö, Üü";

	//std::cout << test_utf;
}