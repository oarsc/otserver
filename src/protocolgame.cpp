//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Implementation of tibia v8.0 protocol
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

#include "protocolgame.h"
#include "networkmessage.h"
#include "outputmessage.h"
#include "items.h"
#include "tile.h"
#include "player.h"
#include "chat.h"
#include "configmanager.h"
#include "actions.h"
#include "game.h"
#include "map.h"
#include "spells.h"
#include "ioplayer.h"
#include "house.h"
#include "waitlist.h"
#include "ban.h"
#include "ioaccount.h"
#include "connection.h"
#include "creatureevent.h"
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <list>
#include <fstream>

extern Game g_game;
extern ConfigManager g_config;
extern Spells* g_spells;
extern SpellSets g_spellsets;
extern BanManager g_bans;
extern CreatureEvents* g_creatureEvents;
Chat g_chat;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolGame::protocolGameCount = 0;
#endif

// Helping templates to add dispatcher tasks

template<class FunctionType>
void ProtocolGame::addGameTaskInternal(bool droppable, uint32_t delay, const FunctionType& func)
{
	if(droppable)
		g_dispatcher.addTask(createTask(delay, func));
	else
		g_dispatcher.addTask(createTask(func));
}

ProtocolGame::ProtocolGame(Connection_ptr connection) :
	Protocol(connection)
{
	player = NULL;
	m_debugAssertSent = false;
	m_acceptPackets = false;
	eventConnect = 0;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	protocolGameCount++;
#endif
}

ProtocolGame::~ProtocolGame()
{
	player = NULL;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	protocolGameCount--;
#endif
}

void ProtocolGame::setPlayer(Player* p)
{
	player = p;
}

void ProtocolGame::releaseProtocol()
{
	//dispatcher thread
	if(player){
		if(player->client == this){
			player->client = NULL;
		}
	}

	Protocol::releaseProtocol();
}

void ProtocolGame::deleteProtocolTask()
{
	//dispatcher thread
	if(player){
		#ifdef __DEBUG_NET_DETAIL__
		std::cout << "Deleting ProtocolGame - Protocol:" << this << ", Player: " << player << std::endl;
		#endif

		g_game.FreeThing(player);
		player = NULL;
	}

	Protocol::deleteProtocolTask();
}

bool ProtocolGame::login(const std::string& name, bool isSetGM)
{
	//dispatcher thread
	Player* _player = g_game.getPlayerByName(name);
	if(!_player || g_config.getNumber(ConfigManager::ALLOW_CLONES)){
		player = new Player(name, this);
		player->useThing2();
		player->setID();

		if(!IOPlayer::instance()->loadPlayer(player, name, true)){
#ifdef __DEBUG__
			std::cout << "ProtocolGame::login - preloading loadPlayer failed - " << name << std::endl;
#endif
			disconnectClient(0x14, "Your character could not be loaded.");
			return false;
		}

		if(g_bans.isPlayerBanished(name) && !player->hasFlag(PlayerFlag_CannotBeBanned)){
			disconnectClient(0x14, "Your character is locked!");
			return false;
		}

		if(g_bans.isAccountBanished(player->getAccountId()) && !player->hasFlag(PlayerFlag_CannotBeBanned)){
			disconnectClient(0x14, "Your account is banished!");
			return false;
		}

		if(isSetGM && !player->hasFlag(PlayerFlag_CanAlwaysLogin) &&
			!g_config.getNumber(ConfigManager::ALLOW_GAMEMASTER_MULTICLIENT))
		{
			disconnectClient(0x14, "You may only login with a Gamemaster account.");
			return false;
		}

		if(g_game.getGameState() == GAME_STATE_CLOSING && !player->hasFlag(PlayerFlag_CanAlwaysLogin)){
			disconnectClient(0x14, "The game is just going down.\nPlease try again later.");
			return false;
		}

		if(g_game.getGameState() == GAME_STATE_CLOSED && !player->hasFlag(PlayerFlag_CanAlwaysLogin)){
			disconnectClient(0x14, "Server is closed.");
			return false;
		}

		if(g_config.getNumber(ConfigManager::CHECK_ACCOUNTS) && !player->hasFlag(PlayerFlag_CanAlwaysLogin)
			&& g_game.getPlayerByAccount(player->getAccountId())){
			disconnectClient(0x14, "You may only login with one character per account.");
			return false;
		}

		if(!WaitingList::getInstance()->clientLogin(player)){
			int32_t currentSlot = WaitingList::getInstance()->getClientSlot(player);
			int32_t retryTime = WaitingList::getTime(currentSlot);
			std::stringstream ss;

			ss << "Too many players online.\n" << "You are at place "
				<< currentSlot << " on the waiting list.";

			OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
			if(output){
				TRACK_MESSAGE(output);
				output->addByte(0x16);
				output->addString(ss.str());
				output->addByte(retryTime);
				OutputMessagePool::getInstance()->send(output);
			}
			getConnection()->closeConnection();
			return false;
		}

		if(!IOPlayer::instance()->loadPlayer(player, name)){
#ifdef __DEBUG__
			std::cout << "ProtocolGame::login - loadPlayer failed - " << name << std::endl;
#endif
			disconnectClient(0x14, "Your character could not be loaded.");
			return false;
		}

		if(!g_game.placeCreature(player, player->getLoginPosition())){
			if(!g_game.placeCreature(player, player->getTemplePosition(), false, true)){
				disconnectClient(0x14, "Temple position is wrong. Contact the administrator.");
				return false;
			}
		}

		player->lastip = player->getIP();
		player->lastLoginSaved = std::max(time(NULL), player->lastLoginSaved + 1);
		player->lastLoginMs = OTSYS_TIME();
		IOPlayer::instance()->updateLoginInfo(player);
		m_acceptPackets = true;

		return true;
	}
	else{
		if(eventConnect != 0){
			//A task has already been scheduled just bail out (should not be overriden)
			disconnectClient(0x14, "You are already logged in.");
			return false;
		}

		if(_player->client){
			g_chat.removeUserFromAllChannels(_player);
			_player->disconnect();
			_player->isConnecting = true;
			addRef();
			eventConnect = g_scheduler.addEvent(
				createSchedulerTask(1000, std::bind(&ProtocolGame::connect, this, _player->getID(), true)));
			return true;
		}

		addRef();
		return connect(_player->getID());
	}

	return false;
}

bool ProtocolGame::connect(uint32_t playerId, const bool isLoggingIn)
{
	unRef();
	eventConnect = 0;
	Player* _player = g_game.getPlayerByID(playerId);
	if(!_player || _player->isRemoved() || _player->client){
		disconnectClient(0x14, "You are already logged in.");
		return false;
	}

	player = _player;
	player->useThing2();
	player->isConnecting = false;
	player->client = this;
	player->client->sendAddCreature(player, player->getPosition(),
		player->getTile()->__getIndexOfThing(player), isLoggingIn);
	player->lastip = player->getIP();
	IOPlayer::instance()->updateLoginInfo(player);
	m_acceptPackets = true;

	return true;
}

bool ProtocolGame::logout(bool forced)
{
	//dispatcher thread
	if(!player)
		return false;

	if(!player->isRemoved()){
		if(!forced){
			if(player->getTile()->hasFlag(TILESTATE_NOLOGOUT)){
				player->sendCancelMessage(RET_YOUCANNOTLOGOUTHERE);
				return false;
			}

			if(player->hasCondition(CONDITION_INFIGHT)){
				player->sendCancelMessage(RET_YOUMAYNOTLOGOUTDURINGAFIGHT);
				return false;
			}

			//scripting event - onLogOut
			if(!g_creatureEvents->playerLogOut(player)){
				//Let the script handle the error message
				return false;
			}
		}
	}

	if(Connection_ptr connection = getConnection()){
		connection->closeConnection();
	}

	return g_game.removeCreature(player);
}

void ProtocolGame::writeToOutputBuffer(const NetworkMessage& msg)
{
	OutputMessage_ptr out = getOutputBuffer(msg.getMessageLength());
	if (msg.isPriorized()) out->priorize();
	
	TRACK_MESSAGE(out);
	out->append(msg);
}

bool ProtocolGame::parseFirstPacket(NetworkMessage& msg)
{
	if(g_game.getGameState() == GAME_STATE_SHUTDOWN){
		getConnection()->closeConnection();
		return false;
	}

	/*uint16_t clientos =*/ msg.getU16();
	uint16_t version  = msg.getU16();

	if(!RSA_decrypt(msg)){
		getConnection()->closeConnection();
		return false;
	}

	uint32_t key[4];
	key[0] = msg.getU32();
	key[1] = msg.getU32();
	key[2] = msg.getU32();
	key[3] = msg.getU32();
	enableXTEAEncryption();
	setXTEAKey(key);

	bool isSetGM = (msg.getByte() == 1);
	uint32_t accnumber = msg.getU32();
	const std::string name = msg.getString();
	const std::string password = msg.getString();

	if(version < CLIENT_VERSION_MIN || version > CLIENT_VERSION_MAX){
		disconnectClient(0x0A, STRING_CLIENT_VERSION);
		return false;
	}

	if(g_game.getGameState() == GAME_STATE_STARTUP){
		std::string clientMessage = g_config.getString(ConfigManager::WORLD_NAME) + " is starting up. Please wait.";
		disconnectClient(0x14, clientMessage.c_str());
		return false;
	}

	if(g_bans.isIpDisabled(getIP())){
		disconnectClient(0x14, "Too many connections attempts from this IP. Try again later.");
		return false;
	}

	if(g_bans.isIpBanished(getIP())){
		disconnectClient(0x14, "Your IP is banished!");
		return false;
	}

	std::string acc_pass;
	if (!(IOAccount::instance()->getPassword(accnumber, name, acc_pass) && passwordTest(password, acc_pass))){
		g_bans.addLoginAttempt(getIP(), false);
		getConnection()->closeConnection();
		return false;
	}

	g_bans.addLoginAttempt(getIP(), true);

	g_dispatcher.addTask(
		createTask(std::bind(&ProtocolGame::login, this, name, isSetGM)));

	return true;
}

