local healthCombat = createCombatObject()
setCombatParam(healthCombat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(healthCombat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_RED)
setCombatParam(healthCombat, COMBAT_PARAM_AGGRESSIVE, false)
setCombatParam(healthCombat, COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)

function onGetLifeFluidFormulaValues(cid, level, maglevel)
	return 180, 275
end

setCombatCallback(healthCombat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetLifeFluidFormulaValues")


function onUse(cid, item, fromPosition, itemEx, toPosition)

	if item.itemid == 5098 then
		if not doCombat(cid, healthCombat, numberToVariant(cid)) then
			return false
		end

	elseif item.itemid == 5099 then
		if not doPlayerAddMana(cid, math.random(180, 275)) then
			return false
		end
		doSendMagicEffect(toPosition, CONST_ME_MAGIC_BLUE)

	end

	doRemoveItem(item.uid, 1)
	doPlayerSay(cid, "Aaaah...", TALKTYPE_ORANGE)
	return true
end