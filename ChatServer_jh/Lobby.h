#pragma once

#include "Header.h"

class User;
struct LobbyUser
{
	short Index = 0;
	User* pUser = nullptr;
};

class Lobby
{
public:
	Lobby(int lobbyIndex, const short maxLobbyUserCount);
	~Lobby() = default;


public:
	int GetLobbyIndex() { return mLobbyIndex;  }
	short GetUserCount() { return static_cast<short>(mUserIndexMap.size()); }
	short GetLobbyMaxUserCount() { return mMaxLobbyUserCount; }
	void EnterUser(User* pUser);
	void LeaveUser(User* pUser);
	void NotifyChat(const int sessionIndex, const char* userID, const wchar_t* msg);
	void SendToAllUser(const short packetId, const short dataSize, char* pData, const int sessionIndex);


private:
	short mLobbyIndex = 0;
	short mMaxLobbyUserCount = 10;
	std::unordered_map<int, User*> mUserIndexMap;
	std::unordered_map<const char*, User*> mUserIDMap;
};