void ProtocolGame::onRecvFirstMessage(NetworkMessage& msg)
{
	parseFirstPacket(msg);
}

void ProtocolGame::onConnect()
{
	OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);

	OutputMessagePool::getInstance()->send(output);
}

void ProtocolGame::disconnectClient(uint8_t error, const char* message)
{
	OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
	if(output){
		TRACK_MESSAGE(output);
		output->addByte(error);
		output->addString(message);
		OutputMessagePool::getInstance()->send(output);
	}
	disconnect();
}

void ProtocolGame::disconnect()
{
	if(getConnection()){
		getConnection()->closeConnection();
	}
}

void ProtocolGame::parsePacket(NetworkMessage &msg)
{
	if(!player || !m_acceptPackets || g_game.getGameState() == GAME_STATE_SHUTDOWN || msg.getMessageLength() <= 0)
		return;

	uint8_t recvbyte = msg.getByte();
	//a dead player can not performs actions
	if((player->isRemoved() || player->getHealth() <= 0) && recvbyte != 0x14){
		return;
	}

	bool kickPlayer = false;

	switch(recvbyte){
	case 0x14: // logout
		g_dispatcher.addTask(createTask(std::bind(&ProtocolGame::logout, this, false)));
		break;

	case 0x1E: // keep alive / ping response
		g_dispatcher.addTask(createTask(std::bind(&Game::playerReceivePing, &g_game, player->getID())));
		break;

	case 0x1D: // ping
		player->sendPingBack();
		break;

	case 0x64: // move with steps
		parseAutoWalk(msg);
		break;

	case 0x65: // move north
		addGameTask(&Game::playerMove, player->getID(), NORTH);
		break;

	case 0x66: // move east
		addGameTask(&Game::playerMove, player->getID(), EAST);
		break;

	case 0x67: // move south
		addGameTask(&Game::playerMove, player->getID(), SOUTH);
		break;

	case 0x68: // move west
		addGameTask(&Game::playerMove, player->getID(), WEST);
		break;

	case 0x69: // stop-autowalk
		addGameTask(&Game::playerStopAutoWalk, player->getID());
		break;

	case 0x6A:
		addGameTask(&Game::playerMove, player->getID(), NORTHEAST);
		break;

	case 0x6B:
		addGameTask(&Game::playerMove, player->getID(), SOUTHEAST);
		break;

	case 0x6C:
		addGameTask(&Game::playerMove, player->getID(), SOUTHWEST);
		break;

	case 0x6D:
		addGameTask(&Game::playerMove, player->getID(), NORTHWEST);
		break;

	case 0x6F: // turn north
		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerTurn, player->getID(), NORTH);
		break;

	case 0x70: // turn east
		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerTurn, player->getID(), EAST);
		break;

	case 0x71: // turn south
		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerTurn, player->getID(), SOUTH);
		break;

	case 0x72: // turn west
		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerTurn, player->getID(), WEST);
		break;

	case 0x78: // throw item
		parseThrow(msg);
		break;

	case 0x7D: // Request trade
		parseRequestTrade(msg);
		break;

	case 0x7E: // Look at an item in trade
		parseLookInTrade(msg);
		break;

	case 0x7F: // Accept trade
		addGameTask(&Game::playerAcceptTrade, player->getID());
		break;

	case 0x80: // Close/cancel trade
		parseCloseTrade();
		break;

	case 0x82: // use item
		parseUseItem(msg);
		break;

	case 0x83: // use item
		parseUseItemEx(msg);
		break;

	case 0x84: // battle window
		parseBattleWindow(msg);
		break;

	case 0x85:	//rotate item
		parseRotateItem(msg);
		break;

	case 0x87: // close container
		parseCloseContainer(msg);
		break;

	case 0x88: //"up-arrow" - container
		parseUpArrowContainer(msg);
		break;

	case 0x89:
		parseTextWindow(msg);
		break;

	case 0x8A:
		parseHouseWindow(msg);
		break;

	case 0x8C: // throw item
		parseLookAt(msg);
		break;

	case 0x96:  // say something
		parseSay(msg);
		break;

	case 0x97: // request Channels
		addGameTask(&Game::playerRequestChannels, player->getID());
		break;

	case 0x98: // open Channel
		parseOpenChannel(msg);
		break;

	case 0x99: // close Channel
		parseCloseChannel(msg);
		break;

	case 0x9A: // open priv
		parseOpenPriv(msg);
		break;

	case 0x9B: //process report
		parseProcessRuleViolation(msg);
		break;

	case 0x9C: //gm closes report
		parseCloseRuleViolation(msg);
		break;

	case 0x9D: //player cancels report
		addGameTask(&Game::playerCancelRuleViolation, player->getID());
		break;

	case 0xA0: // set attack and follow mode
		parseFightModes(msg);
		break;

	case 0xA1: // attack
		parseAttack(msg);
		break;

	case 0xA2: //follow
		parseFollow(msg);
		break;

	case 0xA3:
		parseInviteToParty(msg);
		break;

	case 0xA4:
		parseJoinParty(msg);
		break;

	case 0xA5:
		parseRevokePartyInvitation(msg);
		break;

	case 0xA6:
		parsePassPartyLeadership(msg);
		break;

	case 0xA7:
		addGameTask(&Game::playerLeaveParty, player->getID());
		break;

	case 0xAA:
		addGameTask(&Game::playerCreatePrivateChannel, player->getID());
		break;

	case 0xAB:
		parseChannelInvite(msg);
		break;

	case 0xAC:
		parseChannelExclude(msg);
		break;

	case 0xBE: // cancel move
		addGameTask(&Game::playerCancelAttackAndFollow, player->getID());
		break;

	case 0xC9: //client request to resend the tile
		parseUpdateTile(msg);
		break;

	case 0xCA: //client request to resend the container (happens when you store more than container maxsize)
		parseUpdateContainer(msg);
		break;

	case 0xD2: // request outfit
		addGameTask(&Game::playerRequestOutfit, player->getID());
		break;

	case 0xD3: // set outfit
		parseSetOutfit(msg);
		break;

	case 0xDC:
		parseAddVip(msg);
		break;

	case 0xDD:
		parseRemoveVip(msg);
		break;

	case 0xE6:
		parseBugReport(msg);
		break;

	case 0xE7:
		parseViolationWindow(msg);
		break;

	case 0xE8:
		parseDebugAssert(msg);
		break;

	case 0x34:
		parseSayTargeted(msg);
		break;

	case 0x35:
		parseSellItem(msg);
		break;	

	case 0x36:
		parseBuySpells(msg);
		break;	

	default:
		std::cout << "Unknown packet header: " << std::hex << (int)recvbyte << std::dec << ", player " << player->getName() << std::endl;
		disconnectClient(0x14, "Unknown packet sent.");
		break;
	}

	if(msg.isOverrun()){ //we've got a badass over here
		std::cout << "msg.isOvverrun() == true, player " << player->getName() << std::endl;
		kickPlayer = true;
	}

	if(kickPlayer){
		player->kickPlayer();
	}
}

void ProtocolGame::getTileDescription(NetworkMessage &msg, const Tile* tile)
{
	if(tile){
		int count = 0;
		if(tile->ground){
			msg.addItem(tile->ground);
			count++;
		}

		const TileItemVector* items = tile->getItemList();
		const CreatureVector* creatures = tile->getCreatures();

		ItemVector::const_iterator it;
		if(items){
			for(it = items->getBeginTopItem(); ((it != items->getEndTopItem()) && (count < 10)); ++it){
				msg.addItem(*it);
				count++;
			}
		}

		if(creatures){
			CreatureVector::const_reverse_iterator cit;
			for(cit = creatures->rbegin(); ((cit != creatures->rend()) && (count < 10)); ++cit){
				if(player->canSeeCreature(*cit)){
					bool known;
					uint32_t removedKnown;
					checkCreatureAsKnown((*cit)->getID(), known, removedKnown);
					addCreature(msg, *cit, known, removedKnown);
					count++;
				}
			}
		}

		if(items){
			for(it = items->getBeginDownItem(); ((it != items->getEndDownItem()) && (count < 10)); ++it){
				msg.addItem(*it);
				count++;
			}
		}
	}
}

void ProtocolGame::getMapDescription(NetworkMessage &msg, int32_t x, int32_t y, int32_t z, int32_t width, int32_t height)
{
	int32_t skip = -1;
	int32_t startz, endz, zstep = 0;

	if(z > 7){
		startz = z - 2;
		endz = std::min((int32_t)MAP_MAX_LAYERS - 1, (int32_t)z + 2);
		zstep = 1;
	}
	else{
		startz = 7;
		endz = 0;

		zstep = -1;
	}

	for(int32_t nz = startz; nz != endz + zstep; nz += zstep){
		getFloorDescription(msg, x, y, nz, width, height, z - nz, skip);
	}

	if(skip >= 0){
		msg.addByte(skip);
		msg.addByte(0xFF);
	}

#ifdef __DEBUG__
	//printf("tiles in total: %d \n", cc);
#endif
}

