$setinvoker('on_goto', 'go', 'jump')
$setinvoker('on_resc', 'rescue', 'take', 'help')
$setinvoker('on_use', 'OffnenDieseTurUndIchMochteNachGehen','OffnenDieseTur', 'open')
$setinvoker('on_ask', 'search', 'ask', 'find')
//  --- N P C S ---
Beauty<NPC> = {
	cnt: 0
	inc: 1
	help: () => {
		$invoker("$onload", -1)
		if(cnt != inc){
			cnt += inc
			$print('Princess: Please take me out of here')
		}
	}
	take: () => {
		$invoker("on_resc", -1)
		if(cnt){
			$player.main.princ := $this
			$print('The princess follows you')
		}
	}
}

Kight<NPC> = {
	wander: () =>{
		$invoker('$onload', -1)
		$random(`$parent.exits`).knight := $this
	}
}

Beast<NPC> = {
	lastRoom: Kitchen
	kill: () =>{
		$invoker('$onload', -1)
		if($active.$room.name == lastRoom.name){
			if(lastRoom.knight){
				$print('You are rescued by the Kight, he said: RUN, RUN AS FAR AS YOU CAN')
			}
			else {
				$print('You are killed by the beast')
				$setflag("Terminate")
			}
		}
		else {
			$print("The beast is at ${lastRoom.name}")
			$random(`lastRoom.exits`).beast := $this
			$this.lastRoom := $this.$parent
		}
	}
}

//  --- R O O M S ---
Hall<Room> = {
	name: 'Hall'
	cnt: 0
	inc: 1
	welc: `Hey, welcome to the game. You job is to find princess and take her out of here. Be aware that there the beast is patroling. You can't fight the beast as you are unarmed. Good news is, there is also a kight wandering, and he will save you when beast strike iff you 3 are in the same room.`
	init: () => {
		$invoker("$onload")
		if(cnt != inc){
			cnt += inc
			$print("${welc}")
			$random("$room").princ := $npc.Beauty
		}
	}
	win: () => {
		$invoker("$onload")
		if($player.main.princ){
			$print("You saved the princess !!!!!")
			$setflag("Terminate")
		}
	}
	exits:{
		north: Living
		south: Dinning
		down: Dungeon
		up: SecondFloor
	}
}

Living<Room> = {
	name: 'Living Room'
	exits:{
		south: Hall
		window: Garden
	}
}

SecondFloor<Room> = {
	name: 'Second Floor'
	exits: {
		down: Hall
		first: SleepA
		second: SleepB
		third: SleepC
	}
}
Garden<Room> = {
	name: 'Garden'
	exits:{
		north: Dinning
		south: Hall
	}
}

Dinning<Room> = {
	name: 'Dining Room'
	exits:{
		east: Kitchen
		window: Garden
	}
}

SleepA<Room> = {
	name: 'Huge Sleeping Room'
	exits:{
		north: BalconyA
		back: SecondFloor
	}
}
SleepB<Room> = {
	name: 'Small Sleeping Room'
	exits:{
		back: SecondFloor
		magic: Magic
	}
}

SleepC<Room> = {
	name: 'Medium Sleeping Room'
	exits:{
		back: SecondFloor
	}
	knight: Kight
}

BalconyA<Room> = {
	name: 'Balcony'
	exits: {
		outside: VOID
		back: SleepA
	}
}

VOID<Room> = {
	name: 'You can see Garden outside'
	dead: ()=>{
		$invoker("$onload")
		$print("Your fell and dead")
		$setflag("Terminate")
	}
	exits:{
		// these are the exits for beast
		// otherwise beast may disappear for good
		back: BalconyA
	}
}
// /show ${$room.$size}
// /show ${$npc.Beauty.$parent.name}
// /show ${$npc.Beauty.name.$parent}[${$node$} : $name$]
Kitchen<Room> = {
	name: 'Kitchen'
	call: () =>{
		$invoker("$onload")
		$call("princ.help")
	}
	exits: {
		west: Dinning
	}
}


Dungeon<Room> = {
	name: 'Dungeon'
	diss: () => {
		$invoker("$onload")
		$print("You see A bunch of people down there. But it seems you can't help the")
	}
	help: () => {
		$invoker("on_ask")
		$print("You can't find the princess. 'No one will lock a princess in the Dungeon', people laughed.")
	}
	exits:{
		up: Garden
	}
}


// Magic is not defined as room, 
// so the only way out is random door
Magic<Room> = {
	name: 'Magic Room'
	item: WillTur
	code: 'alohomora'
	give: () =>{
		$invoker("$onload")
		$print("There is a HUUUGE box here. Wanna Open? Try a spell")
		somthing = $read()
		if(somthing == code){
			$print("You get a random door here. It can take you any where. \nTry say the spell 'OffnenDieseTurUndIchMochteNachGehen'")
			$player.main.item = item
			$room.SleepB.exits.magic := null
		}
		else{
			$print("Wrong Spell....Please resume your journey")
		}
	}
	exits:{
		death: VOID
	}
}

main<Hero> = { }
//  --- I T E M S ---
WillTur = {
	use: () => {
		$invoker("on_use", 4)
		$print("Choose the room you want to go: ${$room}[>${$name$}(${$node$.name})>,<.]")
		dst = $read()
		$activate('$room', "$room.${dst}")
	}
}

$srand()
$activate('$room', "$room.Hall")