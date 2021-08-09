
#include "TcpNetwork.h"
#include "ConnectedUserManager.h"

void ConnectedUserManager::Init(const int maxSessionCount)
{
	for (int idx = 0; idx < maxSessionCount; ++idx)
	{
		mConnectedUserList.emplace_back(ConnectedUser());
	}
	// 연결 후 특정 시간 이내에 로그인 완료 여부 조사
	mIsLoginCheck = true;
}

void ConnectedUserManager::SetConnectSession(const int sessionIndex)
{
	time(&mConnectedUserList[sessionIndex].mConnectedTime);
}

void ConnectedUserManager::SetLogin(const int sessionIndex)
{
	mConnectedUserList[sessionIndex].mIsLoginSuccess = true;
}

void ConnectedUserManager::SetDisConnectSession(const int sessionIndex)
{
	mConnectedUserList[sessionIndex].Clear();
}

void ConnectedUserManager::LoginCheck()
{
	auto curTime = std::chrono::system_clock::now();
	auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - mLatestLoginCheckTime);

	//60밀리초마다 검사
	if (diffTime.count() < 60)
	{
		return;
	}
	else
	{
		mLatestLoginCheckTime = curTime;
	}

	auto curSecTime = std::chrono::system_clock::to_time_t(curTime);

	const auto maxSessionCount = static_cast<int>(mConnectedUserList.size());

	if (mLatestLoginCheckIndex >= maxSessionCount)
	{
		mLatestLoginCheckIndex = -1;
	}

	++mLatestLoginCheckIndex;

	auto lastCheckIndex = mLatestLoginCheckIndex + 100;
	lastCheckIndex = (lastCheckIndex > maxSessionCount) ? maxSessionCount : lastCheckIndex;

	for (; mLatestLoginCheckIndex < lastCheckIndex; ++mLatestLoginCheckIndex)
	{
		auto idx = mLatestLoginCheckIndex;

		if ((0 >= mConnectedUserList[idx].mConnectedTime) ||
			(false == mConnectedUserList[idx].mIsLoginSuccess))
		{
			continue;
		}

		auto diff = curSecTime - mConnectedUserList[idx].mConnectedTime;
		if (diff >= 180)
		{
			GTcpNetwork.ForcingClose(idx);
		}
	}
}
