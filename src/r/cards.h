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

#ifndef __OTSERV_CARDS_H__
#define __OTSERV_CARDS_H__

#include "../const.h"
#include "../definitions.h"
#include "../combat.h"
#include "../enums.h"
#include "../itemloader.h"
#include "../items.h"
#include "../position.h"
#include <cstdint>
#include <map>

enum CardAttachment_t {
	CARD_ATTACHMENT_WEAPON = 0,
	CARD_ATTACHMENT_MELEE,
	CARD_ATTACHMENT_DISTANCE,
	CARD_ATTACHMENT_WAND,
	CARD_ATTACHMENT_SHIELD,
	CARD_ATTACHMENT_HELMET,
	CARD_ATTACHMENT_ANY_ARMOR,
	CARD_ATTACHMENT_CHEST,
	CARD_ATTACHMENT_LEGS,
	CARD_ATTACHMENT_BOOTS,
	CARD_ATTACHMENT_RING,
	CARD_ATTACHMENT_NECKLACE,
	CARD_ATTACHMENT_LAST
};

enum CardEffect_t {
	CARD_EFFECT_DAMAGE_EDIT = 0,
	CARD_EFFECT_DAMAGE_ADD,
	CARD_EFFECT_STEAL,
	CARD_EFFECT_ABSORB,
	CARD_EFFECT_ADD,
	CARD_EFFECT_COUNTER,
	CARD_EFFECT_NO_AMMO,
};

class CardEffect;

class Card {
private:
	//It is private because calling it can cause unexpected results
	Card(const Card& it){};

public:
	Card();
	~Card() = default;

	CardAttachment_t attachment;
	uint32_t id;
	uint32_t itemId;
	std::string description;
	std::string article;
	std::string name;
	std::vector<CardEffect*> effects;
};


class Cards {
public:
	Cards() = default;
	~Cards() = default;

	const Card* getCardById(uint32_t id) const;
	bool loadFromXml(const std::string& datadir);

	void applyDoDamageFromCards(const Player* player, CombatParams &params) const;
	void applyReceiveDamageFromCards(const Player* player, CombatParams &params) const;
	void applyReceiveDamageFromCards(const Player* player, const CombatType_t &combatType, int &damage) const;
	void applyPostAttackFromCards(Player* player, const CombatParams &params) const;
	bool applyVarSkillsFromCards(Player* player) const;
	int getExtraPropertyValueFromCards(const Item* item) const;
	int getExtraSpeedFromCards(const Player* player) const;
	bool shouldConsumeAmmoFromCards(const Player* player, const Item* ammoItem) const;

private:
	template <typename Callback>
	void loopCardEffects(const Player* player, Callback&& consumer) const;

protected:
	typedef std::map<uint32_t, Card*> CardMap;
	CardMap cards;

	std::string m_datadir;
};




class CardEffect {
public:
	virtual ~CardEffect() = default;
	virtual CardEffect_t getType() const = 0;

	static CardEffect* loadFromXml(const xmlNodePtr &effectNode);

	virtual void applyAddDamage(CombatParams &combat) const {}
	virtual void applyReduceDamage(CombatParams &combat) const {}
	virtual void applyReduceDamage(const CombatType_t &combatType, int &damage) const {}
	virtual void applyPostAttack(Player* player, const CombatParams &params) const {}
	virtual void applySkill(Player* player) const {}
	virtual void applyPropertyValue(const Item* item, int &value) const {}
	virtual void applySpeed(const int currentSpeed, int &value) const {}
	virtual bool applyNoAmmo(const Player* player, const Item* ammoItem) const { return false; };

protected:
	bool success(const uint8_t &chance) const;
};


class CardEffectAttackModify : public CardEffect
{
public:
	bool percent = false;
	int value = 0;
	uint8_t chance = 100;
	CombatType_t type = COMBAT_NONE;

	CardEffect_t getType() const override { return CARD_EFFECT_DAMAGE_EDIT; }

	void applyAddDamage(CombatParams &combat) const override;
};


class CardEffectDamage : public CardEffect
{
public:
	bool percent = false;
	int value = 0;
	uint8_t chance = 100;
	CombatType_t type;
	int8_t blocked = -1;

	CardEffect_t getType() const override { return CARD_EFFECT_DAMAGE_ADD; }

	void applyAddDamage(CombatParams &combat) const override;
};

class CardEffectSteal : public CardEffect
{
public:
	bool percent = false;
	int value = 0;
	uint8_t chance = 100;
	bool health = true;

	CardEffect_t getType() const override { return CARD_EFFECT_STEAL; }

	void applyPostAttack(Player* player, const CombatParams &params) const override;
};

class CardEffectAbsorb : public CardEffect
{
public:
	bool percent = false;
	int value = 0;
	uint8_t chance = 100;
	CombatType_t type;

	CardEffect_t getType() const override { return CARD_EFFECT_ABSORB; }

	void applyReduceDamage(CombatParams &combat) const override;
	void applyReduceDamage(const CombatType_t &combatType, int &damage) const override;
};

class CardEffectAdd : public CardEffect
{
public:
	bool percent = false;
	int16_t armor = 0;
	int16_t speed = 0;
	int16_t attackOrDefense = 0;
	int16_t skillValue = 0;
	skills_t skill = SKILL_FIST;

	CardEffect_t getType() const override { return CARD_EFFECT_ADD; }

	void applySkill(Player* player) const override;
	void applyPropertyValue(const Item* item, int &value) const override;
	void applySpeed(const int currentSpeed, int &value) const override;
};

class CardEffectNoAmmo : public CardEffect
{
public:
	uint8_t chance = 100;
	std::vector<uint16_t> ammos;

	CardEffect_t getType() const override { return CARD_EFFECT_NO_AMMO; }

	bool applyNoAmmo(const Player* player, const Item* ammoItem) const override;
};


#endif
