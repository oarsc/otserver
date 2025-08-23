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

#include "../tools.h"
#include "../player.h"
#include "cards.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <array>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

static bool getCardAttachment(const std::string &str, CardAttachment_t &attachment)
{
	const std::string strValue = asLowerCaseString(str);
	if (strValue == "weapon")         attachment = CARD_ATTACHMENT_WEAPON;
	else if (strValue == "melee")     attachment = CARD_ATTACHMENT_MELEE;
	else if (strValue == "distance")  attachment = CARD_ATTACHMENT_DISTANCE;
	else if (strValue == "wand")      attachment = CARD_ATTACHMENT_WAND;
	else if (strValue == "shield")    attachment = CARD_ATTACHMENT_SHIELD;
	else if (strValue == "any-armor") attachment = CARD_ATTACHMENT_ANY_ARMOR;
	else if (strValue == "helmet")    attachment = CARD_ATTACHMENT_HELMET;
	else if (strValue == "chest")     attachment = CARD_ATTACHMENT_CHEST;
	else if (strValue == "legs")      attachment = CARD_ATTACHMENT_LEGS;
	else if (strValue == "boots")     attachment = CARD_ATTACHMENT_BOOTS;
	else if (strValue == "ring")      attachment = CARD_ATTACHMENT_RING;
	else if (strValue == "necklace")  attachment = CARD_ATTACHMENT_NECKLACE;
	else return false;
	return true;
}

static bool getCombatType(const std::string &str, CombatType_t &combat)
{
	const std::string strValue = asLowerCaseString(str);
	if (strValue == "physical")    combat = COMBAT_PHYSICALDAMAGE;
	else if (strValue == "slash")  combat = COMBAT_SLASHDAMAGE;
	else if (strValue == "strike") combat = COMBAT_STRIKEDAMAGE;
	else if (strValue == "pierce") combat = COMBAT_PIERCEDAMAGE;
	else if (strValue == "poison") combat = COMBAT_POISONDAMAGE;
	else if (strValue == "energy") combat = COMBAT_ENERGYDAMAGE;
	else if (strValue == "earth")  combat = COMBAT_EARTHDAMAGE;
	else if (strValue == "fire")   combat = COMBAT_FIREDAMAGE;
	else if (strValue == "ice")    combat = COMBAT_ICEDAMAGE;
	else if (strValue == "death")  combat = COMBAT_DEATHDAMAGE;
	else if (strValue == "holy")   combat = COMBAT_HOLYDAMAGE;
	else return false;
	return true;
}

static bool getSkillType(const std::string &str, skills_t &skill)
{
	const std::string strValue = asLowerCaseString(str);
	if (strValue == "fist")          skill = SKILL_FIST;
	else if (strValue == "strike")   skill = SKILL_STRIKE;
	else if (strValue == "pierce")   skill = SKILL_PIERCE;
	else if (strValue == "slash")    skill = SKILL_SLASH;
	else if (strValue == "distance") skill = SKILL_DIST;
	else if (strValue == "shield")   skill = SKILL_SHIELD;
	else if (strValue == "fish")     skill = SKILL_FISH;
	else return false;
	return true;
}


Card::Card()
{
	attachment = CARD_ATTACHMENT_WEAPON;
	id = 0;
	itemId = 0;
	description = "";
	name = "";
	article = "";
}

const Card* Cards::getCardById(uint32_t id) const
{
	auto it = cards.find(id);
	return it != cards.end()
		? it->second
		: nullptr;
}

