include 'templates.js'

// $setinvoker(event name, input1, input2, ...)
// once input is seen, will fire the event
$setinvoker('on_hi', "hi", "hello")
$setinvoker('on_goto', "go", "goto")



Yunkai<Room> = {
	name: 'Yunkai'
	// testing forward declaration.
	npc: DaenerysTargaryen, JorahMormont
	numbers:{
		a: 100
		b: 200
	}
	check: 'open'
	// testing nested block expr.
	exits: {
		north: WinterFall
	}
	// testing lambda call.
	queensays: () => {
		// runtime functions start with dollar.
		// invoker(event name) bind this function to a listener
		$invoker("$onload")
		$print("Testing: ${npc}[--This is prefix-->${$name$}: ${$node$.name}> Seperator <--Posfix--9]")
		// $npc.DaenerysTargaryen is activated as $npc
		$activate('$npc', "$npc.DaenerysTargaryen")
		$call("npc.DaenerysTargaryen.say")
		$call("npc.JorahMormont.say")
		numbers.c = numbers.a
		numbers.c += numbers.b
		$print(`${numbers}[Numbers: >${$name$} = ${$node$}>,<.]`)
		if(numbers.c){
			$print("there is c")
		} else {
			$print("there is no c")
		}
		uvw = $read()
		if(uvw == check){
			$print("code is correct")
		}else {
			$print("code is wrong")
		}
	}
	leave: () => {
		$invoker("$onleave")
		$print(`leaving ${name}`)
		//TODO: Deactivate $npc.DaenerysTargaryen
	}
}


RiverRun<Room> = {
	name: 'River Run'
	exits: {
		south: Yunkai
		north: WinterFall
	}
}

WinterFall<Room> = {
	name: 'Winter Fall'
	npc: NedStark
	exits: {
		south: Yunkai
		southwest: RiverRun
		king: Kingsland
	}
}

Kingsland<Room> = {
	name: `King's land`
	npc: KingRobert
	exits: {
		west: CastleRock
		northwest: RiverRun
		north: WinterFall
	}
	items: 'iron throne'
}

CastleRock<Room> = {
	name: `Castle Rock`
	npc: LittleFinger, CersiLanister
	eixts:{
		west: Kingsland
		north: RiverRun
	}
}

DaenerysTargaryen<NPC> = {
	name: 'Daenerys Targaryen'
	title: 'Daenerys Targaryen'
	numa: 20
	numb: 30
	say: () => {
		$invoker("on_hi")
		$print("Daenerys Targaryen says: I'm your queen")
	}
	sayanother: () => {
		$invoker("$onload")
		$print("Daenerys Targaryen is loaded")
		if(name != title){
			$print("name != title")
		}
		// test nested if-else
		else if(name == title) {
			$print("name == title")
		}
		else {
			$print("name ? title")
		}
	}
}

NedStark<NPC> = {
	name: 'Ned Stark'
}

JorahMormont<Knight> = {
	name: 'Jorah Mormont'
}

KingRobert<NPC> = {
	name: 'King Robert'
}
LittleFinger<NPC> = {
	name: 'Little Finger'
}
CersiLanister<NPC> = {
	name: 'Cersi Lanister'
}

// there will be only ONE $room be activated
$activate('$room', '$room.Kingsland')