void ProtocolGame::getFloorDescription(NetworkMessage &msg, int32_t x, int32_t y, int32_t z,
	int32_t width, int32_t height, int32_t offset, int32_t& skip)
{
	Tile* tile;

	for(int32_t nx = 0; nx < width; ++nx){
		for(int32_t ny = 0; ny < height; ++ny){
			tile = g_game.getTile(x + nx + offset, y + ny + offset, z);

			if(tile){
				if(skip >= 0){
					msg.addByte(skip);
					msg.addByte(0xFF);
				}
				skip = 0;


				getTileDescription(msg, tile);
			}
			else {
				skip++;
				if(skip == 0xFF){
					msg.addByte(0xFF);
					msg.addByte(0xFF);
					skip = -1;
				}
			}
		}
	}
}

void ProtocolGame::checkCreatureAsKnown(uint32_t id, bool &known, uint32_t &removedKnown)
{
	// loop through the known player and check if the given player is in
	std::list<uint32_t>::iterator i;
	for(i = knownCreatureList.begin(); i != knownCreatureList.end(); ++i){
		if((*i) == id){
			// know... make the creature even more known...
			knownCreatureList.erase(i);
			knownCreatureList.push_back(id);

			known = true;
			return;
		}
	}

	// ok, he is unknown...
	known = false;

	// ... but not in future
	knownCreatureList.push_back(id);

	// to many known creatures?
	if(knownCreatureList.size() > 150){
		// lets try to remove one from the end of the list
		for (int n = 0; n < 150; ++n){
			removedKnown = knownCreatureList.front();

			Creature* c = g_game.getCreatureByID(removedKnown);
			if ((!c) || (!canSee(c)))
				break;

			// this creature we can't remove, still in sight, so back to the end
			knownCreatureList.pop_front();
			knownCreatureList.push_back(removedKnown);
		}

		// hopefully we found someone to remove :S, we got only 150 tries
		// if not... lets kick some players with debug errors :)
		knownCreatureList.pop_front();
	}
	else{
		// we can cache without problems :)
		removedKnown = 0;
	}
}

bool ProtocolGame::canSee(const Creature* c) const
{
	if(c->isRemoved())
		return false;

	if(!player->canSeeCreature(c))
		return false;

	return canSee(c->getPosition());
}

bool ProtocolGame::canSee(const Position& pos) const
{
	return canSee(pos.x, pos.y, pos.z);
}

bool ProtocolGame::canSee(int x, int y, int z) const
{
#ifdef __DEBUG__
	if(z < 0 || z >= MAP_MAX_LAYERS) {
		std::cout << "WARNING! ProtocolGame::canSee() Z-value is out of range!" << std::endl;
	}
#endif

	const Position& myPos = player->getPosition();

	if(myPos.z <= 7){
		//we are on ground level or above (7 -> 0)
		//view is from 7 -> 0
		if(z > 7){
			return false;
		}
	}
	else if(myPos.z >= 8){
		//we are underground (8 -> 15)
		//view is +/- 2 from the floor we stand on
		if(std::abs(myPos.z - z) > 2){
			return false;
		}
	}

	//negative offset means that the action taken place is on a lower floor than ourself
	int offsetz = myPos.z - z;



	if ((x >= myPos.x - Map::clientViewportX + offsetz) && (x <= myPos.x + 1 + Map::clientViewportX + offsetz) &&
		(y >= myPos.y - Map::clientViewportY + offsetz) && (y <= myPos.y + 1 + Map::clientViewportY + offsetz))
		return true;

	return false;
}

//********************** Parse methods *******************************
void ProtocolGame::parseChannelInvite(NetworkMessage& msg)
{
	const std::string name = msg.getString();

	addGameTask(&Game::playerChannelInvite, player->getID(), name);
}

void ProtocolGame::parseChannelExclude(NetworkMessage& msg)
{
	const std::string name = msg.getString();

	addGameTask(&Game::playerChannelExclude, player->getID(), name);
}

void ProtocolGame::parseOpenChannel(NetworkMessage& msg)
{
	uint16_t channelId = msg.getU16();

	addGameTask(&Game::playerOpenChannel, player->getID(), channelId);
}

void ProtocolGame::parseCloseChannel(NetworkMessage &msg)
{
	uint16_t channelId = msg.getU16();

	addGameTask(&Game::playerCloseChannel, player->getID(), channelId);
}

void ProtocolGame::parseOpenPriv(NetworkMessage& msg)
{
	const std::string receiver = msg.getString();

	addGameTask(&Game::playerOpenPrivateChannel, player->getID(), receiver);
}

void ProtocolGame::parseProcessRuleViolation(NetworkMessage& msg)
{
	const std::string reporter = msg.getString();

	addGameTask(&Game::playerProcessRuleViolation, player->getID(), reporter);
}

void ProtocolGame::parseCloseRuleViolation(NetworkMessage& msg)
{
	const std::string reporter = msg.getString();

	addGameTask(&Game::playerCloseRuleViolation, player->getID(), reporter);
}

void ProtocolGame::parseDebug(NetworkMessage& msg)
{
	int dataLength = msg.getMessageLength() - 1;
	if(dataLength != 0){
		printf("data: ");
		int data = msg.getByte();
		while(dataLength > 0){
			printf("%d ", data);
			if(--dataLength > 0)
				data = msg.getByte();
		}
		printf("\n");
	}
}

void ProtocolGame::parseAutoWalk(NetworkMessage& msg)
{
	std::list<Direction> path;

	size_t numdirs = msg.getByte();
	for (size_t i = 0; i < numdirs; ++i) {
		uint8_t rawdir = msg.getByte();
		switch(rawdir) {
			case 1: path.push_back(EAST); break;
			case 2: path.push_back(NORTHEAST); break;
			case 3: path.push_back(NORTH); break;
			case 4: path.push_back(NORTHWEST); break;
			case 5: path.push_back(WEST); break;
			case 6: path.push_back(SOUTHWEST); break;
			case 7: path.push_back(SOUTH); break;
			case 8: path.push_back(SOUTHEAST); break;
			default: break;
		};
	}

	if (path.empty())
		return;

	addGameTask(&Game::playerAutoWalk, player->getID(), path);
}

void ProtocolGame::parseSetOutfit(NetworkMessage& msg)
{
	uint16_t lookType = msg.getU16();

	Outfit_t newOutfit;

	// only first 4 outfits
	uint8_t lastFemaleOutfit = 139;
	uint8_t lastMaleOutfit = 131;

	// if premium then all 7 outfits
	if (player->getSex() == PLAYERSEX_FEMALE && player->isPremium())
		lastFemaleOutfit = 143;
	else if (player->getSex() == PLAYERSEX_MALE && player->isPremium())
		lastMaleOutfit = 135;

	if ((player->getSex() == PLAYERSEX_FEMALE &&
		lookType >= 136 &&
		lookType <= lastFemaleOutfit) ||
		(player->getSex() == PLAYERSEX_MALE &&
		lookType >= 128 &&
		lookType <= lastMaleOutfit))
	{
		newOutfit.lookType = lookType;
		newOutfit.lookHead = msg.getByte();
		newOutfit.lookBody = msg.getByte();
		newOutfit.lookLegs = msg.getByte();
		newOutfit.lookFeet = msg.getByte();
	}

	addGameTask(&Game::playerChangeOutfit, player->getID(), newOutfit);
}

void ProtocolGame::parseUseItem(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t stackpos = msg.getByte();
	uint8_t index = msg.getByte();

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerUseItem, player->getID(), pos, stackpos, index, spriteId);
}

void ProtocolGame::parseUseItemEx(NetworkMessage& msg)
{
	Position fromPos = msg.getPosition();
	uint16_t fromSpriteId = msg.getU16();
	uint8_t fromStackPos = msg.getByte();
	Position toPos = msg.getPosition();
	uint16_t toSpriteId = msg.getU16();
	uint8_t toStackPos = msg.getByte();

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerUseItemEx, player->getID(), fromPos, fromStackPos, fromSpriteId, toPos, toStackPos, toSpriteId);
}

void ProtocolGame::parseBattleWindow(NetworkMessage &msg)
{
	Position fromPos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t fromStackPos = msg.getByte();
	uint32_t creatureId = msg.getU32();

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerUseBattleWindow, player->getID(), fromPos, fromStackPos, creatureId, spriteId);
}

void ProtocolGame::parseCloseContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.getByte();

	addGameTask(&Game::playerCloseContainer, player->getID(), cid);
}

void ProtocolGame::parseUpArrowContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.getByte();

	addGameTask(&Game::playerMoveUpContainer, player->getID(), cid);
}

void ProtocolGame::parseUpdateTile(NetworkMessage& msg)
{
	Position pos = msg.getPosition();

	//addGameTask(&Game::playerUpdateTile, player->getID(), pos);
}

void ProtocolGame::parseUpdateContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.getByte();

	addGameTask(&Game::playerUpdateContainer, player->getID(), cid);
}

