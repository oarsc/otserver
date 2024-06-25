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

#ifndef __OTSERV_ATTACK_H__
#define __OTSERV_ATTACK_H__

#include "../weapons.h"
#include <functional>

class Attack {
public:
    static void playerAutoattack(
        Player* player, 
        Creature* target, 
        const Weapon* weapon, 
        int32_t damage, 
        const CombatParams& params
    );

    static void monsterAutoattack(
        Monster* monster, 
        Creature* target, 
        Combat* combat
    );

    static void performAttack(
        Creature* attacker, 
        Creature* target, 
        int32_t damage, 
        const CombatParams& params, 
        const std::function<void(void)>& callback = nullptr, 
        const std::function<bool(int32_t&)>& confirmation = nullptr
    );

    static bool performCombatScript(
        LuaScriptInterface* scriptInterface,
        int32_t &scriptId,
        Creature* attacker,
        const Creature* target,
        Item* item = nullptr
    );

private:
    static void attack(
        Creature* attacker, 
        Creature* target, 
        int32_t damage, 
        const CombatParams& params
    );
};

#endif
