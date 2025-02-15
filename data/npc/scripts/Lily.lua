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

keywordHandler:addKeyword({'how are you'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Very well. Thank you."})
keywordHandler:addKeyword({'offer'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I'll be happy to buy some blueberries from you."})
keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I am a druid, bound to the spirit of nature. Oh, and I buy blueberries, of course."})
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "My name is Lily."})
keywordHandler:addKeyword({'hyacinth'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Hyacinth lives in the forest. He's never in town so I don't know him very well."})
keywordHandler:addKeyword({'monster'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Many monsters are poisonous. Be aware."})
keywordHandler:addKeyword({'creature'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Many monsters are poisonous. Be aware."})
keywordHandler:addKeyword({'poison'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "Many monsters are poisonous. Be aware."})
keywordHandler:addKeyword({'life fluid'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "I'm sorry, but Hyacinth is the only one on Rookgaard who knows how to brew life fluids."})
keywordHandler:addKeyword({'time'}, StdModule.say, {npcHandler = npcHandler, onlyFocus = true, text = "It is about |TIME|."})

function creatureSayCallback(cid, type, msg)
	if not npcHandler:hasFocus(cid) then
		return false
	end
	msg = string.lower(msg)
	local cidData = npcHandler:getFocusPlayerData(cid)

	if msgcontains(msg, 'blueberry') or msgcontains(msg, 'blueberries') then
		npcHandler:playerSay(cid, "Do you want to sell 5 blueberries for 1 gold?", 1)
		cidData.state = 301

	elseif cidData.state == 301 and msgcontains(msg, 'yes') then
		if doPlayerRemoveItem(cid, 2677, 5) == true then
			doPlayerAddMoney(cid, 1)
			cidData.state = 0
		else
			npcHandler:playerSay(cid, "Oh, I'm sorry. I'm not buying less than 5 blueberries.", 1)
		end
	elseif cidData.state == 301 and msgcontains(msg, '08') then
		npcHandler:playerSay(cid, "As you wish.", 1)

	end
    return true
end


npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())