bool Cards::loadFromXml(const std::string& datadir)
{
	m_datadir = datadir;
	std::string filename = m_datadir + "/items/cards.xml";

	xmlDocPtr doc = xmlParseFile(filename.c_str());
	if(doc == NULL){
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);

	if(xmlStrcmp(root->name,(const xmlChar*)"cards") != 0){
		xmlFreeDoc(doc);
		return false;
	}

	int intValue;
	std::string strValue;
	uint32_t id = 0;
	Card* mCard = nullptr;

	std::map<uint16_t, bool> cardsLoaded;
	xmlNodePtr itemNode = root->children;
	while(itemNode){
		if (xmlStrcmp(itemNode->name,(const xmlChar*)"card") == 0) {
			if (readXMLInteger(itemNode, "id", intValue)) {
				id = intValue;

				if (cardsLoaded[id]){
					std::cout << "Warning: [Cards::loadFromXml]. Duplicate cardid \"" << intValue << "\"" << std::endl;
					itemNode = itemNode->next;
					continue;
				}

				if (!readXMLInteger(itemNode, "itemId", intValue)) {
					std::cout << "Warning: [Cards::loadFromXml]. Card definition requires an itemId" << std::endl;
					itemNode = itemNode->next;
					continue;
				}

				mCard = new Card();
				mCard->id = id;
				mCard->itemId = intValue;

				cardsLoaded[id] = true;
				cards[id] = mCard;

				if (readXMLString(itemNode, "name", strValue)) {
					mCard->name = strValue;
				}

				if (readXMLString(itemNode, "article", strValue)) {
					mCard->article = strValue;
				}

				xmlNodePtr itemAttributesNode = itemNode->children;
				while (itemAttributesNode) {
					if (xmlStrcmp(itemAttributesNode->name, (const xmlChar*)"description") == 0) {
						if (readXMLString(itemAttributesNode, "value", strValue)) {
							mCard->description = strValue;
						}

					} else if (xmlStrcmp(itemAttributesNode->name, (const xmlChar*)"attachment") == 0) {
						if (readXMLString(itemAttributesNode, "value", strValue)) {
							if (!getCardAttachment(strValue, mCard->attachment)) {
								std::cout << "Warning: [Cards::loadFromXml] Unknown card attachment value " << strValue << std::endl;
							}
						}

					} else if (xmlStrcmp(itemAttributesNode->name, (const xmlChar*)"effects") == 0) {
						xmlNodePtr effectNode = itemAttributesNode->children;
						while (effectNode) {

							if (effectNode->type != XML_ELEMENT_NODE) {
									effectNode = effectNode->next;
									continue;
							}

							if (CardEffect* effect = CardEffect::loadFromXml(effectNode)) {
								mCard->effects.push_back(effect);
							}
							effectNode = effectNode->next;
						}
					}

					itemAttributesNode = itemAttributesNode->next;
				}
			}
		}

		itemNode = itemNode->next;
	}

	return true;
}

template <typename Callback>
void Cards::loopCardEffects(const Player* player, Callback&& consumer) const
{
	constexpr static std::array<slots_t, 8> slots = {
			SLOT_HEAD, SLOT_NECKLACE, SLOT_ARMOR, SLOT_RIGHT,
			SLOT_LEFT, SLOT_LEGS, SLOT_FEET, SLOT_RING
	};
	bool cont = true;

	for (slots_t s : slots) {
    const Item* item = player->getInventoryItem(s);
    if (!item) continue;

    for (const uint32_t& id : item->getCardsInSlot()) {
			const Card* card = getCardById(id);
			if (!card) continue;

			for (const CardEffect* effect : card->effects) {
				consumer(effect, cont);
				if (!cont) break;
			}
			if (!cont) break;
    }
		if (!cont) break;
	}
}

void Cards::applyDoDamageFromCards(const Player* player, CombatParams &params) const
{
	loopCardEffects(player,
		[&params](const CardEffect* effect, bool &cont) { effect->applyAddDamage(params); });
}

void Cards::applyReceiveDamageFromCards(const Player* player, CombatParams &params) const
{
	loopCardEffects(player,
		[&params](const CardEffect* effect, bool &cont) { effect->applyReduceDamage(params); });
}

void Cards::applyReceiveDamageFromCards(const Player* player, const CombatType_t &combatType, int &damage) const
{
	loopCardEffects(player,
		[&combatType, &damage](const CardEffect* effect, bool &cont) { effect->applyReduceDamage(combatType, damage); });
}

void Cards::applyPostAttackFromCards(Player* player, const CombatParams &params) const
{
	loopCardEffects(player,
		[&player, &params](const CardEffect* effect, bool &cont) { effect->applyPostAttack(player, params); });
}

bool Cards::applyVarSkillsFromCards(Player* player) const
{
	int32_t previousCardSkills[SKILL_LAST + 1];
	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i){
		previousCardSkills[i] = player->getVarCardSkill((skills_t)i);
		player->setVarCardSkill((skills_t)i, 0);
	}

	loopCardEffects(player,
		[&player](const CardEffect* effect, bool &cont) { effect->applySkill(player); });

	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i){
		if (previousCardSkills[i] != player->getVarCardSkill((skills_t)i)) {
			return true;
		}
	}
	return false;
}

