
#include "ClientSession.h"
#include "PacketInfo.h"
#include "Common/PacketID.h"
#include "TcpNetwork.h"

TcpNetwork::TcpNetwork()
{
}

TcpNetwork::~TcpNetwork()
{
}

bool TcpNetwork::InitServer()
{
	// 버전2.2
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	// 윈속 초기화 : 프로그램에서 요구하는 윈도우 소켓의 버전을 알리고, 해당 버전을 지원하는 라이브러리의 초기화 작업을 진행해야한다.
	WSAStartup(wVersionRequested, &wsaData);

	// 성공 시 소켓 핸들, 실패 시 INVAILD_SOCKET 반환
	mServerSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mServerSockfd < 0)
	{
		return false;
	}

	auto n = 1;
	if (setsockopt(mServerSockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof(n)) < 0)
	{
		return false;
	}

	short port = 11021;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	// bind
	if (bind(mServerSockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		return false;
	}

	auto netError = SetNonBlockSocket(mServerSockfd);
	if (false == netError)
		return false;

	// listen
	if (listen(mServerSockfd, 128) == SOCKET_ERROR)
	{
		return false;
	}

	m_MaxSockFD = mServerSockfd;

	std::cout << "listen" << std::endl;

	FD_ZERO(&m_Readfds);
	FD_SET(mServerSockfd, &m_Readfds);

	int maxClientCount = 1024;
	for (int i = 0; i < maxClientCount; ++i)
	{
		ClientSession session;
		session.Clear();
		session.Index = i;
		session.pRecvBuffer = new char[8192];
		session.pSendBuffer = new char[8192];

		m_ClientSessionPool.push_back(session);
		mClientSessionPoolIndex.push_back(session.Index);
	}

	std::cout << "create session pool" << std::endl;

	return true;
}

void TcpNetwork::Run()
{
	auto read_set = m_Readfds;
	auto write_set = m_Readfds;

	timeval timeout{ 0, 1000 }; //tv_sec, tv_usec

	auto selectResult = select(0, &read_set, &write_set, 0, &timeout);

	//오류 발생이면 - 1, 타임아웃이면 0, 변화가 발생하면 0 이상에 상태가 바뀐 파일 디스크립터 수.
	if (0 >= selectResult) 
		return;

	// Accept
	if (FD_ISSET(mServerSockfd, &read_set))
	{
		NewSession();
	}

	for (int i = 0; i < m_ClientSessionPool.size(); ++i)
	{
		auto& session = m_ClientSessionPool[i];

		if (session.IsConnected() == false) {
			continue;
		}

		SOCKET fd = session.SocketFD;
		auto sessionIndex = session.Index;

		// check read
		auto retReceive = RunProcessReceive(sessionIndex, fd, read_set);
		if (retReceive == false) {
			continue;
		}

		// check write
		RunProcessWrite(sessionIndex, fd, write_set);
	}
}

bool TcpNetwork::RunProcessReceive(const int sessionIndex, const SOCKET fd, fd_set& read_set)
{
	//FD_ISSET(s, *set)
	//:핸들 s가 set포인터가 가리키는 배열의 멤버라면 TRUE를 아니면 FALSE를 리턴한다.
	if (!FD_ISSET(fd, &read_set))
	{
		return true;
	}

	auto ret = RecvSocket(sessionIndex);
	if (false == ret)
	{
		CloseSession(SOCKET_CLOSE_CASE::SOCKET_RECV_ERROR, fd, sessionIndex);
		return false;
	}

	ret = RecvBufferProcess(sessionIndex);
	if (false == ret)
	{
		CloseSession(SOCKET_CLOSE_CASE::SOCKET_RECV_BUFFER_PROCESS_ERROR, fd, sessionIndex);
		return false;
	}

	return true;
}

void TcpNetwork::RunProcessWrite(const int sessionIndex, const SOCKET fd, fd_set& write_set)
{
	if (!FD_ISSET(fd, &write_set))
	{
		return;
	}

	auto retsend = FlushSendBuff(sessionIndex);
	if (false == retsend)
	{
		CloseSession(SOCKET_CLOSE_CASE::SOCKET_SEND_ERROR, fd, sessionIndex);
	}
}