void ProtocolGame::parseThrow(NetworkMessage& msg)
{
	Position fromPos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t fromStackpos = msg.getByte();
	Position toPos = msg.getPosition();
	const uint16_t count = msg.getU16();

	/*
	std::cout << "parseThrow: " << "from_x: " << (int)fromPos.x << ", from_y: " << (int)fromPos.y
	<<  ", from_z: " << (int)fromPos.z << ", item: " << (int)itemId << ", fromStackpos: "
	<< (int)fromStackpos << " to_x: " << (int)toPos.x << ", to_y: " << (int)toPos.y
	<<  ", to_z: " << (int)toPos.z
	<< ", count: " << (int)count << std::endl;
	*/

	if(toPos != fromPos){
		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION,
			&Game::playerMoveThing, player->getID(), fromPos, spriteId,
			fromStackpos, toPos, count);
	}
}

void ProtocolGame::parseLookAt(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t stackpos = msg.getByte();

/*
#ifdef __DEBUG__
	ss << "You look at x: " << x <<", y: " << y << ", z: " << z << " and see Item # " << itemId << ".";
#endif
*/

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerLookAt, player->getID(), pos, spriteId, stackpos);
}

void ProtocolGame::parseSay(NetworkMessage& msg)
{
	SpeakClasses type = (SpeakClasses)msg.getByte();

	std::string receiver;
	uint16_t channelId = 0;
	switch(type){
	case SPEAK_PRIVATE:
	case SPEAK_PRIVATE_RED:
	case SPEAK_RVR_ANSWER:
		receiver = msg.getString();
		break;
	case SPEAK_CHANNEL_Y:
	case SPEAK_CHANNEL_R1:
	case SPEAK_CHANNEL_R2:
		channelId = msg.getU16();
		break;
	default:
		break;
	}

	const std::string text = msg.getString();
	if(text.length() > 255)
		return;
	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSay, player->getID(), channelId, type, receiver, text);
}

void ProtocolGame::parseSayTargeted(NetworkMessage& msg)
{
	uint8_t targetType = msg.getByte(); // 1 position , 2 creature

	if (targetType == 0x01) { // position
		uint16_t x = msg.getU16();
		uint16_t y = msg.getU16();
		uint16_t z = msg.getU16();
		std::string text = msg.getString();

		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSayTargetPosition, player->getID(), Position(x,y,z), text);

	} else if (targetType == 0x02) { // creature
		uint32_t creatureId = msg.getU32();
		std::string text = msg.getString();

		addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSayTarget, player->getID(), creatureId, text);
	}
}

void ProtocolGame::parseFightModes(NetworkMessage& msg)
{
	uint8_t rawFightMode = msg.getByte(); //1 - offensive, 2 - balanced, 3 - defensive
	uint8_t rawChaseMode = msg.getByte(); //0 - stand while fighting, 1 - chase opponent
	uint8_t rawPickUpMode = msg.getByte(); //0 - no pickup, 1 - pickup ammo
	uint8_t rawSafeMode = msg.getByte();

	bool safeMode = (rawSafeMode == 1);

	chaseMode_t                chaseMode = CHASEMODE_STANDSTILL;
	if(rawChaseMode == 1)      chaseMode = CHASEMODE_FOLLOW;

	pickUpMode_t               pickUpMode = PICKUPMODE_OFF;
	if(rawPickUpMode==1)       pickUpMode = PICKUPMODE_AMMUNITION;

	fightMode_t                fightMode = FIGHTMODE_ATTACK;
	if(rawFightMode == 2)      fightMode = FIGHTMODE_BALANCED;
	else if(rawFightMode == 3) fightMode = FIGHTMODE_DEFENSE;

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSetFightModes, player->getID(), fightMode, chaseMode, pickUpMode, safeMode);
}

void ProtocolGame::parseAttack(NetworkMessage& msg)
{
	uint32_t creatureId = msg.getU32();

	addGameTask(&Game::playerSetAttackedCreature, player->getID(), creatureId);
}

void ProtocolGame::parseFollow(NetworkMessage& msg)
{
	uint32_t creatureId = msg.getU32();

	addGameTask(&Game::playerFollowCreature, player->getID(), creatureId);
}

void ProtocolGame::parseInviteToParty(NetworkMessage& msg)
{
	uint32_t creatureId = msg.getU32();

	addGameTask(&Game::playerInviteToParty, player->getID(), creatureId);
}

void ProtocolGame::parseJoinParty(NetworkMessage& msg)
{
	uint32_t creatureId = msg.getU32();

	addGameTask(&Game::playerJoinParty, player->getID(), creatureId);
}

void ProtocolGame::parseRevokePartyInvitation(NetworkMessage& msg)
{
	uint32_t creatureId = msg.getU32();

	addGameTask(&Game::playerRevokePartyInvitation, player->getID(), creatureId);
}

void ProtocolGame::parsePassPartyLeadership(NetworkMessage& msg)
{
	uint32_t creatureId = msg.getU32();

	addGameTask(&Game::playerPassPartyLeadership, player->getID(), creatureId);
}

void ProtocolGame::parseTextWindow(NetworkMessage& msg)
{
	uint32_t windowTextId = msg.getU32();
	const std::string newText = msg.getString();

	addGameTask(&Game::playerWriteItem, player->getID(), windowTextId, newText);
}

void ProtocolGame::parseHouseWindow(NetworkMessage &msg)
{
	uint8_t doorId = msg.getByte();
	uint32_t id = msg.getU32();
	const std::string text = msg.getString();

	addGameTask(&Game::playerUpdateHouseWindow, player->getID(), doorId, id, text);
}

void ProtocolGame::parseRequestTrade(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t stackpos = msg.getByte();
	uint32_t playerId = msg.getU32();

	addGameTask(&Game::playerRequestTrade, player->getID(), pos, stackpos, playerId, spriteId);
}

void ProtocolGame::parseLookInTrade(NetworkMessage& msg)
{
	bool counterOffer = (msg.getByte() == 0x01);
	uint8_t index = msg.getByte();

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerLookInTrade, player->getID(), counterOffer, index);
}

void ProtocolGame::parseCloseTrade()
{
	addGameTask(&Game::playerCloseTrade, player->getID());
}

void ProtocolGame::parseSellItem(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t stackPos = msg.getByte();

	uint32_t creatureId = msg.getU32();

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerSellItem, player->getID(), creatureId, pos, stackPos, spriteId);
}

void ProtocolGame::parseBuySpells(NetworkMessage& msg)
{
	uint16_t spellCount = msg.getU16();
	std::vector<std::string> spells;

	for(int i = spellCount; i-->0;) {
		spells.push_back(msg.getString());
	}

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerBuySpells, player->getID(), spells);
}

void ProtocolGame::parseAddVip(NetworkMessage& msg)
{
	const std::string name = msg.getString();
	if(name.size() > 32)
		return;

	addGameTask(&Game::playerRequestAddVip, player->getID(), name);
}

void ProtocolGame::parseRemoveVip(NetworkMessage& msg)
{
	uint32_t guid = msg.getU32();

	addGameTask(&Game::playerRequestRemoveVip, player->getID(), guid);
}

void ProtocolGame::parseRotateItem(NetworkMessage& msg)
{
	Position pos = msg.getPosition();
	uint16_t spriteId = msg.getU16();
	uint8_t stackpos = msg.getByte();

	addGameTaskTimed(DISPATCHER_TASK_EXPIRATION, &Game::playerRotateItem, player->getID(), pos, stackpos, spriteId);
}

void ProtocolGame::parseViolationWindow(NetworkMessage& msg)
{
	std::string target = msg.getString();
	uint8_t reason = msg.getByte();
	violationAction_t action = (violationAction_t)msg.getByte();
	std::string comment = msg.getString();

	uint16_t statementId = msg.getU16();
	uint16_t channelId = msg.getU16();
	bool ipBanishment = msg.getByte() != 0;
	addGameTask(&Game::playerViolationWindow, player->getID(), target, reason, action, comment, statementId, channelId, ipBanishment);
}

void ProtocolGame::parseBugReport(NetworkMessage& msg)
{
	std::string comment = msg.getString();
	addGameTask(&Game::playerReportBug, player->getID(), comment);
}

void ProtocolGame::parseDebugAssert(NetworkMessage& msg)
{
	if(!g_config.getNumber(ConfigManager::SAVE_CLIENT_DEBUG_ASSERTIONS)){
		return;
	}

	//only accept 1 report each time
	if(m_debugAssertSent){
		return;
	}
	m_debugAssertSent = true;

	std::string assertLine = msg.getString();
	std::string report_date = msg.getString();
	std::string description = msg.getString();
	std::string comment = msg.getString();

	//write it in the assertions file
	std::ofstream of("client_assertions.txt", std::ios_base::app);
	if(of){
		char today[32];
		formatDate(time(NULL), today);

		of <<
			"-----" << today << " - " << player->getName() << " (" << convertIPToString(player->getIP()) <<
			assertLine << std::endl <<
			report_date << std::endl <<
			description << std::endl <<
			comment << std::endl;
	}
}

