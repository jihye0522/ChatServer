#pragma once

#define FD_SETSIZE 5096

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Header.h"
#include "PacketInfo.h"
#include "ClientSession.h"

class TcpNetwork
{
public:
	TcpNetwork();
	~TcpNetwork();

public:
	bool InitServer();
	void Run();
	bool FlushSendBuff(const int sessionIndex);
	bool SendData(const int sessionIndex, const short packetId, const short bodySize, const char* pMsg);
	void Release();
	RecvPacketInfo GetPacketInfo();
	void ForcingClose(const int sessionIndex);
	void NewSession();
	bool RecvSocket(const int sessionIndex);
	bool RecvBufferProcess(const int sessionIndex);
	bool SetNonBlockSocket(const SOCKET sock);
	void ConnectedSession(const int sessionIndex, const SOCKET fd, const char* pIP);
	void CloseSession(const SOCKET_CLOSE_CASE closeCase, const SOCKET sockFD, const int sessionIndex);

	bool RunProcessReceive(const int sessionIndex, const SOCKET fd, fd_set& read_set);
	void RunProcessWrite(const int sessionIndex, const SOCKET fd, fd_set& write_set);

private:
	fd_set m_Readfds;
	SOCKET mServerSockfd;
	size_t mConnectedSessionCount = 0;
	SOCKET m_MaxSockFD;
	std::vector<ClientSession> m_ClientSessionPool;
	std::deque<int> mClientSessionPoolIndex;
	std::deque<RecvPacketInfo> mPacketQueue;
	int64_t m_ConnectSeq = 0;
};

inline TcpNetwork GTcpNetwork;