bool TcpNetwork::FlushSendBuff(const int sessionIndex)
{
	auto& session = m_ClientSessionPool[sessionIndex];
	auto fd = static_cast<SOCKET>(session.SocketFD);

	if (session.IsConnected() == false)
		return false;

	auto rfds = m_Readfds;

	// 접속되어 있는지 또는 보낼 데이터가 있는지
	if (session.SendSize <= 0)
		return true;

	int ret = send(fd, session.pSendBuffer, session.SendSize, 0);

	if (ret <= 0)
		return false;

	auto sendSize = ret;
	if (sendSize < session.SendSize)
	{
		memmove(&session.pSendBuffer[0], &session.pSendBuffer[sendSize], session.SendSize - sendSize);
		session.SendSize -= sendSize;
	}
	else
	{
		session.SendSize = 0;
	}

	return true;
}

bool TcpNetwork::RecvBufferProcess(const int sessionIndex)
{
	auto& session = m_ClientSessionPool[sessionIndex];
	
	auto readPos = 0;
	const auto dataSize = session.RemainingDataSize;

	PacketHeader* pPktHeader = nullptr;

	while ((dataSize - readPos) >= PACKET_HEADER_SIZE)
	{
		pPktHeader = (PacketHeader*)&session.pRecvBuffer[readPos];
		readPos += PACKET_HEADER_SIZE;
		auto bodySize = (int16_t)(pPktHeader->TotalSize - PACKET_HEADER_SIZE);

		if (bodySize > 0)
		{
			if (bodySize > (dataSize - readPos))
			{
				readPos -= PACKET_HEADER_SIZE;
				break;
			}

			if (bodySize > MAX_PACKET_BODY_SIZE)
				return false;
		}

		//add packet
		RecvPacketInfo packetInfo;
		packetInfo.SessionIndex = sessionIndex;
		packetInfo.PacketId = pPktHeader->Id;
		packetInfo.PacketBodySize = bodySize;
		packetInfo.pRefData = &session.pRecvBuffer[readPos];

		mPacketQueue.push_back(packetInfo);

		readPos += bodySize;
	}

	session.RemainingDataSize -= readPos;
	session.PrevReadPosInRecvBuffer = readPos;

	return true;
}

bool TcpNetwork::RecvSocket(const int sessionIndex)
{
	auto& session = m_ClientSessionPool[sessionIndex];
	auto fd = static_cast<SOCKET>(session.SocketFD);
	if (false == session.IsConnected())
		return false;

	int recvPos = 0;

	//처리할 남은 데이터가 있다면
	if (session.RemainingDataSize > 0)
	{
		memcpy(session.pRecvBuffer, &session.pRecvBuffer[session.PrevReadPosInRecvBuffer], session.RemainingDataSize);
		recvPos += session.RemainingDataSize;
	}

	auto recvSize = recv(fd, &session.pRecvBuffer[recvPos], (MAX_PACKET_BODY_SIZE * 2), 0);

	if (0 == recvSize)
		return false;

	if (0 > recvSize)
	{
		auto netError = WSAGetLastError();
		if (WSAEWOULDBLOCK != netError)
			return false;
		else
			return true;
	}

	session.RemainingDataSize += recvSize;

	return true;
}

void TcpNetwork::ForcingClose(const int sessionIndex)
{
	if (false == m_ClientSessionPool[sessionIndex].IsConnected()) 
		return;

	CloseSession(SOCKET_CLOSE_CASE::FORCING_CLOSE, m_ClientSessionPool[sessionIndex].SocketFD, sessionIndex);
}

bool TcpNetwork::SendData(const int sessionIndex, const short packetId, const short bodySize, const char* pMsg)
{
	auto& session = m_ClientSessionPool[sessionIndex];

	auto pos = session.SendSize;
	auto totalSize = (int16_t)(bodySize + PACKET_HEADER_SIZE);

	if ((pos + totalSize) > 8192)
		return false; //buffer full
	
	PacketHeader pktHeader{ totalSize, packetId, (uint8_t)0 };
	memcpy(&session.pSendBuffer[pos], (char*)&pktHeader, PACKET_HEADER_SIZE);

	if (bodySize > 0)
	{
		memcpy(&session.pSendBuffer[pos + PACKET_HEADER_SIZE], pMsg, bodySize);
	}

	session.SendSize += totalSize;

	return true;
}

