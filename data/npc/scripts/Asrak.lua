dofile(getDataDir() .. 'npc/scripts/lib/greeting.lua')

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

-- OTServ event handling functions
function onCreatureAppear(cid)			npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)		npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)		end
function onThink()				npcHandler:onThink()					end

keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am the overseer of the pits and the trainer of the gladiators."})
keywordHandler:addKeyword({'shop'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am the overseer of the pits and the trainer of the gladiators."})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am known as Asrak the Ironhoof."})
keywordHandler:addKeyword({'king'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I pledge no allegiance to any king, be it human or minotaurean."})
keywordHandler:addKeyword({'tibianus'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I pledge no allegiance to any king, be it human or minotaurean."})
keywordHandler:addKeyword({'venore'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The city pays me well and those undisciplined gladiators need my skills and guidance badly."})
keywordHandler:addKeyword({'gladiator'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Those wannabe fighters are weak and most of them are unable to comprehend a higher concept like the Mooh'Tah."})
keywordHandler:addKeyword({'trainer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Those wannabe fighters are weak and most of them are unable to comprehend a higher concept like the Mooh'Tah."})
keywordHandler:addKeyword({'minotaur'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "In the ancient wars we lost much because of the rage. The one good thing is we lost our trust in the gods, too."})
keywordHandler:addKeyword({'god'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "By them we were imbued with the rage that almost costed our existence. By them we were used as pawns in wars that were not ours."})
keywordHandler:addKeyword({'mintwallin'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The city is only a shadow of what we could have accomplished without that curse of rage that the gods bestowed upon us"})
keywordHandler:addKeyword({'rage'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Rage is the legacy of Blog, the beast. To overcome it is our primal goal. The Mooh'Tah is our only hope of salvation and perfection."})
keywordHandler:addKeyword({'guidance'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Like all true minotaurean blademasters I am a warrior-philosopher of the Mooh'Tah."})
keywordHandler:addKeyword({"mooh'tah"}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The Mooh'Tah teaches us control. It provides you with weapon, armor, and shield. It teaches you harmony and focus."})
keywordHandler:addKeyword({'harmony'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "There is an elegant harmony in every thing done right. If you feel the harmony of an action you can sing its song."})
keywordHandler:addKeyword({'sing'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Each harmonic action has it own song. If you can sing it, you are in harmony with that action. This is where the minotaurean battlesongs come from."})
keywordHandler:addKeyword({'song'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Each harmonic action has it own song. If you can sing it, you are in harmony with that action. This is where the minotaurean battlesongs come from."})
keywordHandler:addKeyword({'battlesongs'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Each Mooh'Tah master focuses his skills on the harmony of battle. He is one with the song that he's singing with his voice or at least his heart."})
keywordHandler:addKeyword({"mooh'tah master"}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Mooh'Tah masters are the epitome of the minotaurean warrior-philosophers. Full in control, free of rage, focused in perfect harmony with their actions."})
keywordHandler:addKeyword({"warrior-philosopher"}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Mooh'Tah masters are the epitome of the minotaurean warrior-philosophers. Full in control, free of rage, focused in perfect harmony with their actions."})
keywordHandler:addKeyword({'general'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Your human generals are like their warriors. They lack the focus to be a true warrior."})
keywordHandler:addKeyword({'army'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Your human army might be big, but without skills. They are only sheep to be slaughtered."})
keywordHandler:addKeyword({'ferumbras'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "To rely on magic is like to cheat fate. All cheaters will find their just punishment one day, and so will he."})
keywordHandler:addKeyword({'excalibug'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "If it's truly a weapon to slay gods it might be worth to be sought for."})
keywordHandler:addKeyword({'new'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Focus on your own life, not on that of others."})
keywordHandler:addKeyword({'help'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I teach worthy warriors the way of the knight."})
keywordHandler:addKeyword({'monster'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Inferior creatures of rage, driven by their primitive urges. Only worthy to be noticed to test one's skills."})
keywordHandler:addKeyword({'dungeon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The dungeons of your desires and fears are the only ones you really should fear and those you really have to conquer."})
keywordHandler:addKeyword({'thank'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I hope you learned something."})
keywordHandler:addKeyword({'offer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I offer you the teachings of knighthood and the way of the paladin."})
keywordHandler:addKeyword({'training'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I offer you the teachings of knighthood and the way of the paladin."})
keywordHandler:addKeyword({'sell'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am not a merchant, but a warrior."})
keywordHandler:addKeyword({'have'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am not a merchant, but a warrior."})
keywordHandler:addKeyword({'weapon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Make your will your weapon, and your enemies will perish."})
keywordHandler:addKeyword({'armor'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Courage is the only armor that shields you against rage and fear, the greatest dangers you will have to face."})
keywordHandler:addKeyword({'shield'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Your confidence shall be your shield. Nothing can penetrate that defence. No emotion will let you lose your focus."})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is |TIME|."})


local spellSellModule = SpellSellModule:new()
npcHandler:addModule(spellSellModule)

spellSellModule.condition = function(cid) return isPaladin(cid) or isKnight(cid) end
spellSellModule.conditionFailText = "Sorry, I only teach paladins and knights!"
spellSellModule:addSpellStock({
	'Find Person',
	'Light Magic Missile',
	'Food',
	'Light Healing',
	'Antidote',
	'Intense Healing',
	'Great Light',
	'Magic Shield',
	'Conjure Poisoned Arrow',
	'Fireball',
	'Ultimate Healing',
	'Conjure Explosive Arrow',
	'Invisibility',
	'Light',
	'Conjure Arrow',
})

npcHandler:addModule(FocusModule:new())