local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_RED)
setCombatParam(combat, COMBAT_PARAM_CREATEITEM, 5107)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 1)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end