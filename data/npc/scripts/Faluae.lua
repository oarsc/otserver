dofile(getDataDir() .. 'npc/scripts/lib/greeting.lua')

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)



-- OTServ event handling functions
function onCreatureAppear(cid)			npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)		npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)		end
function onThink()				npcHandler:onThink()					end


keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am the spokesperson of the Deraisim caste."})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am known as Faluae Ethrathil."})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Sorry, I don't know."})
keywordHandler:addKeyword({'carlin'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "For a human city Carlin is not that bad. Its still a scar in the natural enviroment of course."})
keywordHandler:addKeyword({'thais'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Thais is fa away and that is a good thing."})
keywordHandler:addKeyword({'venore'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I don't appreciate that my people buy that much human wares but there is little I can do about it."})
keywordHandler:addKeyword({'roderick'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He is the thaian ambassador."})
keywordHandler:addKeyword({'olrik'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He is the assistant of the thaian ambassador."})
keywordHandler:addKeyword({'king'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I know nothing about kings."})
keywordHandler:addKeyword({'elves'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The elves are split in three castes: the Deraisim, the Kuridai, and the Cenath."})
keywordHandler:addKeyword({'dwarf'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Funny little people sometimes. But their tunnels are harmful to the enviroment."})
keywordHandler:addKeyword({'human'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I don't like their stone cities and their stench."})
keywordHandler:addKeyword({'troll'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I still can't stand the thought of the Kuridai keeping these disgusting creatures in our settlement."})
keywordHandler:addKeyword({'army'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I don't understand what you mean."})
keywordHandler:addKeyword({'cenath'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "They think they are so wise but they lost the ability to adore the simple things."})
keywordHandler:addKeyword({'kuridai'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "They are paranoid, what makes them so aggressive."})
keywordHandler:addKeyword({'deraisim'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "We only stay here to keep our people together. We hunt for them, provide them with food, and scout the area."})
keywordHandler:addKeyword({'abdaisim'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Our lost brothers and sisters. Oh, we miss them so much."})
keywordHandler:addKeyword({'teshial'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I can only hope they will return one day."})
keywordHandler:addKeyword({'ferumbras'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I hope this fallen servant of evil will never find us."})
keywordHandler:addKeyword({'crunor'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I praise the Great Tree and Mother Earth, who gave birth to all life, and Nera, the celestial paladin."})
keywordHandler:addKeyword({'nera'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The lady of spring, helper of Crunor and Earth itself."})
keywordHandler:addKeyword({'excalibug'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "What is that?"})
keywordHandler:addKeyword({'new'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Sorry, the only news I have concern the growth of plants and the coming and going of animal herds."})
keywordHandler:addKeyword({'wand'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I'm sorry, I don't have any left. but think the magic shop got some."})
keywordHandler:addKeyword({'rod'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I'm sorry, I don't have any left. but think the magic shop got some."})
keywordHandler:addKeyword({'druid'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Druids are very close to Crunor."})
keywordHandler:addKeyword({'sorcerer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "They are so ... destructive."})
keywordHandler:addKeyword({'spellbook'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Sorry, I have none on me."})


local spellSellModule = SpellSellModule:new()
npcHandler:addModule(spellSellModule)

spellSellModule.condition = function(cid) return isDruid(cid) or isPaladin(cid) end
spellSellModule.conditionFailText = "Sorry, I only teach paladins and druids!"
spellSellModule:addSpellStock({
	"Find Person",
	"Great Light",
	"Light",
	"Food",
})

npcHandler:addModule(FocusModule:new())