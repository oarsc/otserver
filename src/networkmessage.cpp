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

#include "networkmessage.h"
#include "container.h"
#include "creature.h"
#include "player.h"
#include "position.h"
#include "rsa.h"
#include <string>
#include <iostream>
#include <sstream>

int32_t NetworkMessage::decodeHeader()
{
	return (int32_t)(m_MsgBuf[0] | m_MsgBuf[1] << 8);
}

/******************************************************************************/
std::string NetworkMessage::getString()
{
	uint16_t stringlen = getU16();
	if(stringlen >= (NETWORKMESSAGE_MAXSIZE - m_ReadPos))
		return std::string();

	char* v = (char*)(m_MsgBuf + m_ReadPos);
	m_ReadPos += stringlen;
	return std::string(v, stringlen);
}

std::string NetworkMessage::getRaw()
{
	uint16_t stringlen = m_MsgSize- m_ReadPos;
	if(stringlen >= (NETWORKMESSAGE_MAXSIZE - m_ReadPos))
		return std::string();

	char* v = (char*)(m_MsgBuf + m_ReadPos);
	m_ReadPos += stringlen;
	return std::string(v, stringlen);
}

Position NetworkMessage::getPosition()
{
	Position pos;
	pos.x = getU16();
	pos.y = getU16();
	pos.z = getByte();
	return pos;
}
/******************************************************************************/

void NetworkMessage::addString(const char* value)
{
	uint32_t stringlen = (uint32_t)strlen(value);
	if(!canAdd(stringlen+2) || stringlen > 8192)
		return;

	addU16(stringlen);
	strcpy((char*)(m_MsgBuf + m_ReadPos), value);
	m_ReadPos += stringlen;
	m_MsgSize += stringlen;
}

void NetworkMessage::addBytes(const char* bytes, uint32_t size)
{
	if(!canAdd(size) || size > 8192)
		return;

	memcpy(m_MsgBuf + m_ReadPos, bytes, size);
	m_ReadPos += size;
	m_MsgSize += size;
}

void NetworkMessage::addPaddingBytes(uint32_t n)
{
	if(!canAdd(n))
		return;

	memset((void*)&m_MsgBuf[m_ReadPos], 0x33, n);
	m_MsgSize = m_MsgSize + n;
}

void NetworkMessage::addPosition(const Position& pos)
{
	addU16(pos.x);
	addU16(pos.y);
	addByte(pos.z);
}

void NetworkMessage::addItem(uint16_t id, uint16_t count, uint8_t rank)
{
	const ItemType &it = Item::items[id];
	addU16(it.clientId);

	if(it.stackable){
		addU16(count);

	} else {
		addByte(rank);

		if(it.isSplash() || it.isFluidContainer()){
			addU16(Item::items.getClientFluidType(FluidTypes_t(count)));
		}
	}
}

void NetworkMessage::addItem(const Item* item)
{
	addItem(item->getID(), item->getSubType(), item->getRank()); // TODO: send card slots....
}

void NetworkMessage::addItemId(const Item *item){
	const ItemType &it = Item::items[item->getID()];
	addU16(it.clientId);
}

void NetworkMessage::addItemId(uint16_t itemId){
	const ItemType &it = Item::items[itemId];
	addU16(it.clientId);
}
