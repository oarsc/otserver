dofile(getDataDir() .. 'npc/scripts/lib/greeting.lua')

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

-- OTServ event handling functions
function onCreatureAppear(cid)         npcHandler:onCreatureAppear(cid)         end
function onCreatureDisappear(cid)      npcHandler:onCreatureDisappear(cid)      end
function onCreatureSay(cid, type, msg) npcHandler:onCreatureSay(cid, type, msg) end
function onThink()                     npcHandler:onThink()                     end

function greetCallback(cid)
	if isPremium(cid) then
		npcHandler:setMessage(MESSAGE_GREET,'Hello, hello, '.. getPlayerName(cid) ..'! Please come in, look, and buy!')
		return true
	else
		npcHandler:playerSay(cid, 'I\'m sorry '.. getPlayerName(cid) ..', but I only serve premium account customers.')
		return false
	end
end

npcHandler:setCallback(CALLBACK_GREET, greetCallback)

local shopModule = ShopModule:new()
npcHandler:addModule(shopModule)

shopModule:addSellableItem({'dagger'}, 2379, 2)
shopModule:addSellableItem({'spear'}, 2389, 3)
shopModule:addSellableItem({'hand axe'}, 2380, 4)
shopModule:addSellableItem({'rapier'}, 2384, 5)
shopModule:addSellableItem({'axe'}, 2386, 7)
shopModule:addSellableItem({'hatchet'}, 2388, 25)
shopModule:addSellableItem({'sabre'}, 2385, 12)
shopModule:addSellableItem({'short sword'}, 2406, 10)
shopModule:addSellableItem({'sword'}, 2376, 25)
shopModule:addSellableItem({'mace'}, 2398, 30)
shopModule:addSellableItem({'doublet'}, 2485, 3)
shopModule:addSellableItem({'leather armor'}, 2467, 5)
shopModule:addSellableItem({'chain armor'}, 2464, 40)
shopModule:addSellableItem({'brass armor'}, 2465, 150)
shopModule:addSellableItem({'leather helmet'}, 2461, 3)
shopModule:addSellableItem({'chain helmet'}, 2458, 12)
shopModule:addSellableItem({'studded helmet'}, 2482, 20)
shopModule:addSellableItem({'wooden shield'}, 2512, 3)
shopModule:addSellableItem({'studded shield'}, 2526, 16)
shopModule:addSellableItem({'brass shield'}, 2511, 25)
shopModule:addSellableItem({'plate shield'}, 2510, 40)
shopModule:addSellableItem({'copper shield'}, 2530, 50)
shopModule:addSellableItem({'leather boots'}, 2643, 2)
shopModule:addSellableItem({'rope'}, 2120, 8)

shopModule:addBuyableItem({'spear'}, 2389, 10)
shopModule:addBuyableItem({'rapier'}, 2384, 15)
shopModule:addBuyableItem({'sabre'}, 2385, 25)
shopModule:addBuyableItem({'dagger'}, 2379, 5)
shopModule:addBuyableItem({'sickle'}, 2405, 8)
shopModule:addBuyableItem({'hand axe'}, 2380, 8)
shopModule:addBuyableItem({'axe'}, 2386, 20)
shopModule:addBuyableItem({'short sword'}, 2406, 30)
shopModule:addBuyableItem({'jacket'}, 2650, 10)
shopModule:addBuyableItem({'coat'}, 2651, 8)
shopModule:addBuyableItem({'doublet'}, 2485, 16)
shopModule:addBuyableItem({'leather armor'}, 2467, 25)
shopModule:addBuyableItem({'leather legs'}, 2649, 10)
shopModule:addBuyableItem({'leather helmet'}, 2461, 12)
shopModule:addBuyableItem({'studded helmet'}, 2482, 63)
shopModule:addBuyableItem({'chain helmet'}, 2458, 52)
shopModule:addBuyableItem({'wooden shield'}, 2512, 15)
shopModule:addBuyableItem({'studded shield'}, 2526, 50)
shopModule:addBuyableItem({'torch'}, 2050, 2)
shopModule:addBuyableItem({'bag'}, 1987, 4)
shopModule:addBuyableItem({'scroll'}, 1949, 5)
shopModule:addBuyableItem({'shovel'}, 2554, 10)
shopModule:addBuyableItem({'backpack'}, 1988, 10)
shopModule:addBuyableItem({'scythe'}, 2550, 50)
shopModule:addBuyableItem({'rope'}, 2120, 50)
shopModule:addBuyableItem({'fishing rod'}, 2580, 150)

