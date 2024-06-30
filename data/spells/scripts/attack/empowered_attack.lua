local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_STRIKEDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_HITAREA)
setCombatParam(combat, COMBAT_PARAM_BLOCKARMOR, 1)

function onGetFormulaValues(cid, level, attackSkill, attackValue, attackFactor)
	local min = math.floor((attackSkill*0.56 + attackValue*0.79 + level*0.45) / attackFactor)
	local max = math.ceil ((attackSkill*0.79 + attackValue*0.71 + level*0.71) / attackFactor)
	return -min, -max
end

setCombatCallback(combat, CALLBACK_PARAM_SKILLVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end