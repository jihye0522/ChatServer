#pragma once

#include "Header.h"

struct ConnectedUser
{
	void Clear()
	{
		mIsLoginSuccess = false;
		mConnectedTime = 0;
	}

	bool mIsLoginSuccess = false;
	time_t mConnectedTime = 0;
};

class ConnectedUserManager
{
public:
	ConnectedUserManager() = default;
	virtual ~ConnectedUserManager() = default;

	void Init(const int maxSessionCount);
	void SetConnectSession(const int sessionIndex);
	void SetLogin(const int sessionIndex);
	void SetDisConnectSession(const int sessionIndex);
	void LoginCheck();

private:
	std::vector<ConnectedUser> mConnectedUserList;
	bool mIsLoginCheck = false;

	std::chrono::system_clock::time_point mLatestLoginCheckTime = std::chrono::system_clock::now();
	int mLatestLoginCheckIndex = -1;
};

inline ConnectedUserManager GConnectedUserManager;