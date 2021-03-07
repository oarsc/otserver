dofile(getDataDir() .. 'npc/scripts/lib/greeting.lua')

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

-- OTServ event handling functions
function onCreatureAppear(cid)			npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)		npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)		end
function onThink()				npcHandler:onThink()					end

local shopModule = ShopModule:new()
npcHandler:addModule(shopModule)

shopModule:addSellableItem({'two handed sword'}, 2377, 450)
shopModule:addSellableItem({'battle axe'}, 2378, 80)
shopModule:addSellableItem({'dagger'}, 2379, 2)
shopModule:addSellableItem({'hand axe'}, 2380, 4)
shopModule:addSellableItem({'halberd'}, 2381, 400)
shopModule:addSellableItem({'rapier'}, 2384, 5)
shopModule:addSellableItem({'sabre'}, 2385, 12)
shopModule:addSellableItem({'axe'}, 2386, 7)
shopModule:addSellableItem({'spear'}, 2389, 3)
shopModule:addSellableItem({'morning star'}, 2394, 90)
shopModule:addSellableItem({'mace'}, 2398, 30)
shopModule:addSellableItem({'short sword'}, 2406, 10)
shopModule:addSellableItem({'battle hammer'}, 2417, 120)
shopModule:addSellableItem({'sword'}, 2376, 25)
shopModule:addBuyableItem({'dagger'}, 2379, 5)
shopModule:addBuyableItem({'hand axe'}, 2380, 8)
shopModule:addBuyableItem({'royal spear'}, 5092, 25)
shopModule:addBuyableItem({'spear'}, 2389, 10)
shopModule:addBuyableItem({'rapier'}, 2384, 15)
shopModule:addBuyableItem({'axe'}, 2386, 20)
shopModule:addBuyableItem({'sabre'}, 2385, 35)
shopModule:addBuyableItem({'sword'}, 2376, 85)
shopModule:addBuyableItem({'mace'}, 2398, 90)
shopModule:addBuyableItem({'battle hammer'}, 2417, 350)
shopModule:addBuyableItem({'machete'}, 2420, 35)
shopModule:addBuyableItem({'throwing knife'}, 2410, 25)
 
keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am Romella, and I will be serving you today."})
keywordHandler:addKeyword({'shop'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am Romella, and I will be serving you today."})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am Romella, and I will be serving you today."})
keywordHandler:addKeyword({'king'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The only royal thing we feel here is the royal tax."})
keywordHandler:addKeyword({'tibianus'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The only royal thing we feel here is the royal tax."})
keywordHandler:addKeyword({'army'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Our warehouse is the main supplier of the local garrison."})
keywordHandler:addKeyword({'ferumbras'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Make sure to buy some extra weapons before facing that one."})
keywordHandler:addKeyword({'excalibug'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I heard the amazons are after it."})
keywordHandler:addKeyword({'new'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It says the amazons are looking for a certain magical weapon in this area."})
keywordHandler:addKeyword({'amazon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I wonder how they finance themselves. I bet they are secretly trading in some strange stuff."})
keywordHandler:addKeyword({'help'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "The weapons we sell are all help you need."})
keywordHandler:addKeyword({'monster'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Just buy enough weapons and you don't have to fear them."})
keywordHandler:addKeyword({'swamp'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Don't go exploring without weapons. Especially you'll need a machete."})
keywordHandler:addKeyword({'thank'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "You are welcome."})
keywordHandler:addKeyword({'offer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell several weapons."})
keywordHandler:addKeyword({'sell'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell several weapons."})
keywordHandler:addKeyword({'have'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I sell several weapons."})
keywordHandler:addKeyword({'weapon'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I have hand axes, axes, spears, maces, battle hammers, swords, rapiers, daggers, sabres, and machetes. What's your choice?"})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is |TIME|."})

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())

function onCreatureSell(...)
	shopModule:sellItemToPlayer(unpack(arg))
end