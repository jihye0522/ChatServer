#pragma once
#include "Header.h"

class User
{
public:
	enum class STATE
	{
		NONE,
		LOGIN,
		LOBBY,
		ROOM,
	};

public:
	explicit User(const int sessionIndex, const char* id)
	{
		mSessionIndex = sessionIndex;
		mID = id;
	}

	~User() = default;

	void SetState(STATE state) { mState = state; }
	STATE GetState() { return mState; }
	short GetLobbyIndex() { return mLobbyIndex; }
	int GetSessionIndex() { return mSessionIndex; }
	std::string& GetID() { return mID; }

	void EnterLobby(const short lobbyIndex)
	{
		mLobbyIndex = lobbyIndex;
		mState = STATE::LOBBY;
	}

	void LeaveLobby()
	{
		mLobbyIndex = -1;
		mState = STATE::LOGIN;
	}

private:
	int mSessionIndex = 0;
	std::string mID;
	STATE mState = STATE::NONE;
	short mLobbyIndex = -1;

};