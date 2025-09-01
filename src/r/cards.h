#ifndef __OTSERV_CARDS_H__
#define __OTSERV_CARDS_H__

#include "../const.h"
#include "../definitions.h"
#include "../enums.h"
#include "../itemloader.h"
#include "../items.h"
#include "../position.h"
#include <map>

enum CardAttachment_t {
	CARD_ATTACHMENT_WEAPON = 0,
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

class Card {
private:
	//It is private because calling it can cause unexpected results
	Card(const Card& it){};

public:
	Card();
	~Card() = default;

	CardAttachment_t attachment;
	uint32_t id;
	std::string description;
	std::string article;
	std::string name;
};


class Cards{
public:
	Cards() = default;
	~Cards() = default;

	const Card* getCardById(uint32_t id) const;
	bool loadFromXml(const std::string& datadir);

protected:
	typedef std::map<uint32_t, Card*> CardMap;
	CardMap cards;

	std::string m_datadir;
};

#endif
