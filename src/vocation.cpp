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
#include "otpch.h"

#include "configmanager.h"
#include "vocation.h"
#include "tools.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <iostream>
#include <cmath>

extern ConfigManager g_config;

Vocations::Vocations()
{
	//
}

Vocations::~Vocations()
{
	for(VocationsMap::iterator it = vocationsMap.begin(); it != vocationsMap.end(); ++it){
		delete it->second;
	}
	vocationsMap.clear();
}

bool Vocations::loadFromXml(const std::string& datadir)
{
	std::string filename = datadir + "vocations.xml";

	xmlDocPtr doc = xmlParseFile(filename.c_str());
	if(doc){
		xmlNodePtr root, p;
		root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"vocations") != 0){
			xmlFreeDoc(doc);
			return false;
		}

		p = root->children;

		while(p){
			std::string str;
			int intVal;
			float floatVal;
			if(xmlStrcmp(p->name, (const xmlChar*)"vocation") == 0){
				Vocation* voc = new Vocation();
				uint32_t voc_id;
				xmlNodePtr skillNode;
				if(readXMLInteger(p, "id", intVal)){
					voc_id = intVal;

					voc->baseVocation = voc_id;
					voc->promotesTo = 0;
					voc->rebirthsTo = 0;

					if(readXMLString(p, "name", str)){
						voc->name = str;
					}
					if(readXMLString(p, "description", str)){
						voc->description = str;
					}
					if(readXMLInteger(p, "gaincap", intVal)){
						voc->gainCap = intVal;
					}
					if(readXMLInteger(p, "gainhp", intVal)){
						voc->gainHP = intVal;
					}
					if(readXMLInteger(p, "gainmana", intVal)){
						voc->gainMana = intVal;
					}
					if(readXMLInteger(p, "gainhpticks", intVal)){
						voc->gainHealthTicks = intVal;
					}
					if(readXMLInteger(p, "gainhpamount", intVal)){
						voc->gainHealthAmount = intVal;
					}
					if(readXMLInteger(p, "gainmanaticks", intVal)){
						voc->gainManaTicks = intVal;
					}
					if(readXMLInteger(p, "gainmanaamount", intVal)){
						voc->gainManaAmount = intVal;
					}
					if(readXMLInteger(p, "attackspeed", intVal)){
						voc->attackSpeed = intVal;
					}
					skillNode = p->children;
					while(skillNode){
						if(xmlStrcmp(skillNode->name, (const xmlChar*)"magic") == 0){
							if(readXMLInteger(skillNode, "base", intVal)){
								voc->manaBase = intVal;
							}
							if(readXMLFloat(skillNode, "multiplier", floatVal)){
								voc->manaMultiplier = floatVal;
							}
						}
						else if(xmlStrcmp(skillNode->name, (const xmlChar*)"skill") == 0){
							int32_t skill_id;
							if(readXMLInteger(skillNode, "id", intVal)){
								skill_id = intVal;
								if(skill_id < SKILL_FIRST || skill_id > SKILL_LAST){
									std::cout << "No valid skill id. " << skill_id << std::endl;

								}
								else{
									if(readXMLInteger(skillNode, "base", intVal)){
										voc->skillBases[skill_id] = intVal;
									}
									if(readXMLFloat(skillNode, "multiplier", floatVal)){
										voc->skillMultipliers[skill_id] = floatVal;
									}
								}
							}
							else{
								std::cout << "Missing skill id." << std::endl;
							}
						}
						else if(xmlStrcmp(skillNode->name, (const xmlChar*)"damage") == 0){
							if(readXMLFloat(skillNode, "magicDamage", floatVal)){
								voc->magicBaseDamage = floatVal;
							}
							if(readXMLFloat(skillNode, "wandDamage", floatVal)){
								voc->wandBaseDamage = floatVal;
							}
							if(readXMLFloat(skillNode, "healingDamage", floatVal)){
								voc->healingBaseDamage = floatVal;
							}
						}
						else if(xmlStrcmp(skillNode->name, (const xmlChar*)"meleeDamage") == 0){
							if(readXMLFloat(skillNode, "pierce", floatVal)){
								voc->pierceBaseDamage = floatVal;
							}
							if(readXMLFloat(skillNode, "slash", floatVal)){
								voc->slashBaseDamage = floatVal;
							}
							if(readXMLFloat(skillNode, "strike", floatVal)){
								voc->strikeBaseDamage = floatVal;
							}
							if(readXMLFloat(skillNode, "dist", floatVal)){
								voc->distBaseDamage = floatVal;
							}
							if(readXMLFloat(skillNode, "fist", floatVal)){
								voc->fistBaseDamage = floatVal;
							}
						}
						else if(xmlStrcmp(skillNode->name, (const xmlChar*)"defense") == 0){
							if(readXMLFloat(skillNode, "baseDefense", floatVal)){
								voc->baseDefense = floatVal;
							}
							if(readXMLFloat(skillNode, "armorDefense", floatVal)){
								voc->armorDefense = floatVal;
							}
						}
						else if(xmlStrcmp(skillNode->name, (const xmlChar*)"evolution") == 0){
							if(readXMLInteger(skillNode, "base", intVal)){
								voc->baseVocation = intVal;
							}
							if(readXMLInteger(skillNode, "promotes", intVal)){
								voc->promotesTo = intVal;
							}
							if(readXMLInteger(skillNode, "rebirths", intVal)){
								voc->rebirthsTo = intVal;
							}
						}
						skillNode = skillNode->next;
					}

					//std::cout << "Voc id: " << voc_id << std::endl;
					//voc->debugVocation();
					vocationsMap[voc_id] = voc;
					voc->loadCacheSkillValues();

				}
				else{
					std::cout << "Missing vocation id." << std::endl;
				}
			}
			p = p->next;
		}
		xmlFreeDoc(doc);

	}
	return true;
}