void TcpNetwork::Release()
{
	WSACleanup();
}

RecvPacketInfo TcpNetwork::GetPacketInfo()
{
	RecvPacketInfo packetInfo;

	if (mPacketQueue.empty() == false)
	{
		packetInfo = mPacketQueue.front();
		mPacketQueue.pop_front();
	}

	return packetInfo;
}

void TcpNetwork::NewSession()
{
	auto tryCount = 0;

	do
	{
		++tryCount;

		struct sockaddr_in client_adr;

		auto client_len = static_cast<int>(sizeof(client_adr));

		//accept
		auto client_sockfd = accept(mServerSockfd, (struct sockaddr*)&client_adr, &client_len);

		if (client_sockfd == INVALID_SOCKET)
		{
			if (WSAEWOULDBLOCK == WSAGetLastError())
				return;

			return;
		}

		if (true == mClientSessionPoolIndex.empty())
		{
			return;
		}

		int index = mClientSessionPoolIndex.front();
		mClientSessionPoolIndex.pop_front();

		if(index < 0)
		{
			CloseSession(SOCKET_CLOSE_CASE::SESSION_POOL_EMPTY, client_sockfd, -1);
			return; //더 이상 수용 불가
		}

		char clientIP[MAX_IP_LEN] = { 0, };
		inet_ntop(AF_INET, &(client_adr.sin_addr), clientIP, MAX_IP_LEN - 1);

		//SockOption
		linger ling;
		ling.l_onoff = 0;
		ling.l_linger = 0;
		setsockopt(client_sockfd, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));

		int size1 = 10240;// m_Config.MaxClientSockOptRecvBufferSize;
		int size2 = 10240;// m_Config.MaxClientSockOptSendBufferSize;
		setsockopt(client_sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&size1, sizeof(size1));
		setsockopt(client_sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&size2, sizeof(size2));

		SetNonBlockSocket(client_sockfd);

		FD_SET(client_sockfd, &m_Readfds);

		ConnectedSession(index, client_sockfd, clientIP);

	} while (tryCount < FD_SETSIZE);

	return;
}

void TcpNetwork::ConnectedSession(const int sessionIndex, const SOCKET fd, const char* pIP)
{
	if (m_MaxSockFD < fd)
	{
		m_MaxSockFD = fd;
	}

	++m_ConnectSeq;

	auto& session = m_ClientSessionPool[sessionIndex];
	session.Seq = m_ConnectSeq;
	session.SocketFD = fd;
	memcpy(session.IP, pIP, MAX_IP_LEN - 1);

	++mConnectedSessionCount;

	RecvPacketInfo packetInfo;
	packetInfo.SessionIndex = sessionIndex;
	packetInfo.PacketId = (short)NCommon::PACKET_ID::CONNECT_SESSION;
	packetInfo.PacketBodySize = 0;
	packetInfo.pRefData = nullptr;

	mPacketQueue.push_back(packetInfo);
}

bool TcpNetwork::SetNonBlockSocket(const SOCKET sock)
{
	unsigned long mode = 1;

	if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

void TcpNetwork::CloseSession(const SOCKET_CLOSE_CASE closeCase, const SOCKET sockFD, const int sessionIndex)
{
	if (closeCase == SOCKET_CLOSE_CASE::SESSION_POOL_EMPTY)
	{
		closesocket(sockFD);
		FD_CLR(sockFD, &m_Readfds);
		return;
	}

	if (m_ClientSessionPool[sessionIndex].IsConnected() == false) {
		return;
	}

	closesocket(sockFD);

	FD_CLR(sockFD, &m_Readfds);

	m_ClientSessionPool[sessionIndex].Clear();
	--mConnectedSessionCount;
	mClientSessionPoolIndex.push_back(sessionIndex);
	m_ClientSessionPool[sessionIndex].Clear();

	RecvPacketInfo packetInfo;
	packetInfo.SessionIndex = sessionIndex;
	//packetInfo.PacketId = (short)NCommon::PACKET_ID::NTF_SYS_CLOSE_SESSION;
	packetInfo.PacketBodySize = 0;
	packetInfo.pRefData = nullptr;

	mPacketQueue.push_back(packetInfo);
}