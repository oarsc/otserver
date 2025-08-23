function onUse(cid, item, fromPosition, itemEx, toPosition)

	local result = setCardToItem(item.uid, itemEx.uid);

	if result then
		doRemoveItem(item.uid, 1)
		doSendMagicEffect(toPosition, CONST_ME_MAGIC_RED)
		return true

	else
		doPlayerSendCancel(cid, "You cannot put the card there.")
		return false
	end

	return true
end