bool Vocations::exists(const uint32_t& vocationId) const
{
	return vocationsMap.find(vocationId) != vocationsMap.end();
}

bool Vocations::getVocation(const uint32_t vocationId, Vocation*& vocation)
{
	VocationsMap::const_iterator it = vocationsMap.find(vocationId);
	if(it != vocationsMap.end()){
		vocation = it->second;
		return true;
	}
	std::cout << "Warning: [Vocations::getVocation] Vocation " << vocationId << " not found." << std::endl;
	return false;
}

bool Vocations::getVocationId(const std::string& name, int32_t& vocationId) const
{
	for(VocationsMap::const_iterator it = vocationsMap.begin(); it != vocationsMap.end(); ++it){
		if(iequals(it->second->name, name)){
			vocationId = it->first;
			return true;
		}
	}
	return false;
}

bool Vocations::getUnpromotedVocation(const uint32_t vocationId, uint32_t& unpromoted)
{
	bool found = false;
	Vocation* vocation;
	for(VocationsMap::iterator it = vocationsMap.begin(); it != vocationsMap.end(); ++it){
		vocation = it->second;
		if (vocation->getPromotion() == vocationId) {
			unpromoted = it->first;
			return true;
		}
	}
	return false;
}

Vocation::Vocation()
{
	name = "none";
	gainHealthTicks = 6;
	gainHealthAmount = 1;
	gainManaTicks = 6;
	gainManaAmount = 1;
	gainCap = 5;
	gainMana = 5;
	gainHP = 5;
	manaBase = 1600;
	manaMultiplier = 4.0;
	attackSpeed = 1500;

	skillMultipliers[0] = 1.5f;
	skillMultipliers[1] = 2.0f;
	skillMultipliers[2] = 2.0f;
	skillMultipliers[3] = 2.0f;
	skillMultipliers[4] = 2.0f;
	skillMultipliers[5] = 1.5f;
	skillMultipliers[6] = 1.1f;

	skillBases[0] = 50;
	skillBases[1] = 50;
	skillBases[2] = 50;
	skillBases[3] = 50;
	skillBases[4] = 30;
	skillBases[5] = 100;
	skillBases[6] = 20;

	pierceBaseDamage = 1.;
	slashBaseDamage = 1.;
	strikeBaseDamage = 1.;
	distBaseDamage = 1.;
	fistBaseDamage = 1.;

	magicBaseDamage = 1.;
	wandBaseDamage = 1.;
	healingBaseDamage = 1.;

	baseDefense = 1.;
	armorDefense = 1.;

	promotesTo = 0;
	baseVocation = 0;
}

Vocation::~Vocation()
{
	cacheMana.clear();
	for(uint32_t i = SKILL_FIRST; i < SKILL_LAST; ++i){
		cacheSkill[i].clear();
	}
}

