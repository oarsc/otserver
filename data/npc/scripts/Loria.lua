dofile(getDataDir() .. 'npc/scripts/lib/greeting.lua')

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

-- OTServ event handling functions
function onCreatureAppear(cid)         npcHandler:onCreatureAppear(cid)         end
function onCreatureDisappear(cid)      npcHandler:onCreatureDisappear(cid)      end
function onCreatureSay(cid, type, msg) npcHandler:onCreatureSay(cid, type, msg) end
function onThink()                     npcHandler:onThink()                     end

keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am studying the power of magic all the time."})
keywordHandler:addKeyword({'lake'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I hope you like it. It is named like my master, Alatar, the Sage"})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am Loria, a former apprentice of Alatar, the Sage."})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Time means nothing to me."})
keywordHandler:addKeyword({'buy'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I don't care for money, I care for magic."})
keywordHandler:addKeyword({'power'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Although your attack spells get stronger with your usage of magic, real power is gained by finding strategies to properly use your magic abilities."})
keywordHandler:addKeyword({'mana'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Mana is the source of all magic. If you use spells, it will drain mana from your energy pool. This mana regenerates slowly, if you eat, or if you drink those mana fluids you can buy at Xodet's."})
keywordHandler:addKeyword({'gorn'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He runs an equipment shop close to the north gate of the city."})
keywordHandler:addKeyword({'xodet'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He runs a magic shop in the main road."})
keywordHandler:addKeyword({'praise'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I praise my master Alatar."})
keywordHandler:addKeyword({'alatar'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Well, he was my great master. He taught me all these fantastic things about magic. I really miss him."})
keywordHandler:addKeyword({'kill'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Killing and destruction are just foolish steaps to entrophy."})
keywordHandler:addKeyword({'quest'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I heard from a mystic bone of the lich lord below the House of Necromant. Bring it to me, and you will receive a reward."})
keywordHandler:addKeyword({'crystal'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The mystic crystal should be able to resurrect fresh corpses."})
keywordHandler:addKeyword({'necromant'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He lived in a lonely house in the south eastern part of Tibia beyond the mountains."})
keywordHandler:addKeyword({'rune'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "All spells starting with the syllable 'ad' must be burned into a rune. For this buy a rune from Xodet and put it in one of your hands. Now cast the formula of the spell."})
keywordHandler:addKeyword({'Muriel'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He runs his magic shop in the southwest of the city. He sells runes and spells and helps you, if you want to become a sorcerer."})

function creatureSayCallback(cid, type, msg)
	if not npcHandler:hasFocus(cid) then
		return false
	end
	msg = string.lower(msg)
	local cidData = npcHandler:getFocusPlayerData(cid)

	--name the spell--
	if msgcontains(msg, 'reward') then
		npcHandler:playerSay(cid, "I'll teach you a very seldom spell.", 1)
		cidData.state = 4

	elseif cidData.state == 1 then
		if msgcontains(msg, 'find person') then
			npcHandler:playerSay(cid, "Say the words: exiva 'name'", 1)
		elseif msgcontains(msg, 'light healing') then
			npcHandler:playerSay(cid, "Say the word: exura", 1)
		elseif msgcontains(msg, 'antidote') then
			npcHandler:playerSay(cid, "Say the words: exana pox", 1)
		elseif msgcontains(msg, 'intense healing') then
			npcHandler:playerSay(cid, "Say the words: exura gran", 1)
		elseif msgcontains(msg, 'poison field') then
			npcHandler:playerSay(cid, "Say the words: adevo grav pox", 1)
		elseif msgcontains(msg, 'great light') then
			npcHandler:playerSay(cid, "Say the words: utevo gran lux", 1)
		elseif msgcontains(msg, 'fire field') then
			npcHandler:playerSay(cid, "Say the words: adevo grav flam", 1)
		elseif msgcontains(msg, 'heavy magic missile') then
			npcHandler:playerSay(cid, "Say the words: adori gran", 1)
		elseif msgcontains(msg, 'magic shield') then
			npcHandler:playerSay(cid, "Say the words: utamo vita", 1)
		elseif msgcontains(msg, 'energy field') then
			npcHandler:playerSay(cid, "Say the words: adevo grav vis", 1)
		elseif msgcontains(msg, 'destroy field') then
			npcHandler:playerSay(cid, "Say the words: adito grav", 1)
		elseif msgcontains(msg, 'fire wave') then
			npcHandler:playerSay(cid, "Say the words: exevo flam hur", 1)
		elseif msgcontains(msg, 'ultimate healing') then
			npcHandler:playerSay(cid, "Say the words: exura vita", 1)
		elseif msgcontains(msg, 'great fireball') then
			npcHandler:playerSay(cid, "Say the words: adori gran flam", 1)
		elseif msgcontains(msg, 'fire bomb') or msgcontains(msg, 'firebomb') then
			npcHandler:playerSay(cid, "Say the words: adevo mas flam", 1)
		elseif msgcontains(msg, 'creature illusion') then
			npcHandler:playerSay(cid, "Say the words: utevo res ina 'creature'", 1)
		elseif msgcontains(msg, 'poison wall') then
			npcHandler:playerSay(cid, "Say the words: adevo mas grav pox", 1)
		elseif msgcontains(msg, 'explosion') then
			npcHandler:playerSay(cid, "Say the words: adevo mas hur", 1)
		elseif msgcontains(msg, 'fire wall') then
			npcHandler:playerSay(cid, "Say the words: adevo mas grav flam", 1)
		elseif msgcontains(msg, 'great energy beam') then
			npcHandler:playerSay(cid, "Say the words: exevo gran vis lux", 1)
		elseif msgcontains(msg, 'invisible') then
			npcHandler:playerSay(cid, "Say the words: utana vid", 1)
		elseif msgcontains(msg, 'summon creature') then
			npcHandler:playerSay(cid, "Say the words: utevo res 'creature'", 1)
		elseif msgcontains(msg, 'energy wall') then
			npcHandler:playerSay(cid, "Say the words: adevo mas grav vis", 1)
		elseif msgcontains(msg, 'energy wave') then
			npcHandler:playerSay(cid, "Say the words: exevo mort hur", 1)
		elseif msgcontains(msg, 'sudden death') then
			npcHandler:playerSay(cid, "Say the words: adori vita vis", 1)
		elseif msgcontains(msg, 'energy beam') then
			npcHandler:playerSay(cid, "Say the words: exevo vis lux", 1)
		elseif msgcontains(msg, 'light') then
			npcHandler:playerSay(cid, "Say the words: utevo lux", 1)
		end

	elseif cidData.state == 4 and msgcontains(msg, 'spell') then
		npcHandler:playerSay(cid, "I'll teach you 'exevo gran mas vis', but bring me this bone first!", 1)
		cidData.state = 4

	elseif msgcontains(msg, 'magic') or msgcontains(msg, 'spell') then
		if isKnight(cid) then
			npcHandler:playerSay(cid, "I could tell you much about all mage spells, but you won't understand it. Anyway, feel free to ask me.", 1)
		else
			npcHandler:playerSay(cid, "Oh, I can tell you a lot about all mage spells. Feel free to ask me.", 1)
		end
		cidData.state = 0

	elseif msgcontains(msg, 'find person') then
		npcHandler:playerSay(cid, "If you search someone, this spell will give you an idea of the direction you must head. You will be able to see, whether he is below or above you.", 1)
	elseif msgcontains(msg, 'light healing') then
		npcHandler:playerSay(cid, "The paths to the next temple are long. Even in Tibia. So learn this spell, and be able to heal yourself during your travels. This spell will only cure small wounds, but it is pretty helpful.", 1)
	elseif msgcontains(msg, 'antidote') then
		npcHandler:playerSay(cid, "This spell sucks the venom out of your veins, that some enemy might have injected.", 1)
	elseif msgcontains(msg, 'intense healing') then
		npcHandler:playerSay(cid, "This spell will cure more wounds or greater ones at once. This is of course more 'mana intensive', but everybody will sooner or later get in a situation where mana is nothing - compared to life.", 1)
	elseif msgcontains(msg, 'poison field') then
		npcHandler:playerSay(cid, "This spell will create a single field of poisonous gas. Cast it on a creature you were not able to arrange a peace treaty with. If it has no antidote, watch what could happen if you forget yours.", 1)
	elseif msgcontains(msg, 'great light') then
		npcHandler:playerSay(cid, "This spell will illuminate your whole screen and last longer than 'light'. Use it in deep dungeons, 'cause behind every corner there could be your last enemy ... and you might just walk into him.", 1)
	elseif msgcontains(msg, 'fire field') then
		npcHandler:playerSay(cid, "This spell acts similar to the 'poison field' spell, except that you create fire instead of poisonous gas. Don't enter it yourself or you will realize why it is said that 'fire eats everything'.", 1)
	elseif msgcontains(msg, 'heavy magic missile') then
		npcHandler:playerSay(cid, "Remember the spell where you only got to wave your hand? Well, wave it twice and shoot a heavy magic missile at your enemy. This spell will create a rune with five charges.", 1)
	elseif msgcontains(msg, 'magic shield') then
		npcHandler:playerSay(cid, "Well, mages are more bookworms than sportsmen, and as such often neglect their physical fitness. In ancient tomes lies the power to use mana as an equivalent to life. So use this spell to survive.", 1)
	elseif msgcontains(msg, 'energy field') then
		npcHandler:playerSay(cid, "This one will create a field of energy. Everyone stepping in will be struck from lightning. This field will not last as long as poison or fire fields, but it is more deadly.", 1)
	elseif msgcontains(msg, 'destroy field') then
		npcHandler:playerSay(cid, "Trapped again between fire, poison and energy fields? This spell will give you the ability to destruct the fields, so you can pass safely.", 1)
	elseif msgcontains(msg, 'fire wave') then
		npcHandler:playerSay(cid, "Turn to you opponent and release the forces of nature with a whisper of your voice. A triangle of fire will burn all persons in your view, so take care, in which direction you look!", 1)
	elseif msgcontains(msg, 'ultimate healing') then
		npcHandler:playerSay(cid, "This spell is able to cure almost every injury at a higher cost than the other healing spells.", 1)
	elseif msgcontains(msg, 'great fireball') then
		npcHandler:playerSay(cid, "Imagine scaling the normal fireball by two and raising the fire temperature.", 1)
	elseif msgcontains(msg, 'firebomb') then
		npcHandler:playerSay(cid, "With a snip of your finger you can cover the floor with a burning carpet that keeps on burning for a while.", 1)
	elseif msgcontains(msg, 'creature illusion') then
		npcHandler:playerSay(cid, "A good one to scare childs. You can change your appearance to any monster. You can be as handsome as a ghoul!", 1)
	elseif msgcontains(msg, 'poison wall') then
		npcHandler:playerSay(cid, "With this one you can create a huge wall of poisonous gas. Many monsters will be too scared to pass the wall and if they do, they will choke from nausea.", 1)
	elseif msgcontains(msg, 'explosion') then
		npcHandler:playerSay(cid, "A strong blast of fire wounds the opponent you point at, and the adjacent squares.", 1)
	elseif msgcontains(msg, 'fire wall') then
		npcHandler:playerSay(cid, "As the poison wall, this spell creates an even larger wall of fire, burning everyone who passes.", 1)
	elseif msgcontains(msg, 'great energy beam') then
		npcHandler:playerSay(cid, "A lightning bolt strikes the point you look at.", 1)
	elseif msgcontains(msg, 'energy beam') then
		npcHandler:playerSay(cid, "A ray of energy will strike everyone in your current direction", 1)
	elseif msgcontains(msg, 'invisible') then
		npcHandler:playerSay(cid, "This spell drains the colors out of you body, making yourself invisible for an hour or two.", 1)
	elseif msgcontains(msg, 'summon creature') then
		npcHandler:playerSay(cid, "This one gives you the ability to summon monsters that aid you in your battles.", 1)
	elseif msgcontains(msg, 'energy wall') then
		npcHandler:playerSay(cid, "Attracts lightning bolts from the sky, to form a giant wall, seriously damaging everyone who passes.", 1)
	elseif msgcontains(msg, 'energy wave') then
		npcHandler:playerSay(cid, "Shoots a triangular bundle of lightning bolts in the direction you look.", 1)
	elseif msgcontains(msg, 'sudden death') then
		npcHandler:playerSay(cid, "The best spell for deciding a battle within seconds. The spell tries to interrupt the opponents heart beat, leading to his instant death in most cases.", 1)
	elseif msgcontains(msg, 'light') then
		npcHandler:playerSay(cid, "A ray of light will emerge from your flat hand to illuminate your environment.", 1)
	elseif msgcontains(msg, 'formula') then
		npcHandler:playerSay(cid, "Which is the spell, you need the formula to?", 1)
		cidData.state = 1
	end
end

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())