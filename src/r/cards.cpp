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
#include "cards.h"
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

Card::Card()
{
	attachment = CARD_ATTACHMENT_WEAPON;
	id = 0;
	description = "";
	name = "";
	article = "";
}

const Card* Cards::getCardById(uint32_t id) const
{
	auto it = cards.find(id);
	if (it != cards.end()) {
		return it->second;
	}
	return nullptr;
}

bool Cards::loadFromXml(const std::string& datadir)
{
	m_datadir = datadir;
	std::string filename = m_datadir + "/items/cards.xml";

	static const std::unordered_map<std::string, CardAttachment_t> attachmentMap = {
			{"weapon",    CARD_ATTACHMENT_WEAPON},
			{"shield",    CARD_ATTACHMENT_SHIELD},
			{"any-armor", CARD_ATTACHMENT_ANY_ARMOR},
			{"helmet",    CARD_ATTACHMENT_HELMET},
			{"chest",     CARD_ATTACHMENT_CHEST},
			{"legs",      CARD_ATTACHMENT_LEGS},
			{"boots",     CARD_ATTACHMENT_BOOTS},
			{"ring",      CARD_ATTACHMENT_RING},
			{"necklace",  CARD_ATTACHMENT_NECKLACE}
	};

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
					std::cout << std::endl << "Duplicate cardid \"" << intValue << "\"";
					itemNode = itemNode->next;
					continue;
				}

				mCard = new Card();
				mCard->id = id;

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
							auto it = attachmentMap.find(asLowerCaseString(strValue));
							if (it != attachmentMap.end()) {
									mCard->attachment = it->second;
							} else {
									std::cout << "Warning: [Cards::loadFromXml] Unknown card attachment value " << strValue << std::endl;
							}
						}

					} else if (xmlStrcmp(itemAttributesNode->name, (const xmlChar*)"effects") == 0) {
						xmlNodePtr effectNode = itemAttributesNode->children;
						while (effectNode) {
							// load scripts and effects here
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
};




/*
else if(asLowerCaseString(strValue) == "cardattachment"){
							if(it.type == ITEM_TYPE_CARD && readXMLString(itemAttributesNode, "value", strValue)){
								
							}

*/