void Vocation::loadCacheSkillValues()
{
	// SKILLS
	int32_t maxSkill = g_config.getNumber(ConfigManager::MAX_SKILL);
	if (maxSkill <= 0) {
		maxSkill = 1000;
	}

	for (int skill = SKILL_FIRST; skill <= SKILL_LAST; skill++) {
		skillCacheMap& skillMap = cacheSkill[skill];
		const float &mult = skillMultipliers[skill];

		double value = skillBases[skill];
		skillMap[10] = 0;
		skillMap[11] = value;
		for (int level=12; level<=maxSkill; level++) {
			value *= mult;
			uint64_t accValue = skillMap[level-1]+(uint64_t)value;
			if (accValue < UINT_MAX) {
				skillMap[level] = accValue;
			} else {
				break;
			}
		}
	}

	// MAGIC LEVEL
	int32_t maxMagic = g_config.getNumber(ConfigManager::MAX_MAGIC);
	if (maxMagic <= 0) {
		maxMagic = 1000;
	}

	double value = manaBase;
	cacheMana[0] = 0;
	cacheMana[1] = value;
	for (int level=2; level<=maxMagic; level++) {
		value *= manaMultiplier;
		if (value < UINT64_MAX && cacheMana[level-1] < UINT64_MAX-(uint64_t)value) {
			cacheMana[level] = cacheMana[level-1]+(uint64_t)value;
		} else {
			break;
		}
	}
}

int32_t Vocation::getSkillPercent(const int32_t &skill, const uint64_t &level, uint64_t count)
{
	skillCacheMap& skillMap = cacheSkill[skill];

	skillCacheMap::iterator it = skillMap.find(level+1);
	if (it == skillMap.end()) {
		return -1;
	}

	const uint64_t &baseCount = skillMap[level];
	const uint64_t &endValue = it->second - baseCount;
	count -= baseCount;

	if(endValue > 0){
		if (count >= endValue) {
			return 100;
		} else {
			return (int32_t)((double)count / endValue * 100);
		}
	}

	return 0;
}

uint32_t Vocation::getSkillLevel(const int32_t &skill, const uint64_t &count)
{
	skillCacheMap& skillMap = cacheSkill[skill];

	for(skillCacheMap::iterator it = skillMap.begin(); it != skillMap.end();it++){
		if (count < it->second) {
			return it->first-1;
		}
	}
	return 0;
}

bool Vocation::adjustMaxSkillCount(const int32_t &skill, uint64_t &count, uint32_t &skillPercent)
{
	skillCacheMap& skillMap = cacheSkill[skill];
	uint64_t maxCount = skillMap.rbegin()->second;
	if (count >= maxCount) {
		count = maxCount;
		skillPercent = 0;
		return false;
	} else {
		return true;
	}
}

int32_t Vocation::getMagicLevelPercent(const uint64_t &mLevel, uint64_t manaUsed)
{
	std::map<uint32_t, uint64_t>::iterator it = cacheMana.find(mLevel+1);
	if (it == cacheMana.end()) {
		return -1;
	}

	const uint64_t &baseCount = cacheMana[mLevel];
	const uint64_t endValue = it->second - baseCount;
	manaUsed -= baseCount;

	if(endValue > 0){
		if (manaUsed >= endValue) {
			return 100;
		} else {
			return (int32_t)((double)manaUsed / endValue * 100);
		}
	}

	return 0;
}

uint32_t Vocation::getMagicLevel(const uint64_t &manaUsed)
{
	for(std::map<uint32_t, uint64_t>::iterator it = cacheMana.begin(); it != cacheMana.end();it++){
		if (manaUsed < it->second) {
			return it->first-1;
		}
	}
	std::map<uint32_t, uint64_t>::reverse_iterator it = cacheMana.rbegin();
	if (manaUsed >= it->second) {
		return it->first;
	}
	return 0;
}


bool Vocation::adjustMaxManaSpent(uint64_t &manaUsed, uint32_t &magLevelPercent)
{
	uint64_t maxMana = cacheMana.rbegin()->second;
	if (manaUsed >= maxMana) {
		manaUsed = maxMana;
		magLevelPercent = 0;
		return false;
	} else {
		return true;
	}
}

void Vocation::debugVocation()
{
	std::cout << "name: " << name << std::endl;
	std::cout << "gain cap: " << gainCap << " hp: " << gainHP << " mana: " << gainMana << std::endl;
	std::cout << "gain time: Health(" << gainHealthTicks << " ticks, +" << gainHealthAmount << "). Mana(" <<
		gainManaTicks << " ticks, +" << gainManaAmount << ")" << std::endl;
	std::cout << "mana multiplier: " << manaMultiplier << std::endl;
	for(int i = SKILL_FIRST; i < SKILL_LAST; ++i){
		std::cout << "Skill id: " << i << " multiplier: " << skillMultipliers[i] << std::endl;
	}
}