int Cards::getExtraPropertyValueFromCards(const Item* item) const
{
	if (!item) return 0;

	int value = 0;
	for (const uint32_t& id : item->getCardsInSlot()) {
		const Card* card = getCardById(id);
		if (!card) continue;

		for (const CardEffect* effect : card->effects) {
			effect->applyPropertyValue(item, value);
		}
	}
	return value;
}

int Cards::getExtraSpeedFromCards(const Player* player) const
{
	int baseSpeed = player->getBaseSpeed();
	int value = 0;
	loopCardEffects(player,
		[&baseSpeed, &value](const CardEffect* effect, bool &cont) { effect->applySpeed(baseSpeed, value); });
	return value;
}

bool Cards::shouldConsumeAmmoFromCards(const Player* player, const Item* ammoItem) const
{
	if (!ammoItem) return true;
	bool result = true;

	loopCardEffects(player,
		[&player, &ammoItem, &result](const CardEffect* effect, bool &cont) {
			if (effect->applyNoAmmo(player, ammoItem)) {
				result = false;
				cont = false;
			}
		});

	return result;
}

CardEffect* CardEffect::loadFromXml(const xmlNodePtr &effectNode)
{
	int intValue;
	std::string strValue;

	if (xmlStrcmp(effectNode->name,(const xmlChar*)"damage") == 0) {
		CardEffectDamage* effect = new CardEffectDamage();

		if (!readXMLString(effectNode, "type", strValue) || !getCombatType(strValue, effect->type)) {
			std::cout << "Warning: [CardEffect::loadFromXml] Invalid damage card effect type" << std::endl;
			delete effect;
			return nullptr;
		}

		if (readXMLInteger(effectNode, "value", intValue) && intValue != 0) {
			effect->value = intValue;
		} else {
			std::cout << "Warning: [CardEffect::loadFromXml] Invalid damage card effect value" << std::endl;
			delete effect;
			return nullptr;
		}

		effect->percent = readXMLString(effectNode, "percent", strValue) && strValue == "true";

		if (readXMLInteger(effectNode, "chance", intValue) && intValue > 0) {
			effect->chance = intValue;
		}

		if (readXMLInteger(effectNode, "blocked", intValue) && intValue >= 0 && intValue <= 3) {
			effect->blocked = intValue;
		}

		return effect;

	} else if (xmlStrcmp(effectNode->name,(const xmlChar*)"attackModify") == 0) {
		CardEffectAttackModify* effect = new CardEffectAttackModify();

		effect->percent = readXMLString(effectNode, "percent", strValue) && strValue == "true";

		if (readXMLString(effectNode, "type", strValue)) {
			getCombatType(strValue, effect->type);
		}

		if (readXMLInteger(effectNode, "value", intValue)) {
			effect->value = intValue;
		}

		if (readXMLInteger(effectNode, "chance", intValue) && intValue > 0) {
			effect->chance = intValue;
		}		
		
		return effect;

	} else if (xmlStrcmp(effectNode->name,(const xmlChar*)"steal") == 0) {
		CardEffectSteal* effect = new CardEffectSteal();

		effect->percent = readXMLString(effectNode, "percent", strValue) && strValue == "true";

		if (readXMLString(effectNode, "type", strValue)) {
			if (asLowerCaseString(strValue) == "mana") {
				effect->health = false;
			} else if (asLowerCaseString(strValue) != "health") {
				std::cout << "Warning: [CardEffect::loadFromXml] Wrong type parameter \"" << strValue << "\"" << std::endl;
				delete effect;
				return nullptr;
			}
		} else {
			std::cout << "Warning: [CardEffect::loadFromXml] Missing type parameter for steal effect" << std::endl;
			delete effect;
			return nullptr;
		}

		if (readXMLInteger(effectNode, "value", intValue)) {
			effect->value = intValue;
 		} else {
			std::cout << "Warning: [CardEffect::loadFromXml] Missing value parameter for steal effect" << std::endl;
			delete effect;
			return nullptr;
		}

		if (readXMLInteger(effectNode, "chance", intValue) && intValue > 0) {
			effect->chance = intValue;
		}
		
		return effect;

	} else if (xmlStrcmp(effectNode->name,(const xmlChar*)"absorb") == 0) {
		
		if (readXMLString(effectNode, "type", strValue)) {
			CombatType_t type;
			if (getCombatType(strValue, type)) {
					if (readXMLInteger(effectNode, "value", intValue) && intValue > 0) {

						CardEffectAbsorb* effect = new CardEffectAbsorb();
						effect->type = type;
						effect->value = intValue;
						effect->percent = readXMLString(effectNode, "percent", strValue) && strValue == "true";
						if (readXMLInteger(effectNode, "chance", intValue) && intValue > 0) {
							effect->chance = intValue;
						}
						return effect;

					} else {
						std::cout << "Warning: [CardEffect::loadFromXml] Missing \"value\" parameter for absorb effect" << std::endl;
					}
			} else {
				std::cout << "Warning: [CardEffect::loadFromXml] Unknown absorb card effect type \"" << strValue << "\"" << std::endl;
			}
		} else {
			std::cout << "Warning: [CardEffect::loadFromXml] Missing \"type\" parameter for absorb effect" << std::endl;
		}

	} else if (xmlStrcmp(effectNode->name,(const xmlChar*)"add") == 0) {
		bool skillSet = false;
		CardEffectAdd* effect = new CardEffectAdd();

		if (readXMLString(effectNode, "skill", strValue)) {
			skillSet = getSkillType(strValue, effect->skill);
			if (!skillSet) {
				std::cout << "Warning: [CardEffect::loadFromXml] Unknown add card skill type \"" << strValue << "\"" << std::endl;
			}
		}

		if (readXMLInteger(effectNode, "value", intValue) && intValue != 0) {
			effect->skillValue = intValue;
		}

		if (skillSet && effect->skillValue == 0) {
			std::cout << "Warning: [CardEffect::loadFromXml] Missing skill type or value for CardEffectAdd" << std::endl;
			delete effect;
			return nullptr;
		}

		if (readXMLInteger(effectNode, "armor", intValue) && intValue > 0) {
			effect->armor = intValue;
		} else if (readXMLInteger(effectNode, "attack", intValue) && intValue > 0) {
			effect->attackOrDefense = intValue;
		}
		if (readXMLInteger(effectNode, "defense", intValue) && intValue > 0) {
			effect->attackOrDefense = intValue;
		}

		if (readXMLInteger(effectNode, "speed", intValue) && intValue > 0) {
			effect->speed = intValue;
		}

		effect->percent = readXMLString(effectNode, "percent", strValue) && strValue == "true";

		return effect;
	} else if (xmlStrcmp(effectNode->name,(const xmlChar*)"noAmmo") == 0) {
		CardEffectNoAmmo* effect = new CardEffectNoAmmo();

		if (readXMLInteger(effectNode, "chance", intValue) && intValue > 0) {
			effect->chance = intValue;
		}

		if (readXMLString(effectNode, "itemIds", strValue) && strValue != "") {
			std::vector<std::string> tokens = explodeString(strValue, ",");

			for (const std::string& token : tokens) {
				try {
					uint16_t value = static_cast<uint16_t>(std::stoi(token));
					effect->ammos.push_back(value);
				} catch (const std::invalid_argument& e) {
					std::cerr << "Invalid number: '" << token << "'" << std::endl;
				} catch (const std::out_of_range& e) {
					std::cerr << "Number out of range: '" << token << "'" << std::endl;
				}
			}
		}

		if (effect->ammos.size() == 0) {
			std::cout << "Warning: [CardEffect::loadFromXml] Missing itemIds value for CardEffectNoAmmo" << std::endl;
			delete effect;
			return nullptr;
		}

		return effect;
	} else {
			std::cout << "Warning: [CardEffect::loadFromXml] Unknown card effect \"" << effectNode->name << "\"" << std::endl;
	}

	return nullptr;
}

