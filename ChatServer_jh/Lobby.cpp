

#include "TcpNetwork.h"
#include "Common/Packet.h"
#include "User.h"
#include "Lobby.h"

Lobby::Lobby(int lobbyIndex, const short maxLobbyUserCount)
	:mLobbyIndex(lobbyIndex), mMaxLobbyUserCount(maxLobbyUserCount)
{
}

void Lobby::EnterUser(User* user)
{
	user->EnterLobby(mLobbyIndex);
	mUserIndexMap.insert({ user->GetSessionIndex(), user });
	mUserIDMap.insert({ user->GetID().c_str(), user });
}

void Lobby::LeaveUser(User* user)
{
	user->LeaveLobby();
	mUserIndexMap.erase(user->GetSessionIndex());
	mUserIDMap.erase(user->GetID().c_str());
}

void Lobby::NotifyChat(const int sessionIndex, const char* userID, const wchar_t* msg)
{
	NCommon::PktLobbyChatNtf pkt;
	strncpy_s(pkt.UserID, (NCommon::MAX_USER_ID_SIZE + 1), userID, NCommon::MAX_USER_ID_SIZE);
	wcsncpy_s(pkt.Msg, NCommon::MAX_LOBBY_CHAT_MSG_SIZE + 1, msg, NCommon::MAX_LOBBY_CHAT_MSG_SIZE);
	SendToAllUser((short)NCommon::PACKET_ID::LOBBY_CHAT_NTF, sizeof(pkt), (char*)&pkt, sessionIndex);
}

void Lobby::SendToAllUser(const short packetId, const short dataSize, char* pData, const int sessionIndex)
{
	for (auto& user : mUserIndexMap)
	{
		GTcpNetwork.SendData(user.second->GetSessionIndex(), packetId, dataSize, pData);
		printf("%s : %s", user.second->GetID().c_str(), pData);
	}
}