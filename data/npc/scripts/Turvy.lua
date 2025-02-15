dofile(getDataDir() .. 'npc/scripts/lib/greeting.lua')

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)



-- OTServ event handling functions
function onCreatureAppear(cid)         npcHandler:onCreatureAppear(cid)         end
function onCreatureDisappear(cid)      npcHandler:onCreatureDisappear(cid)      end
function onCreatureSay(cid, type, msg) npcHandler:onCreatureSay(cid, type, msg) end
function onThink()                     npcHandler:onThink()                     end

local shopModule = ShopModule:new()
npcHandler:addModule(shopModule)

NpcPreset:mainWeaponShop(shopModule)
NpcPreset:mainArmorShop(shopModule)

NpcPreset:altWeaponShop(shopModule)
NpcPreset:altArmorShop(shopModule)

keywordHandler:addKeyword({'how are you'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I'm just fine and dandy, thank you for asking."})
keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is an absolute honour to provide weaponry and armor to the courageous adventurers of Thais, just so long as you have the gold to pay for it."})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Some call me Turvy. Actually, everybody calls me Turvy."})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is nearly time for my afternoon nap, so please hurry!"})
keywordHandler:addKeyword({'help'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Help to self-help - that is my motto."})
keywordHandler:addKeyword({'monster'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "There is a monster here? HERE?! Time to double the prices!"})
keywordHandler:addKeyword({'dungeon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "If you want to see dungeons go and insult the guards.  On second thoughts - don't do that."})
keywordHandler:addKeyword({'sewer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is very effective, but attracts almost as many wannabe heroes as it does rats."})
keywordHandler:addKeyword({'assistant'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am not a mere assistant! I have a job of great responsibility! But mostly I keep annoying personages away from my boss."})
keywordHandler:addKeyword({'annoying'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Oh gosh - I could tell you some stories. But I won't."})
keywordHandler:addKeyword({'thank'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "So polite . . . bless you!"})
keywordHandler:addKeyword({'god'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The Gods of Tibia play games with the fate of Tibians - but they haven't bothered to read the instructions."})
keywordHandler:addKeyword({'king'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Ah, yes, yes, hail to King Tibianus! May he in his infinite wisdom reduce my taxes... and so on..."})
keywordHandler:addKeyword({'sam'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "A simple shopkeeper, who was last in the queue when they were handing out intelligence."})
keywordHandler:addKeyword({'benjamin'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Ah, such a shame about poor Benjamin. Lost it a bit after receiving one too many blows to the head."})
keywordHandler:addKeyword({'gorn'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "He does a good line in second-rate scrolls for first-rate prices."})
keywordHandler:addKeyword({'quentin'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "You can't tech an old monk new tricks. He is stubborn to the extreme and overly concerned about Thais. He should care more about his gods and less about that king."})
keywordHandler:addKeyword({'bozo'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Bozo - such a tragic story. If only I could remember it."})
keywordHandler:addKeyword({'weapon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The word on the street is that Sam does not forge all his weapons himself, but buys them from his cousin, who is married to a cyclops."})
keywordHandler:addKeyword({'magic'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Magic is a thing of the past. Why bother with a colourful bit of rock and a few fancy words when you can have a foot of razor-sharp steel in your hand?!"})
keywordHandler:addKeyword({'power'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "There are people who talk about a rebellion against King Tibianus."})
keywordHandler:addKeyword({'rebellion'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Well, Venore is richer than Thais, and some people want to live in a democracy free from an oppressive tyrant - I mean monarch. I'm not one of them."})
keywordHandler:addKeyword({'spell'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Spells - dodgy mumbo jumbo if you ask me. A sword never backfires on its user!"})
keywordHandler:addKeyword({'elane'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "A true tragedy - she has lost so many husbands in such unusual circumstances."})
keywordHandler:addKeyword({'venore'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Ah... Venore - a wonderful city! Full of culture! So many friendly faces! So unlike Thais!"})
keywordHandler:addKeyword({'thais'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Thais is OK - I suppose. Not as nice as Venore, but good for business."})
keywordHandler:addKeyword({'carlin'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Those women really know how to run things - look at how well the trade is going there!"})
keywordHandler:addKeyword({'kazordoon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "You need to shrink before you go there - they say the dwarves aren't too keen on sharing their mountain with us Tibians."})
keywordHandler:addKeyword({'dwarf'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I don't know much about them - there are some civilised dwarves, of course, but I can never tell whether they are male or female."})
keywordHandler:addKeyword({"Ab'dendriel"}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Aah... a beautiful leafy city. Shame about the elves."})
keywordHandler:addKeyword({'elves'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Elves are good with a bow and arrow, or so I am told. Shame that they are no good at peace-making."})
keywordHandler:addKeyword({'chester'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I have never heard any rumours concerning him, isn't that odd?"})
keywordHandler:addKeyword({'ardua'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Well - she isn't really my kind of person. Please don't mention her name again."})
keywordHandler:addKeyword({'partos'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Some thief they caught for all I know."})
keywordHandler:addKeyword({'gamel'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Some sinister guy that is. He's not allowed to enter that markethall and thats for a good reason."})
keywordHandler:addKeyword({'gamon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Shhh! He's a spy! He watches us all the time! Just keep smiling and he'll go away!"})
keywordHandler:addKeyword({'quest'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Hmmm yes. I think Topsy might have something for you."})
keywordHandler:addKeyword({'offer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "My offers are weapons, armors, helmets, legs, and shields."})
keywordHandler:addKeyword({'sell'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "My offers are weapons, armors, helmets, legs, and shields."})
keywordHandler:addKeyword({'have'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "My offers are weapons, armors, helmets, legs, and shields."})
keywordHandler:addKeyword({'weapon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I have hand axes, axes, spears, maces, battle hammers, swords, rapiers, daggers, and sabres. What's your choice?"})
keywordHandler:addKeyword({'helmet'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am selling leather helmets and chain helmets. What do you want?"})
keywordHandler:addKeyword({'armor'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am selling leather, chain, and brass armor. What do you need?"})
keywordHandler:addKeyword({'shield'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am selling wooden shields and steel shields. What do you want?"})
keywordHandler:addKeyword({'trousers'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am selling chain legs. Do you want to buy any?"})
keywordHandler:addKeyword({'legs'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am selling chain legs. Do you want to buy any?"})


function creatureSayCallback(cid, type, msg)
	if not npcHandler:hasFocus(cid) then
		return false
	end
	msg = string.lower(msg)
	local cidData = npcHandler:getFocusPlayerData(cid)

	if msgcontains(msg, 'rumo') or msgcontains(msg, 'gossip') or msgcontains(msg, 'new') then
		npcHandler:say("You know a rumour? Well then - don't keep it to yourself.", 1)
		cidData.state = 3

	elseif cidData.state == 3 and msgcontains(msg, '') then
		npcHandler:say("Go on! I can't wait to hear more!", 1)
		cidData.state = 0
	end
	return true
end


npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())

function onCreatureSell(...)
	shopModule:sellItemToPlayer(unpack(arg))
end