local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HOLYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_HITBYHOLY)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_HOLY)

function onGetFormulaValues(cid, level, maglevel)
	local base = 45
	local variation = 10

	local min = math.max((base - variation), ((3 * maglevel + 2 * level) * (base - variation) / 100))
	local max = math.max((base + variation), ((3 * maglevel + 2 * level) * (base + variation) / 100))

	return -min, -max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end