#pragma once

#include "Common/PacketID.h"
#include "PacketInfo.h"
#include "TcpNetwork.h"

class PacketProcess
{
public:
	using PacketInfo = RecvPacketInfo;
	//using PacketFunc = void(*)(PacketInfo);
	typedef void(PacketProcess::* PacketFunc)(PacketInfo);
	PacketFunc PacketFuncArray[(short)NCommon::PACKET_ID::MAX];


public:
	PacketProcess() = default;
	~PacketProcess() = default;


public:
	void Init();
	void Process(PacketInfo packetInfo);
	void StateCheck();
	void ConnectSession(PacketInfo packetInfo);
	void CloseSession(PacketInfo packetInfo);
	void Login(PacketInfo packetInfo);
	void LobbyList(PacketInfo packetInfo);
	void LobbyEnter(PacketInfo packetInfo);
	void LobbyLeave(PacketInfo packetInfo);
	void LobbyChat(PacketInfo packetInfo);

};

inline PacketProcess GPacketProcess;