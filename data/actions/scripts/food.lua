local MAX_FOOD = 120

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if (FOODS[item.itemid] == nil) then
		return false
	end

	local size = math.ceil(FOODS[item.itemid][1]/15)
	local sound = FOODS[item.itemid][2]

	if (getPlayerFood(cid) > MAX_FOOD) then
		doPlayerSendCancel(cid, "You are full.")
		return true
	end
	
	doPlayerFeed(cid, size)
	doRemoveItem(item.uid, 1)
	doPlayerSay(cid, sound, TALKTYPE_ORANGE)
	return true
end