//********************** Send methods  *******************************
void ProtocolGame::sendPingBack()
{
	NetworkMessage msg;
	msg.addByte(0x1E);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendPing()
{
	NetworkMessage msg;
	msg.addByte(0x1D);
	writeToOutputBuffer(msg);
}


void ProtocolGame::sendOpenPrivateChannel(const std::string& receiver)
{
	NetworkMessage msg;
	msg.addByte(0xAD);
	msg.addString(receiver);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCreatureOutfit(const Creature* creature, const Outfit_t& outfit)
{
	if(canSee(creature)){
		NetworkMessage msg;
		msg.addByte(0x8E);
		msg.addU32(creature->getID());
		addCreatureOutfit(msg, creature, outfit);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendCreatureLight(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage msg;
		addCreatureLight(msg, creature);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendWorldLight(const LightInfo& lightInfo)
{
	NetworkMessage msg;
	addWorldLight(msg, lightInfo);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCreatureSkull(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage msg;
		msg.addByte(0x90);
		msg.addU32(creature->getID());
		#ifdef __SKULLSYSTEM__
		msg.addByte(player->getSkullClient(creature->getPlayer()));
		#else
		msg.addByte(SKULL_NONE);
		#endif
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendCreatureShield(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage msg;
		msg.addByte(0x91);
		msg.addU32(creature->getID());
		msg.addByte(player->getPartyShield(creature->getPlayer()));
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendCreatureSquare(const Creature* creature, SquareColor_t color)
{
	if(canSee(creature)){
		NetworkMessage msg;
		msg.addByte(0x86);
		msg.addU32(creature->getID());
		msg.addByte((uint8_t)color);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendStats()
{
	NetworkMessage msg;
	addPlayerStats(msg);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendTextMessage(MessageClasses mclass, const std::string& message)
{
	NetworkMessage msg;
	addTextMessage(msg, mclass, message);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendClosePrivate(uint16_t channelId)
{
	NetworkMessage msg;
	msg.addByte(0xB3);
	msg.addU16(channelId);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName)
{
	NetworkMessage msg;
	msg.addByte(0xB2);
	msg.addU16(channelId);
	msg.addString(channelName);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendChannelsDialog()
{
	NetworkMessage msg;
	ChannelList list;
	list = g_chat.getChannelList(player);

	msg.addByte(0xAB);
	msg.addByte(list.size());

	while(list.size()){
		ChatChannel *channel;
		channel = list.front();
		list.pop_front();

		msg.addU16(channel->getId());
		msg.addString(channel->getName());
	}
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendChannel(uint16_t channelId, const std::string& channelName)
{
	NetworkMessage msg;
	msg.addByte(0xAC);
	msg.addU16(channelId);
	msg.addString(channelName);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendRuleViolationsChannel(uint16_t channelId)
{
	NetworkMessage msg;
	msg.addByte(0xAE);
	msg.addU16(channelId);
	RuleViolationsMap::const_iterator it = g_game.getRuleViolations().begin();
	for( ; it != g_game.getRuleViolations().end(); ++it){
		RuleViolation& rvr = *it->second;
		if(rvr.isOpen && rvr.reporter){
			addCreatureSpeak(msg, rvr.reporter, SPEAK_RVR_CHANNEL, rvr.text, channelId, rvr.time);
		}
	}
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendRemoveReport(const std::string& name)
{
	NetworkMessage msg;
	msg.addByte(0xAF);
	msg.addString(name);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendRuleViolationCancel(const std::string& name)
{
	NetworkMessage msg;
	msg.addByte(0xB0);
	msg.addString(name);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendLockRuleViolation()
{
	NetworkMessage msg;
	msg.addByte(0xB1);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendIcons(PlayerIconsData iconsData)
{
	NetworkMessage msg;
	msg.addByte(0xA2);
	msg.addU16(iconsData.icons);

	msg.addByte(iconsData.modes.size());

	for (std::size_t i = 0; i < iconsData.modes.size(); ++i) {
		msg.addByte(iconsData.modes[i]);
		msg.addU16(iconsData.ticks[i]);
	}

	writeToOutputBuffer(msg);
}

void ProtocolGame::sendTargetRequirement(const uint8_t &reason, const std::string &text)
{
	NetworkMessage msg;
	msg.addByte(0x34);
	msg.addByte(reason);
	msg.addString(text);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendSpellTree()
{
	NetworkMessage msg;
	addSpellTree(msg);
	addPlayerBalance(msg);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendContainer(uint32_t cid, const Container* container, bool hasParent)
{
	NetworkMessage msg;
	msg.addByte(0x6E);
	msg.addByte(cid);
	msg.addItemId(container);
	msg.addByte(0); // rank
	msg.addString(container->getName());
	msg.addByte(container->capacity());
	msg.addByte(hasParent ? 0x01 : 0x00);
	if(container->size() > 255){
		msg.addByte(255);
	}
	else{
		msg.addByte(container->size());
	}

	ItemList::const_iterator cit;
	uint32_t i = 0;
	for(cit = container->getItems(); cit != container->getEnd() && i < 255; ++cit, ++i){
		msg.addItem(*cit);
	}
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendTradeItemRequest(const Player* player, const Item* item, bool ack)
{
	NetworkMessage msg;
	if(ack){
		msg.addByte(0x7D);
	}
	else{
		msg.addByte(0x7E);
	}

	msg.addString(player->getName());

	if(const Container* tradeContainer = item->getContainer()){
		std::list<const Container*> listContainer;
		ItemList::const_iterator it;
		Container* tmpContainer = NULL;

		listContainer.push_back(tradeContainer);

		std::list<const Item*> listItem;
		listItem.push_back(tradeContainer);

		while(!listContainer.empty()) {
			const Container* container = listContainer.front();
			listContainer.pop_front();

			for(it = container->getItems(); it != container->getEnd(); ++it){
				if((tmpContainer = (*it)->getContainer())){
					listContainer.push_back(tmpContainer);
				}

				listItem.push_back(*it);
			}
		}

		msg.addByte(listItem.size());
		while(!listItem.empty()) {
			const Item* item = listItem.front();
			listItem.pop_front();
			msg.addItem(item);
		}
	}
	else {
		msg.addByte(1);
		msg.addItem(item);
	}
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCloseTrade()
{
	NetworkMessage msg;
	msg.addByte(0x7F);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCloseContainer(uint32_t cid)
{
	NetworkMessage msg;
	msg.addByte(0x6F);
	msg.addByte(cid);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCreatureTurn(const Creature* creature, uint32_t stackpos)
{
	if(stackpos < 10){
		if(canSee(creature)){
			NetworkMessage msg;
			msg.addByte(0x6B);
			msg.addPosition(creature->getPosition());
			msg.addByte(stackpos);
			msg.addU16(0x63); /*99*/
			msg.addU32(creature->getID());
			msg.addByte(creature->getDirection());
			writeToOutputBuffer(msg);
		}
	}
}

void ProtocolGame::sendCreatureSay(const Creature* creature, SpeakClasses type, const std::string& text)
{
	NetworkMessage msg;
	addCreatureSpeak(msg, creature, type, text, 0);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendToChannel(const Creature * creature, SpeakClasses type, const std::string& text, uint16_t channelId, uint32_t time /*= 0*/)
{
	NetworkMessage msg;
	addCreatureSpeak(msg, creature, type, text, channelId, time);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendStartSellingTransaction(const Npc* npc)
{
	NetworkMessage msg;
	msg.addByte(0xA9);
	msg.addU32(npc->getID());
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendNpcFocusLost(const Npc* npc)
{
	NetworkMessage msg;
	msg.addByte(0xB9);
	msg.addU32(npc->getID());
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendNpcFocus(const Npc* npc)
{
	NetworkMessage msg;
	msg.addByte(0xBA);
	msg.addU32(npc->getID());
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCancel(const std::string& message)
{
	NetworkMessage msg;
	addTextMessage(msg, MSG_STATUS_SMALL, message);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendCancelTarget()
{
	NetworkMessage msg;
	msg.addByte(0xA3);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendChangeSpeed(const Creature* creature, uint32_t speed)
{
	if(canSee(creature)){
		NetworkMessage msg;
		msg.addByte(0x8F);
		msg.addU32(creature->getID());
		msg.addU16(speed);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendCancelWalk()
{
	NetworkMessage msg;
	msg.addByte(0xB5);
	msg.addByte(player->getDirection());
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendSkills()
{
	NetworkMessage msg;
	addPlayerSkills(msg);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendPlayerInfo()
{
	NetworkMessage msg;
	addPlayerInfo(msg);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendPlayerBalance()
{
	NetworkMessage msg;
	addPlayerBalance(msg);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendDistanceShoot(const Position& from, const Position& to, uint8_t type)
{
	if(canSee(from) || canSee(to)){
		NetworkMessage msg;
		addDistanceShoot(msg, from, to, type);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendMagicEffect(const Position& pos, uint8_t type)
{
	if(canSee(pos)){
		NetworkMessage msg;
		addMagicEffect(msg, pos, type);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendAnimatedText(const Position& pos, uint8_t color, std::string text)
{
	if(canSee(pos)){
		NetworkMessage msg;
		addAnimatedText(msg, pos, color, text);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendCreatureHealth(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage msg;
		addCreatureHealth(msg, creature);
		writeToOutputBuffer(msg);
	}
}

//tile
void ProtocolGame::sendAddTileItem(const Tile* tile, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(canSee(pos)){
		NetworkMessage msg;
		addTileItem(msg, pos, stackpos, item);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendUpdateTileItem(const Tile* tile, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(canSee(pos)){
		NetworkMessage msg;
		updateTileItem(msg, pos, stackpos, item);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos)
{
	if(canSee(pos)){
		NetworkMessage msg;
		removeTileItem(msg, pos, stackpos);
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendUpdateTile(const Tile* tile, const Position& pos)
{
	if(canSee(pos)){
		NetworkMessage msg;

		msg.addByte(0x69);
		msg.addPosition(pos);

		if(tile){
			getTileDescription(msg, tile);
			msg.addByte(0);
			msg.addByte(0xFF);
		}
		else{
			msg.addByte(0x01);
			msg.addByte(0xFF);
		}

		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendAddCreature(const Creature* creature, const Position& pos, uint32_t stackpos, bool isLoggingIn)
{
	if(canSee(creature)){
		NetworkMessage msg;

		if(creature == player){
			msg.addByte(0x0A);
			msg.addU32(player->getID());
			msg.addU16(0x0032); // Related to client-side drawing speed
			msg.addByte(player->hasFlag(PlayerFlag_CanReportBugs));

			if (player->hasFlag(PlayerFlag_CanAnswerRuleViolations)){
				msg.addByte(0x0B);
				for (uint8_t i = 0; i < 32; i++) {
					msg.addByte(0xFF);
				}
			}

			addServerParams(msg);
			addMapDescription(msg, pos);

			writeToOutputBuffer(msg);
			msg = NetworkMessage();

			if (isLoggingIn){
				addMagicEffect(msg, player->getPosition(), NM_ME_TELEPORT);
			}

			addInventoryItem(msg, SLOT_HEAD, player->getInventoryItem(SLOT_HEAD));
			addInventoryItem(msg, SLOT_NECKLACE, player->getInventoryItem(SLOT_NECKLACE));
			addInventoryItem(msg, SLOT_BACKPACK, player->getInventoryItem(SLOT_BACKPACK));
			addInventoryItem(msg, SLOT_ARMOR, player->getInventoryItem(SLOT_ARMOR));
			addInventoryItem(msg, SLOT_RIGHT, player->getInventoryItem(SLOT_RIGHT));
			addInventoryItem(msg, SLOT_LEFT, player->getInventoryItem(SLOT_LEFT));
			addInventoryItem(msg, SLOT_LEGS, player->getInventoryItem(SLOT_LEGS));
			addInventoryItem(msg, SLOT_FEET, player->getInventoryItem(SLOT_FEET));
			addInventoryItem(msg, SLOT_RING, player->getInventoryItem(SLOT_RING));
			addInventoryItem(msg, SLOT_AMMO, player->getInventoryItem(SLOT_AMMO));

			addPlayerInfo(msg);
			addPlayerStats(msg);
			addPlayerSkills(msg);
			addPlayerBalance(msg);
			addSpellTree(msg);

			//gameworld light-settings
			LightInfo lightInfo;
			g_game.getWorldLightInfo(lightInfo);
			addWorldLight(msg, lightInfo);

			//player light level
			addCreatureLight(msg, creature);

			for(VIPListSet::iterator it = player->VIPList.begin(); it != player->VIPList.end(); ++it){
				std::string vip_name;
				if(IOPlayer::instance()->getNameByGuid((*it), vip_name)){
					Player *p = g_game.getPlayerByName(vip_name);
					bool online = (p && (!p->isGmInvisible() || player->canSeeGmInvisible(p)));
					addSendVIP(msg,(*it), vip_name, online);
				}
			}
			
			sendIcons(player->getIcons());
		}
		else{
			addTileCreature(msg, pos, stackpos, creature);

			if (isLoggingIn){
				if(creature->getPlayer()){
					addMagicEffect(msg, creature->getPosition(), NM_ME_TELEPORT);
				}
			}
		}

		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendRemoveCreature(const Creature* creature, const Position& pos, uint32_t stackpos, bool isLogout)
{
	if(canSee(pos)){
		NetworkMessage msg;
		removeTileItem(msg, pos, stackpos);

		if(isLogout){
			addMagicEffect(msg, pos, NM_ME_PUFF);
		}
		writeToOutputBuffer(msg);
	}
}

void ProtocolGame::sendMoveCreature(const Creature* creature, const Tile* newTile, const Position& newPos,
	uint32_t newStackPos, const Tile* oldTile, const Position& oldPos, uint32_t oldStackPos, bool teleport)
{
	if(creature == player){
		NetworkMessage msg;

		if(teleport || oldStackPos >= 10){
			removeTileItem(msg, oldPos, oldStackPos);
			addMapDescription(msg, newPos);

		} else {
			msg.priorize();
			if(oldPos.z == 7 && newPos.z >= 8){
				removeTileItem(msg, oldPos, oldStackPos);
			}
			else{
				msg.addByte(0x6D);
				msg.addPosition(oldPos);
				msg.addByte(oldStackPos);
				msg.addPosition(newPos);
			}

			//floor change down
			if(newPos.z > oldPos.z){
				moveDownCreature(msg, creature, newPos, oldPos, oldStackPos);
			}
			//floor change up
			else if(newPos.z < oldPos.z){
				moveUpCreature(msg, creature, newPos, oldPos, oldStackPos);
			}


			int8_t dy = oldPos.y - newPos.y;
			int8_t dx = oldPos.x - newPos.x;

			if (dy > 0) { // north, for old x
				int8_t y = 1;
				do {
					msg.addByte(0x65);
					getMapDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - y - Map::clientViewportY, newPos.z, Map::clientViewportW, 1);
				} while (dy >= ++y);
			}
			else if (dy < 0) { // south, for old x
				int8_t y = -1;
				do {
					msg.addByte(0x67);
					getMapDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - y + 1 + Map::clientViewportY, newPos.z, Map::clientViewportW, 1);
				} while (--y >= dy);
			}

			if (dx < 0) { // east, [with new y]
				int8_t x = -1;
				do {
					msg.addByte(0x66);
					getMapDescription(msg, oldPos.x - x +  1 + Map::clientViewportX, newPos.y - Map::clientViewportY, newPos.z, 1, Map::clientViewportH);
				} while (--x >= dx);
			}
			else if (dx > 0) { // west, [with new y]
				int8_t x = 1;
				do {
					msg.addByte(0x68);
					getMapDescription(msg, oldPos.x - x - Map::clientViewportX, newPos.y - Map::clientViewportY, newPos.z, 1, Map::clientViewportH);
				} while (dx >= ++x);
			}
		}

		writeToOutputBuffer(msg);
	}
	else if(canSee(oldPos) && canSee(newPos)){
		if(player->canSeeCreature(creature)){
			NetworkMessage msg;

			if(teleport || (oldPos.z == 7 && newPos.z >= 8) || oldStackPos >= 10){
				removeTileItem(msg, oldPos, oldStackPos);
				addTileCreature(msg, newPos, newStackPos, creature);
			}
			else{
				msg.addByte(0x6D);
				msg.addPosition(oldPos);
				msg.addByte(oldStackPos);
				msg.addPosition(newPos);
			}
			writeToOutputBuffer(msg);
		}
	}
	else if(canSee(oldPos)){
		if(player->canSeeCreature(creature)){
			NetworkMessage msg;
			removeTileItem(msg, oldPos, oldStackPos);
			writeToOutputBuffer(msg);
		}
	}
	else if(canSee(newPos)){
		if(player->canSeeCreature(creature)){
			NetworkMessage msg;
			addTileCreature(msg, newPos, newStackPos, creature);
			writeToOutputBuffer(msg);
		}
	}
}

//inventory
void ProtocolGame::sendAddInventoryItem(slots_t slot, const Item* item)
{
	NetworkMessage msg;
	addInventoryItem(msg, slot, item);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendUpdateInventoryItem(slots_t slot, const Item* item)
{
	NetworkMessage msg;
	updateInventoryItem(msg, slot, item);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendRemoveInventoryItem(slots_t slot)
{
	NetworkMessage msg;
	removeInventoryItem(msg, slot);
	writeToOutputBuffer(msg);
}

//containers
void ProtocolGame::sendAddContainerItem(uint8_t cid, const Item* item)
{
	NetworkMessage msg;
	addContainerItem(msg, cid, item);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendUpdateContainerItem(uint8_t cid, uint8_t slot, const Item* item)
{
	NetworkMessage msg;
	updateContainerItem(msg, cid, slot, item);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendRemoveContainerItem(uint8_t cid, uint8_t slot)
{
	NetworkMessage msg;
	removeContainerItem(msg, cid, slot);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendTextWindow(uint32_t windowTextId, Item* item, uint16_t maxlen, bool canWrite)
{
	NetworkMessage msg;
	msg.addByte(0x96);
	msg.addU32(windowTextId);
	msg.addItemId(item);
	if(canWrite){
		msg.addU16(maxlen);
		msg.addString(item->getText());
	}
	else{
		msg.addU16(item->getText().size());
		msg.addString(item->getText());
	}

	const std::string& writer = item->getWriter();
	if(writer.size()){
		msg.addString(writer);
	}
	else{
		msg.addString("");
	}
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendTextWindow(uint32_t windowTextId, uint32_t itemId, const std::string& text)
{
	NetworkMessage msg;

	msg.addByte(0x96);
	msg.addU32(windowTextId);
	msg.addItemId(itemId);

	msg.addU16(text.size());
	msg.addString(text);
	
	msg.addString("");

	writeToOutputBuffer(msg);
}

void ProtocolGame::sendHouseWindow(uint32_t windowTextId, House* _house,
	uint32_t listId, const std::string& text)
{
	NetworkMessage msg;

	msg.addByte(0x97);
	msg.addByte(0);
	msg.addU32(windowTextId);
	msg.addString(text);

	writeToOutputBuffer(msg);
}

void ProtocolGame::sendOutfitWindow()
{
	NetworkMessage msg;

	msg.addByte(0xC8);
	addCreatureOutfit(msg, player, player->getDefaultOutfit());

	switch (player->getSex()) {
	case PLAYERSEX_FEMALE:
		msg.addU16(136);
		if (player->isPremium())
			msg.addU16(143);
		else
			msg.addU16(139);

		break;
	case PLAYERSEX_MALE:
		msg.addU16(128);
		if (player->isPremium())
			msg.addU16(135);
		else
			msg.addU16(131);

		break;
	case 2:
		msg.addU16(160);
		msg.addU16(160);

		break;
	default:
		msg.addU16(128);
		msg.addU16(135);
	}

	writeToOutputBuffer(msg);
}

void ProtocolGame::sendVIPLogIn(uint32_t guid)
{
	NetworkMessage msg;
	msg.addByte(0xD3);
	msg.addU32(guid);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendVIPLogOut(uint32_t guid)
{
	NetworkMessage msg;
	msg.addByte(0xD4);
	msg.addU32(guid);
	writeToOutputBuffer(msg);
}

void ProtocolGame::sendVIP(uint32_t guid, const std::string& name, bool isOnline)
{
	NetworkMessage msg;
	addSendVIP(msg, guid, name, isOnline);
	writeToOutputBuffer(msg);
}

////////////// Add common messages
void ProtocolGame::addServerParams(NetworkMessage &msg)
{
	msg.addByte(0x0C);
	int diagonalCost = g_config.getNumber(ConfigManager::DIAGONAL_WALK_FACTOR);
	msg.addU16(diagonalCost);
	msg.addByte(Map::clientViewportX);
	msg.addByte(Map::clientViewportY);
}

void ProtocolGame::addMapDescription(NetworkMessage &msg, const Position& pos)
{
	msg.addByte(0x64);
	msg.addPosition(player->getPosition());
	getMapDescription(msg, pos.x - Map::clientViewportX, pos.y - Map::clientViewportY, pos.z, Map::clientViewportW, Map::clientViewportH);
}

void ProtocolGame::addTextMessage(NetworkMessage &msg, MessageClasses mclass, const std::string& message)
{
	msg.addByte(0xB4);
	msg.addByte(mclass);
	msg.addString(message);
}

void ProtocolGame::addAnimatedText(NetworkMessage &msg, const Position& pos,
	uint8_t color, const std::string& text)
{
	msg.addByte(0x84);
	msg.addPosition(pos);
	msg.addByte(color);
	msg.addString(text);
}

void ProtocolGame::addMagicEffect(NetworkMessage &msg,const Position& pos, uint8_t type)
{
	msg.addByte(0x83);
	msg.addPosition(pos);
	msg.addByte(type + 1);
}

void ProtocolGame::addDistanceShoot(NetworkMessage &msg, const Position& from, const Position& to, uint8_t type)
{
	msg.addByte(0x85);
	msg.addPosition(from);
	msg.addPosition(to);
	msg.addByte(type + 1);
}

void ProtocolGame::addCreature(NetworkMessage &msg,const Creature* creature, bool known, uint32_t remove)
{
	if(known){
		msg.addU16(0x62);
		msg.addU32(creature->getID());
	}
	else{
		msg.addU16(0x61);
		msg.addU32(remove);
		msg.addU32(creature->getID());
		msg.addString(creature->getName());
	}

	msg.addByte((int32_t)std::ceil(((float)creature->getHealth()) * 100 / std::max(creature->getMaxHealth(), (int32_t)1)));
	msg.addByte((uint8_t)creature->getDirection());

	if(creature->isInvisible() ||
		(creature->getPlayer() && creature->getPlayer()->isGmInvisible()))
	{
		if (player->canSeeInvisibility() && player != creature)
		{
			addCreatureOutfit(msg, creature, creature->getCurrentOutfit());
		}
		else {
			msg.addU16(0);
			msg.addU16(0);
		}
		/*static Outfit_t outfit;
		addCreatureOutfit(msg, creature, outfit);*/
	}
	else{
		addCreatureOutfit(msg, creature, creature->getCurrentOutfit());
	}

	LightInfo lightInfo;
	creature->getCreatureLight(lightInfo);
	msg.addByte(lightInfo.level);
	msg.addByte(lightInfo.color);

	msg.addU16(creature->getStepSpeed());
#ifdef __SKULLSYSTEM__
	msg.addByte(player->getSkullClient(creature->getPlayer()));
#else
	msg.addByte(SKULL_NONE);
#endif
	msg.addByte(player->getPartyShield(creature->getPlayer()));
}

void ProtocolGame::addPlayerInfo(NetworkMessage &msg)
{
	msg.addByte(0x9F);

	msg.addByte(player->isPremium());

	Vocation* vocation = player->getVocation();
	msg.addByte(vocation->getBaseVocation()); // vocation
	//msg.addByte(vocation->getPromotion()==0); // isPromoted
	//msg.addByte(vocation->getRebirth()==0); // isReborn

	// can't send spells because they have no numeric Id...
	msg.addU16(0); //spellCount
	//for(int i=0;i < spellCount; ++i)
		//msg.addByte(spells[i]->id);
}

void ProtocolGame::addPlayerBalance(NetworkMessage &msg)
{
	msg.addByte(0x36);
	msg.addU32(player->balance);
}

void ProtocolGame::addPlayerStats(NetworkMessage &msg)
{
	msg.addByte(0xA0);
	msg.addU16(player->getHealth());
	msg.addU16(player->getPlayerInfo(PLAYERINFO_MAXHEALTH));
	msg.addU32((uint32_t)(player->getFreeCapacity()*100));
	uint64_t experience = player->getExperience();
	if(experience <= 0x7FFFFFFF){
		msg.addU32(player->getExperience());
	}
	else{
		msg.addU32(0x00); //Client debugs after 2,147,483,647 exp
	}
	msg.addU16(player->getPlayerInfo(PLAYERINFO_LEVEL));
	msg.addByte(player->getPlayerInfo(PLAYERINFO_LEVELPERCENT));

	msg.addU16(player->getMana());
	msg.addU16(player->getPlayerInfo(PLAYERINFO_MAXMANA));

	msg.addByte(player->getMagicLevel());
	msg.addByte(player->getPlayerInfo(PLAYERINFO_MAGICLEVELPERCENT));
}

void ProtocolGame::addPlayerSkills(NetworkMessage &msg)
{
	msg.addByte(0xA1);
	msg.addByte(player->getSkillLevel(SKILL_FIST));
	msg.addByte(player->getSkill(SKILL_FIST).percent);
	msg.addByte(player->getSkillLevel(SKILL_STRIKE));
	msg.addByte(player->getSkill(SKILL_STRIKE).percent);
	msg.addByte(player->getSkillLevel(SKILL_PIERCE));
	msg.addByte(player->getSkill(SKILL_PIERCE).percent);
	msg.addByte(player->getSkillLevel(SKILL_SLASH));
	msg.addByte(player->getSkill(SKILL_SLASH).percent);
	msg.addByte(player->getSkillLevel(SKILL_DIST));
	msg.addByte(player->getSkill(SKILL_DIST).percent);
	msg.addByte(player->getSkillLevel(SKILL_SHIELD));
	msg.addByte(player->getSkill(SKILL_SHIELD).percent);
	msg.addByte(player->getSkillLevel(SKILL_FISH));
	msg.addByte(player->getSkill(SKILL_FISH).percent);
}

void ProtocolGame::addCreatureSpeak(NetworkMessage& msg, const Creature* creature,
	SpeakClasses type, std::string text, uint16_t channelId, uint32_t time /*= 0*/)
{
	msg.addByte(0xAA);
	msg.addU32(++Player::channelStatementGuid);

	Player::channelStatementMap[Player::channelStatementGuid] = text;

	if (type != SPEAK_CHANNEL_R2){
		if (type != SPEAK_RVR_ANSWER){
			if(creature)
			  msg.addString(creature->getName());
			else
			  msg.addString(""); // anonymous = no name
		}
		else {
			msg.addString("Gamemaster");
		}
	}
	else {
		msg.addString("");
	}

	msg.addByte(type);
	switch(type){
		case SPEAK_SAY:
		case SPEAK_WHISPER:
		case SPEAK_YELL:
		case SPEAK_CAST:
		case SPEAK_MONSTER_SAY:
		case SPEAK_MONSTER_YELL:
		case SPEAK_CHANNEL_NPC:
			assert(creature);
			msg.addPosition(creature->getPosition());
			break;
		case SPEAK_CHANNEL_Y:
		case SPEAK_CHANNEL_R1:
		case SPEAK_CHANNEL_R2:
		case SPEAK_CHANNEL_O:
			msg.addU16(channelId);
			break;
		case SPEAK_RVR_CHANNEL: {
			uint32_t t = (OTSYS_TIME() / 1000) & 0xFFFFFFFF;
			msg.addU32(t - time);
		} break;
		default:
			break;
	}

	msg.addString(text);
}

void ProtocolGame::addCreatureHealth(NetworkMessage &msg,const Creature* creature)
{
	msg.addByte(0x8C);
	msg.addU32(creature->getID());
	msg.addByte((int32_t)std::ceil(((float)creature->getHealth()) * 100 / std::max(creature->getMaxHealth(), (int32_t)1)));
}

void ProtocolGame::addCreatureOutfit(NetworkMessage &msg, const Creature* creature, const Outfit_t& outfit)
{
	msg.addU16(outfit.lookType);
	if(outfit.lookType != 0){
		msg.addByte(outfit.lookHead);
		msg.addByte(outfit.lookBody);
		msg.addByte(outfit.lookLegs);
		msg.addByte(outfit.lookFeet);
	}
	else if(outfit.lookTypeEx != 0){
		msg.addItemId(outfit.lookTypeEx);
	}
	else{
		msg.addU16(outfit.lookTypeEx);
	}
}

void ProtocolGame::addWorldLight(NetworkMessage &msg, const LightInfo& lightInfo)
{
	msg.addByte(0x82);
	msg.addByte(lightInfo.level);
	msg.addByte(lightInfo.color);
}

void ProtocolGame::addCreatureLight(NetworkMessage &msg, const Creature* creature)
{
	LightInfo lightInfo;
	creature->getCreatureLight(lightInfo);
	msg.addByte(0x8D);
	msg.addU32(creature->getID());
	msg.addByte(lightInfo.level);
	msg.addByte(lightInfo.color);
}

void ProtocolGame::addSpellTree(NetworkMessage &msg)
{
	msg.addByte(0x35);

	std::list<InstantSpell*> spells = g_spells->getInstantSpells(player);
	std::list<SpellSet*> spellsets = g_spellsets.getSpellSets(player);

	msg.addU16(spells.size() + spellsets.size());

	std::list<InstantSpell*>::iterator it;
	for (it = spells.begin(); it != spells.end(); ++it){
		InstantSpell* instantSpell = (*it);
		msg.addByte(instantSpell->getType());
		msg.addByte(instantSpell->canCast(player)? 1 : 0);

		msg.addString(instantSpell->getName());
		msg.addString(instantSpell->getWords());
		msg.addByte(instantSpell->getLevel());
		msg.addByte(instantSpell->getMagicLevel());
		msg.addU16(instantSpell->getPrice());
		msg.addU16(instantSpell->getMana());

		msg.addByte(0); // hasn't more
	}

	Vocation* vocation = player->getVocation();

	std::list<SpellSet*>::iterator it2;
	for (it2 = spellsets.begin(); it2 != spellsets.end(); ++it2){
		SpellSet* spellSet = (*it2);
		InstantSpell* instantSpell = spellSet->getFirstElement();

		msg.addByte(instantSpell->getType());
		uint8_t level = 0;
		while (instantSpell) {
			if (instantSpell->availableForVocation(vocation, true)) {
				if (instantSpell->canCast(player))
					level++;
				else
					break;
			}
			instantSpell = instantSpell->getNextSpell();
		}
		msg.addByte(level);

		instantSpell = spellSet->getFirstElement();
		while (instantSpell) {
			msg.addString(instantSpell->getName());
			msg.addString(instantSpell->getWords());
			msg.addByte(instantSpell->getLevel());
			msg.addByte(instantSpell->getMagicLevel());
			msg.addU16(instantSpell->getPrice());
			msg.addU16(instantSpell->getMana());

			do {
				instantSpell = instantSpell->getNextSpell();
			} while(instantSpell && !instantSpell->availableForVocation(vocation, true));

			if (instantSpell) msg.addByte(1); // has more
		}
		msg.addByte(0); // hasn't more
	}
}

//tile
void ProtocolGame::addTileItem(NetworkMessage &msg, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(stackpos < 10){
		msg.addByte(0x6A);
		msg.addPosition(pos);
		msg.addByte(stackpos);
		msg.addItem(item);
	}
}

void ProtocolGame::addTileCreature(NetworkMessage &msg, const Position& pos, uint32_t stackpos, const Creature* creature)
{
	//if(stackpos < 10){
		msg.addByte(0x6A);
		msg.addPosition(pos);
		msg.addByte(stackpos);

		bool known;
		uint32_t removedKnown;
		checkCreatureAsKnown(creature->getID(), known, removedKnown);
		addCreature(msg, creature, known, removedKnown);
	//}
}

void ProtocolGame::updateTileItem(NetworkMessage &msg, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(stackpos < 10){
		msg.addByte(0x6B);
		msg.addPosition(pos);
		msg.addByte(stackpos);
		msg.addItem(item);
	}
}

void ProtocolGame::removeTileItem(NetworkMessage &msg, const Position& pos, uint32_t stackpos)
{
	if(stackpos < 10){
		msg.addByte(0x6C);
		msg.addPosition(pos);
		msg.addByte(stackpos);
	}
}

void ProtocolGame::moveUpCreature(NetworkMessage &msg, const Creature* creature, const Position& newPos, const Position& oldPos, uint32_t oldStackPos)
{
	if(creature == player){
		//floor change up
		msg.addByte(0xBE);

		//going to surface
		if(newPos.z == 7){
			int32_t skip = -1;
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, 5, Map::clientViewportW, Map::clientViewportH, 3, skip); //(floor 7 and 6 already set)
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, 4, Map::clientViewportW, Map::clientViewportH, 4, skip);
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, 3, Map::clientViewportW, Map::clientViewportH, 5, skip);
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, 2, Map::clientViewportW, Map::clientViewportH, 6, skip);
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, 1, Map::clientViewportW, Map::clientViewportH, 7, skip);
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, 0, Map::clientViewportW, Map::clientViewportH, 8, skip);

			if(skip >= 0){
				msg.addByte(skip);
				msg.addByte(0xFF);
			}
		}
		//underground, going one floor up (still underground)
		else if(newPos.z > 7){
			int32_t skip = -1;
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, oldPos.z - 3, Map::clientViewportW, Map::clientViewportH, 3, skip);

			if(skip >= 0){
				msg.addByte(skip);
				msg.addByte(0xFF);
			}
		}

		//moving up a floor up makes us out of sync
		//west
		msg.addByte(0x68);
		getMapDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y + 1 - Map::clientViewportY, newPos.z, 1, Map::clientViewportH);

		//north
		msg.addByte(0x65);
		getMapDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, newPos.z, Map::clientViewportW, 1);
	}
}

void ProtocolGame::moveDownCreature(NetworkMessage &msg, const Creature* creature, const Position& newPos, const Position& oldPos, uint32_t oldStackPos)
{
	if(creature == player){
		//floor change down
		msg.addByte(0xBF);

		//going from surface to underground
		if(newPos.z == 8){
			int32_t skip = -1;
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, newPos.z, Map::clientViewportW, Map::clientViewportH, -1, skip);
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, newPos.z + 1, Map::clientViewportW, Map::clientViewportH, -2, skip);
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, newPos.z + 2, Map::clientViewportW, Map::clientViewportH, -3, skip);

			if(skip >= 0){
				msg.addByte(skip);
				msg.addByte(0xFF);
			}
		}
		//going further down
		else if(newPos.z > oldPos.z && newPos.z > 8 && newPos.z < 14){
			int32_t skip = -1;
			getFloorDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y - Map::clientViewportY, newPos.z + 2, Map::clientViewportW, Map::clientViewportH, -3, skip);

			if(skip >= 0){
				msg.addByte(skip);
				msg.addByte(0xFF);
			}
		}

		//moving down a floor makes us out of sync
		//east
		msg.addByte(0x66);
		getMapDescription(msg, oldPos.x + 1 + Map::clientViewportX, oldPos.y - 1 - Map::clientViewportY, newPos.z, 1, Map::clientViewportH);

		//south
		msg.addByte(0x67);
		getMapDescription(msg, oldPos.x - Map::clientViewportX, oldPos.y + 1 + Map::clientViewportY, newPos.z, Map::clientViewportW, 1);
	}
}

//vip
void ProtocolGame::addSendVIP(NetworkMessage &msg, uint32_t guid, const std::string &name, bool &isOnline)
{
	msg.addByte(0xD2);
	msg.addU32(guid);
	msg.addString(name);
	msg.addByte(isOnline ? 1 : 0);
}

//inventory
void ProtocolGame::addInventoryItem(NetworkMessage &msg, slots_t slot, const Item* item)
{
	if(item == NULL){
		msg.addByte(0x79);
		msg.addByte(slot);
	}
	else{
		msg.addByte(0x78);
		msg.addByte(slot);
		msg.addItem(item);
	}
}

void ProtocolGame::updateInventoryItem(NetworkMessage &msg, slots_t slot, const Item* item)
{
	if(item == NULL){
		msg.addByte(0x79);
		msg.addByte(slot);
	}
	else{
		msg.addByte(0x78);
		msg.addByte(slot);
		msg.addItem(item);
	}
}

void ProtocolGame::removeInventoryItem(NetworkMessage &msg, slots_t slot)
{
	msg.addByte(0x79);
	msg.addByte(slot);
}

//containers
void ProtocolGame::addContainerItem(NetworkMessage &msg, uint8_t cid, const Item* item)
{
	msg.addByte(0x70);
	msg.addByte(cid);
	msg.addItem(item);
}

void ProtocolGame::updateContainerItem(NetworkMessage &msg, uint8_t cid, uint8_t slot, const Item* item)
{
	msg.addByte(0x71);
	msg.addByte(cid);
	msg.addByte(slot);
	msg.addItem(item);
}

void ProtocolGame::removeContainerItem(NetworkMessage &msg, uint8_t cid, uint8_t slot)
{
	msg.addByte(0x72);
	msg.addByte(cid);
	msg.addByte(slot);
}
