#pragma once

#include "Header.h"

struct ClientSession
{
	bool IsConnected() { return SocketFD != 0 ? true : false; }

	void Clear()
	{
		Seq = 0;
		SocketFD = 0;
		IP[0] = '\0';
		RemainingDataSize = 0;
		PrevReadPosInRecvBuffer = 0;
		SendSize = 0;
	}

	int Index = 0;
	long long Seq = 0;
	unsigned long long	SocketFD = 0;
	char    IP[MAX_IP_LEN] = { 0, };

	char*   pRecvBuffer = nullptr;
	int     RemainingDataSize = 0;
	int     PrevReadPosInRecvBuffer = 0;

	char*   pSendBuffer = nullptr;
	int     SendSize = 0;
};