bool CardEffect::success(const uint8_t &chance) const
{
	return chance >= (uint8_t)random_range(0, 100);
}

void CardEffectDamage::applyAddDamage(CombatParams &combat) const
{
	if (!success(chance)) {
		return;
	}

	ExtraCombatParams extra;
	extra.combatType = type;

	switch(blocked) {
		case -1:
			extra.blockedByArmor = combat.blockedByArmor;
			extra.blockedByShield = combat.blockedByShield;
			break;
		case 1:
			extra.blockedByArmor = true;
			break;
		case 2:
			extra.blockedByShield = true;
			break;
		case 3:
			extra.blockedByArmor = true;
			extra.blockedByShield = true;
			break;
	}

	extra.damage = extra.baseDamage = percent
		? (combat.baseDamage * value) / 100
		: -value;

	combat.extras.push_back(extra);
}

void CardEffectAttackModify::applyAddDamage(CombatParams &combat) const
{
	if (!success(chance)) {
		return;
	}

	if (type != COMBAT_NONE) {
		combat.combatType = type;
	}

	if (value != 0) {
		if (percent) {
			combat.damage = (combat.baseDamage * (100 + value)) / 100;
		} else {
			combat.damage -= value;
		}
	}
}

void CardEffectSteal::applyPostAttack(Player* player, const CombatParams &params) const
{
	if (!success(chance)) {
		return;
	}

	int recoveredValue = value;
	if (percent) {
		int32_t damage = params.damage;
		for (const ExtraCombatParams &extra : params.extras) {
			damage += extra.damage;
		}
		recoveredValue = damage * value / 100;
	}

	if (health) {
		int healthRegain = std::min(player->getMaxHealth() - player->getHealth(), recoveredValue);
		if (healthRegain > 0) {
			player->changeHealth(healthRegain);
			player->sendAnimatedText(player->getPosition(), TEXTCOLOR_LIGHTGREEN, std::to_string(healthRegain));
		}
	} else {
		int manaRegain = std::min(player->getMaxMana() - player->getMana(), recoveredValue);
		if (manaRegain > 0) {
			player->changeMana(manaRegain);
			player->sendAnimatedText(player->getPosition(), TEXTCOLOR_LIGHTBLUE, std::to_string(manaRegain));
		}
	}
}

