//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "attack.h"
#include "cards.h"
#include "../game.h"
#include "../const.h"
#include "../tools.h"

extern Game g_game;
extern Cards g_cards;

void Attack::playerAutoattack(
	Player* player,
	Creature* target,
	const Weapon* weapon,
	int32_t damage,
	const CombatParams &params
) {
	if (player == target || Combat::canDoCombat(player, target) != RET_NOERROR) return;

	if (params.combatType != COMBAT_NONE && params.combatType != COMBAT_MANADRAIN && params.combatType != COMBAT_HEALING) {

		int32_t reducedDamage;
		if (dynamic_cast<const WeaponDistance*>(weapon)) {
			reducedDamage = -random_range(0, -damage, DISTRO_CUSTOM_ATTACK, 0.85, 0.05);
		} else {
			reducedDamage = -random_range(0, -damage, DISTRO_CUSTOM_ATTACK, 0.9, 0);
		}

		CombatParams paramsWithDamage = params;
		paramsWithDamage.baseDamage = reducedDamage;
		paramsWithDamage.damage = reducedDamage;

		g_cards.applyDoDamageFromCards(player, paramsWithDamage);

		performAttack(player, target, paramsWithDamage,
			[=] {
				player->useActiveSpellAttack(target);
				g_cards.applyPostAttackFromCards(player, paramsWithDamage);
			});

	} else {
		std::cout << "Wrong player attack(name=" << player->getName() << ",combatType=" << params.combatType << ")" << std::endl;
	}
}

void Attack::monsterAutoattack(
	Monster* monster,
	Creature* target,
	Combat* combat
) {
	if (monster == target || Combat::canDoCombat(monster, target) != RET_NOERROR) return;

	CombatParams params = combat->params;

	if(params.combatType != COMBAT_NONE && params.combatType != COMBAT_MANADRAIN && params.combatType != COMBAT_HEALING){
		int32_t minChange = 0;
		int32_t maxChange = 0;

		combat->getMinMaxValues(monster, target, minChange, maxChange);
		int32_t damage = random_range(minChange, maxChange);

		CombatParams paramsWithDamage = params;
		paramsWithDamage.baseDamage = damage;
		paramsWithDamage.damage = damage;

		performAttack(monster, target, paramsWithDamage);

	} else {
		std::cout << "Wrong monster attack(name=" << monster->getName() << ",combatType=" << params.combatType << ")" << std::endl;
	}
}

void Attack::performAttack(
	Creature* attacker,
	Creature* target,
	CombatParams& params,
	const std::function<void(void)>& callback /* = nullptr */,
	const std::function<bool(CombatParams&)>& confirmation /* = nullptr */
) {
	// check card absorbs, blocking, armor and monster resistances
	if (g_game.combatBlockHit(params, attacker, target)) {
		if (params.distanceEffect != NM_ME_NONE) {
			g_game.addDistanceEffect(attacker->getPosition(), target->getPosition(), params.distanceEffect);
		}
		return;
	}

	Combat::checkPVPDamageReduction(attacker, target, params);

	if (confirmation && !confirmation(params)) {
		return;
	}

	int32_t damage = params.damage;

	if (params.distanceEffect == NM_ME_NONE) {
		attack(attacker, target, damage, params);
		if (callback) callback();

	} else {
		g_game.addDistanceEffect(attacker->getPosition(), target->getPosition(), params.distanceEffect);

		g_scheduler.addEvent(createSchedulerTask(100, [=] {
			attack(attacker, target, damage, params);
			if (callback) callback();
		}));
	}
}

void Attack::attack(
	Creature* attacker,
	Creature* target,
	int32_t damage,
	const CombatParams& params
) {
	int32_t damageSum = params.damage;
	for (const ExtraCombatParams &extra : params.extras) {
		damageSum += extra.damage;
	}

	bool result = g_game.combatChangeHealth(params.combatType, params.hitEffect, params.hitTextColor, attacker, target, damageSum);

	if (result) {
		Combat::CombatConditionFunc(attacker, target, params, NULL);
		Combat::CombatDispelFunc(attacker, target, params, NULL);
	}

	if (params.impactEffect != NM_ME_NONE) {
		g_game.addMagicEffect(target->getPosition(), params.impactEffect);
	}
}

bool Attack::performCombatScript(
	LuaScriptInterface* scriptInterface,
	int32_t &scriptId,
	Creature* attacker,
	const Creature* target,
	Item* item /* = nullptr */
) {
	//luaFunction(cid, item, attackerPosition, position, var)

	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = target->getID();

	if (scriptInterface->reserveScriptEnv()) {
		ScriptEnviroment* env = scriptInterface->getScriptEnv();
		LuaVariant var;
		var.type = VARIANT_NUMBER;
		var.number = target->getID();

		env->setScriptId(scriptId, scriptInterface);
		env->setRealPos(attacker->getPosition());

		uint32_t cid = env->addThing(attacker);
		uint32_t itemid = 0;
		if (item) {
			itemid = env->addThing(item);
		}

		lua_State* L = scriptInterface->getLuaState();

		scriptInterface->pushFunction(scriptId);
		lua_pushnumber(L, cid);
		LuaScriptInterface::pushThing(L, item, itemid);
		LuaScriptInterface::pushPosition(L, attacker->getPosition(), 0);
		LuaScriptInterface::pushPosition(L, target->getPosition(), 0);
		scriptInterface->pushVariant(L, var);

		bool result = scriptInterface->callFunction(5);
		scriptInterface->releaseScriptEnv();
		return result;

	} else {
		std::cout << "[Error] Call stack overflow. Attack::performCombatScript" << std::endl;
	}

	return false;
}