keywordHandler:addKeyword({'how are you'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am fine. I'm so glad to have you here as my customer."})
keywordHandler:addKeyword({'sell'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell much. Just read the blackboards for my awesome wares or just ask me."})
keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am a merchant. What can I do for you?"})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "My name is Norma. Do you want to buy something?"})
keywordHandler:addKeyword({'help'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell stuff to prices that low, that all other merchants would mock at my stupidity."})
keywordHandler:addKeyword({'monster'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "If you want to challenge the monsters, you need some weapons and armor I sell. You need them definitely!"})
keywordHandler:addKeyword({'dungeon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "If you want to explore the dungeons, you have to equip yourself with the vital stuff I am selling. It's vital in the deepest sense of the word."})
keywordHandler:addKeyword({'sewer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Oh, our sewer system is very primitive; so primitive it's overrun by rats. But the stuff I sell is safe from them. Do you want to buy some of it?"})
keywordHandler:addKeyword({'king'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The king encouraged salesmen to travel here, but only some dared to take the risk, and a risk it was!"})
keywordHandler:addKeyword({'dallheim'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Some call him a hero."})
keywordHandler:addKeyword({'bug'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Bugs plague this isle, but my wares are bugfree, totally bugfree."})
keywordHandler:addKeyword({'stuff'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell equipment of all kinds, all kind available on this isle. Just ask me about my wares if you are interested."})
keywordHandler:addKeyword({'tibia'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "One day I will return to the continent as a rich, a very rich woman!"})
keywordHandler:addKeyword({'thais'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Thais is a crowded town."})
keywordHandler:addKeyword({'wares'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell weapons, shields, armor, helmets, and equipment. For what do you want to ask?"})
keywordHandler:addKeyword({'offer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell weapons, shields, armor, helmets, and equipment. For what do you want to ask?"})
keywordHandler:addKeyword({'weapon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell spears, rapiers, sabres, daggers, hand axes, axes, and short swords. Just tell me what you want to buy."})
keywordHandler:addKeyword({'armor'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell jackets, coats, doublets, leather armor, and leather legs. Just tell me what you want to buy."})
keywordHandler:addKeyword({'helmet'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell leather helmets, studded helmets, and chain helmets. Just tell me what you want to buy."})
keywordHandler:addKeyword({'shield'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell wooden shields and studded shields. Just tell me what you want to buy."})
keywordHandler:addKeyword({'equipment'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell torches, bags, scrolls, shovels, picks, backpacks, sickles, scythes, ropes, fishing rods and sixpacks of worms. Just tell me what you want to buy."})
keywordHandler:addKeyword({'do you sell'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "What do you need? I sell weapons, armor, helmets, shields, and equipment."})
keywordHandler:addKeyword({'do you have'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "What do you need? I sell weapons, armor, helmets, shields, and equipment."})
keywordHandler:addKeyword({'pick'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Sorry, I fear Al Dee owns the last ones on this isle."})
keywordHandler:addKeyword({'club'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I don't buy this garbage!"})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is about |TIME|. I am so sorry, I have no watches to sell. Do you want to buy something else?"})

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())

function onCreatureSell(...)
	shopModule:sellItemToPlayer(unpack(arg))
end