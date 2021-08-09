#pragma once

#include "Header.h"

class LobbyManager
{
public:
	explicit LobbyManager();
	~LobbyManager() = default;


public:
	void Init();
	Lobby& GetLobby(int index) { return mLobbyList[index]; }
	std::vector<Lobby>& GetLobbyList() { return mLobbyList; }

private:
	std::vector<Lobby> mLobbyList;
};

inline LobbyManager GLobbyManager;