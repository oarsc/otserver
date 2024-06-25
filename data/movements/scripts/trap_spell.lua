local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_THRUSTDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_HITAREA)
setCombatParam(combat, COMBAT_PARAM_BLOCKARMOR, 1)

function onGetFormulaValues(cid, level, maglevel, vocation)
	local base = 100
	local variation = 20

	local min = math.max(base - variation, (3 * maglevel + 3 * level) * (base - variation) / 100)
	local max = math.max(base + variation, (3 * maglevel + 3 * level) * (base + variation) / 100)

	return -min, -max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onStepIn(cid, item, casterPosition, position, var)
	if cid == var.number then
		doRemoveItem(item.uid)
		doPlayerAddMana(cid, 50)
		doSendMagicEffect(casterPosition, CONST_ME_MAGIC_BLUE)
		return true

	elseif not getTilePzInfo(casterPosition) and not getTilePzInfo(position) then
		doTransformItem(item.uid, 5108)
		doPlayerAddMana(cid, 40)
		doDecayItem(item.uid)
		return doCombat(cid, combat, var)
	end
	return false
end
