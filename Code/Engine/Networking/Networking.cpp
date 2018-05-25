#include "Networking.h"
#include "../../External/RakNet/Source/WindowsIncludes.h"
#include "../../External/RakNet/Source/RakPeerInterface.h"
#include "../../External/RakNet/Source/MessageIdentifiers.h"
#include "../../External/RakNet/Source/BitStream.h"
#include "../../External/RakNet/Source/RakNetTypes.h"
#include "../../External/Lua/Includes.h"
#include <vector>

namespace {
	
	enum GameMessages
	{
		ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1
	};
	
	RakNet::RakPeerInterface * s_peer;
	RakNet::SocketDescriptor sd;
	RakNet::RakNetGUID s_remote_guid;
	const unsigned short SERVER_PORT = 60000;
	const uint8_t MAX_CLIENTS = 10;
	const char* const SERVER_IP = "127.0.0.1";

	bool s_isServer = false;
	std::function<void(eae6320::Networking::eSession, bool)> create_player;

	eae6320::Networking::sPlayerData* s_main_player;
	std::function<void(eae6320::Networking::sPlayerData*)> s_remote_player_callback;

	bool LoadMeshScript(const char* i_path, eae6320::Networking::eSession* o_session);
	bool LoadTableValues(lua_State& io_luaState, eae6320::Networking::eSession* o_session);

}
bool eae6320::Networking::Initialize()
{
	return true;
}

void eae6320::Networking::Load(const char* const i_path, std::function<void(eae6320::Networking::eSession, bool)> i_callback) 
{

	s_peer = RakNet::RakPeerInterface::GetInstance();
	eae6320::Networking::eSession session;
	if (!LoadMeshScript(i_path, &session))
	{
		return;
	}

	if (session == eae6320::Networking::eSession::CLIENT) 
	{
		s_peer->Startup(1, &sd, 1);
		s_isServer = false;
		s_peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);
	}
	else 
	{
		sd = RakNet::SocketDescriptor(SERVER_PORT, 0);
		s_peer->Startup(MAX_CLIENTS, &sd, 1);
		s_peer->SetMaximumIncomingConnections(MAX_CLIENTS);
		s_isServer = true;
	}
	create_player = i_callback;
	i_callback(session, true);
}

void eae6320::Networking::Update()
{
	RakNet::Packet* packet;
	if (s_main_player)
	{
		RakNet::BitStream bsOut;
		bsOut.Write(static_cast<RakNet::MessageID>(ID_GAME_MESSAGE_1));
		{
			const char* data = reinterpret_cast<char*>(s_main_player);
			const unsigned int size = sizeof(eae6320::Networking::sPlayerData);
			bsOut.Write(data, size);
		}
		s_peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, s_peer->GetMyGUID(), true);
	}

	for (packet = s_peer->Receive(); packet; s_peer->DeallocatePacket(packet), packet = s_peer->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			break;
		case ID_REMOTE_CONNECTION_LOST:
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			create_player(eSession::SERVER, false);
			break;
		case ID_NEW_INCOMING_CONNECTION:
			create_player(eSession::CLIENT, false);
			return;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			break;
		case ID_CONNECTION_LOST:
			break;
		case ID_GAME_MESSAGE_1:
		{
			sPlayerData playerData;
			unsigned int noOfBytes = sizeof(sPlayerData);
			char* output = reinterpret_cast<char*>(&playerData);
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(output, noOfBytes);
			s_remote_player_callback(&playerData);
		}
		break;
		default:
			break;
		}
	}
}

bool eae6320::Networking::CleanUp() 
{
	RakNet::RakPeerInterface::DestroyInstance(s_peer);
	return true;
}

void eae6320::Networking::SubmitMainPlayerData(sPlayerData * i_playerData)
{
	s_main_player = i_playerData;
}

void eae6320::Networking::UpdateRemotePlayerData(std::function<void(eae6320::Networking::sPlayerData*)> i_update_callback)
{
	s_remote_player_callback = i_update_callback;
}


namespace {
	bool LoadMeshScript(const char * i_path, eae6320::Networking::eSession * o_session)
	{
		bool wereThereErrors = false;
		lua_State* luaState = NULL;
		{
			luaState = luaL_newstate();
			if (!luaState)
			{
				wereThereErrors = true;
				goto OnExit;
			}
		}
		const int stackTopBeforeLoad = lua_gettop(luaState);
		{
			const int luaResult = luaL_loadfile(luaState, i_path);
			if (luaResult != LUA_OK)
			{
				wereThereErrors = true;
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}
		{
			const int argumentCount = 0;
			const int returnValueCount = LUA_MULTRET;
			const int noMessageHandler = 0;
			const int luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
			if (luaResult == LUA_OK)
			{
				const int returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
				if (returnedValueCount == 1)
				{
					if (!lua_istable(luaState, -1))
					{
						wereThereErrors = true;
						lua_pop(luaState, 1);
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					lua_pop(luaState, returnedValueCount);
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}
		if (!LoadTableValues(*luaState, o_session))
		{
			wereThereErrors = true;
		}
		lua_pop(luaState, 1);
	OnExit:
		if (luaState)
		{
			lua_close(luaState);
			luaState = NULL;
		}
		return !wereThereErrors;
	}

	bool LoadTableValues(lua_State& io_luaState, eae6320::Networking::eSession * o_session)
	{
		bool wereThereErrors = false;
		{

			const char* const key = "session";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);
			{
				*o_session = static_cast<eae6320::Networking::eSession>(lua_tointeger(&io_luaState, -1));
				lua_pop(&io_luaState, 1);
			}
			lua_pop(&io_luaState, 1);
		}
		return !wereThereErrors;
	}
}