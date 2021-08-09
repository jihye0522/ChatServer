
#include "Lobby.h"
#include "LobbyManager.h"


LobbyManager::LobbyManager()
{
	Init();
}

void LobbyManager::Init()
{
	for (int idx = 0; idx < 3; ++idx)
	{
		mLobbyList.emplace_back(idx, 10);
	}
}