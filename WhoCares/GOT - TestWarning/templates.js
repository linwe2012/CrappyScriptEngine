// test Template declaration & hashtags
// hashtag tells intepreter to find template and append it as child
// <template> declares it a template
<Knight>#HitPoint#Attack = {
	say: () => {
		$invoker("$onload")
		$print("I'm all yours, my lord")
	}
}
// We declare a template conflicts to builtin
<Room> = {
	
}