void CardEffectAbsorb::applyReduceDamage(CombatParams &combat) const
{
	if (!success(chance)) {
		return;
	}

	if (combat.combatType == type) {
		if (percent) {
			int damageReduction = -combat.baseDamage * value / 100;
			combat.damage += std::min(-combat.damage, damageReduction);
		} else {
			combat.damage += std::min(-combat.damage, value);
		}
	}

	for (ExtraCombatParams &extra : combat.extras) {
		if (extra.combatType == type) {
			if (percent) {
				int damageReduction = -extra.baseDamage * value / 100;
				extra.damage += std::min(-extra.damage, damageReduction);
			} else {
				extra.damage += std::min(-extra.damage, value);
			}
		}
	}
}

void CardEffectAbsorb::applyReduceDamage(const CombatType_t &combatType, int &damage) const
{
	if (combatType == type && success(chance)) {
		if (percent) {
			int damageReduction = -damage * value / 100;
			damage += std::min(-damage, damageReduction);
		} else {
			damage += std::min(-damage, value);
		}
	}
}

void CardEffectAdd::applySkill(Player* player) const
{
	if (skillValue != 0) {
		if (percent) {
			uint32_t skillLevel = player->getSkill(skill).level;
			player->addVarCardSkill(skill, (skillLevel * skillValue) / 100);
		} else {
			player->addVarCardSkill(skill, skillValue);
		}
	}
}

void CardEffectAdd::applyPropertyValue(const Item* item, int &value) const
{
	if (attackOrDefense != 0) {
		const WeaponType_t weaponType = item->getWeaponType();
		if (weaponType == WEAPON_SLASH || weaponType == WEAPON_STRIKE || weaponType == WEAPON_PIERCE || weaponType == WEAPON_DIST || weaponType == WEAPON_SHIELD) {
				value += attackOrDefense;
		}
	}
	if (armor != 0) {
		const int wieldPosition = item->getWieldPosition();
		if (wieldPosition == SLOT_HEAD || wieldPosition == SLOT_ARMOR || wieldPosition == SLOT_LEGS || wieldPosition == SLOT_FEET) {
			value += armor;
		}
	}
}

void CardEffectAdd::applySpeed(const int currentSpeed, int &value) const
{
	if (speed != 0) {
		if (percent) {
			value += (currentSpeed * speed) / 100;
		} else {
			value += speed;
		}
	}
}

bool CardEffectNoAmmo::applyNoAmmo(const Player* player, const Item* ammoItem) const
{
	if (!success(chance)) {
		return false;
	}
	return std::find(ammos.begin(), ammos.end(), ammoItem->getID()) != ammos.end();
}