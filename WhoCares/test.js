Yunkai<Room> = {
	name: 'Yunkai'
	npc: DaenerysTargaryen, JorahMormont
	exits: {
		north: WinterFall
	}
	queensays: () => {
		// runtime functions start with dollar
		$invoker("$onload")
		$print("Testing: ${npc}[--This is prefix-->${$name$}: ${$node$.name}> Seperator <--Posfix--9]")
		$activate("$npc.DaenerysTargaryen")
	}
}

RiverRun<Room> = {
	name: 'River Run'
	exits: {
		south: Yuankai
		north: WinterFall
	}
}

WinterFall<Room> = {
	name: 'Winter Fall'
	npc: NedStark
	exits: {
		south: Yunkai
		southwest: RiverRun
	}
}


DaenerysTargaryen<NPC> = {
	name: 'Daenerys Targaryen'
	saysomething: () => {
		$invoker("$onload")
		$print("Daenerys Targaryen says: Come to your queen")
	}
}

NedStark<NPC> = {
	name: 'Ned Stark'
	
}

JorahMormont<NPC> = {
	name: 'Jorah Mormont'
}
