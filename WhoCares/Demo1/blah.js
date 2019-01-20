
$setinvoker('on_shop', "shop")
$setinvoker('on_but', "buy")
$setinvoker('on_goto', "go")
$setinvoker('on_fight', "fight")
$setinvoker('on_info', 'info', 'self')
<$City> = {
	extra: 50
	select: {
		a: 'You are in the great land of ${name} ${path}[, you can see path to >${$name$}(${$node$.name})><]'
		b: 'The mysterious land of ${name} is infront of you ${path}[, you can see path to >${$name$}(${$node$.name})><]'
	}
	greeting: ()=> {
		$invoker("$onload")
		what = $random("select")
		$print("${what}")
		Person.main.gold += extra
		$print('You have earned ${extra} gold')
	}
	move: ()=>{
		$invoker("on_goto")
		dst = $read()
		if("path.${dst}"){
			$activate('City', 'path.${dst}')
		}
		else{
			$print('No such City')
		}
	}
	fight: () => {
		$invoker('on_fight')
		$print('Select Someone You want to fight: ${npc}[>${$name$}(${$node$.name})>,<.]')
		def = $read()
		if('npc.${def}'){
			$call('npc.${def}.fight')
		}
		else{
			$print('No such person')
		}
	}
}

<Weapon> = {
	buy: () => {
		if(Person.main.gold > gold){
			Person.main.gold -= gold
			'Person.main.weapon.${name}' += $this
			Person.main.atk += atk
			$print('You have successfully bought it. Use info to check out')
		}
		else{
			$print('This weapon need ${gold} gold, yet you only have ${Person.main.gold}')
		}
	}
	
}

Knife<Weapon> = {
	gold: 100
	atk: 20
	name: 'knife'
}
Gun<Weapon> = {
	gold: 9999
	atk: 9999
	name: 'gun'
}
Dragon<Weapon> = {
	gold: 99999
	atk: 99999
	name: 'dragon'
}
<Shop> = {}

S<Shop> = {
	items: Knife, Gun, Dragon
	shop: () => {
		$invoker('on_shop', -1)
		$print('You have ${Person.main.gold} gold')
		$print('We have got ${items}[>${$name$} which costs ${$node$.gold}, atk: ${$node$.atk}>, <.]')
		buy = $read()
		if('items.${buy}'){
			$call("items.${buy}.buy")
		}
	}
}

<Person> = {
	fight: () => {
		if(atk > Person.main.atk){
			$print('You are dead')
			$setflag("Terminate")
		}else{
			$print('You win')
			Person.main.gold += gold
		}
	}
}

Yunkai<City> = {
	name: 'Yunkai'
	npc: DaenerysTargaryen, JorahMormont
	
	path: {
		north: WinterFall
	}
	queensays: () => {
		// runtime functions start with dollar
		$invoker("$on_hi", 4)
		$call("npc.DaenerysTargaryen.saysomething")
	}
}

RiverRun<City> = {
	name: 'River Run'
	path: {
		south: Yuankai
		north: WinterFall
	}
}

WinterFall<City> = {
	name: 'Winter Fall'
	npc: NedStark
	path: {
		south: Yunkai
		southwest: RiverRun
	}
}


DaenerysTargaryen<Person> = {
	atk: 5
	gold: 102
	name: 'Daenerys Targaryen'
	saysomething: () => {
		$print("Daenerys Targaryen says: I'm your queen")
	}
}

NedStark<Person> = {
	atk: 100
	gold: 9999
	name: 'Ned Stark'
}

JorahMormont<Person> = {
	atk: 80
	gold: 899
	name: 'Jorah Mormont'
}

main<Person> = {
	atk: 10
	gold: 100
	info: () =>{
		$invoker('on_info', -1)
		$print('You have ${atk} attack and ${gold} gold')
	}
}

$activate('City', 'City.Yunkai')

