
#include "ConnectedUserManager.h"
#include "Common/Packet.h"
#include "UserManager.h"
#include "Lobby.h"
#include "LobbyManager.h"
#include "PacketProcess.h"
#include "User.h"

void PacketProcess::Init()
{
	GConnectedUserManager.Init(1024);

	for (int idx = 0; idx < (int)NCommon::PACKET_ID::MAX; ++idx)
	{
		PacketFuncArray[idx] = nullptr;
	}

	PacketFuncArray[(short)NCommon::PACKET_ID::CONNECT_SESSION] = &PacketProcess::ConnectSession;
	PacketFuncArray[(short)NCommon::PACKET_ID::CLOSE_SESSION] = &PacketProcess::CloseSession;
	PacketFuncArray[(short)NCommon::PACKET_ID::LOGIN_IN_REQ] = &PacketProcess::Login;
	PacketFuncArray[(short)NCommon::PACKET_ID::LOBBY_LIST_REQ] = &PacketProcess::LobbyList;
	PacketFuncArray[(short)NCommon::PACKET_ID::LOBBY_ENTER_REQ] = &PacketProcess::LobbyEnter;
	PacketFuncArray[(short)NCommon::PACKET_ID::LOBBY_LEAVE_REQ] = &PacketProcess::LobbyLeave;
	PacketFuncArray[(short)NCommon::PACKET_ID::LOBBY_CHAT_REQ] = &PacketProcess::LobbyChat;
}

void PacketProcess::Process(PacketInfo packetInfo)
{
	auto packetId = packetInfo.PacketId;

	if (PacketFuncArray[packetId] == nullptr)
		return;

	(this->*PacketFuncArray[packetId])(packetInfo);
}

void PacketProcess::StateCheck()
{
	GConnectedUserManager.LoginCheck();
}

void PacketProcess::ConnectSession(PacketInfo packetInfo)
{
	GConnectedUserManager.SetConnectSession(packetInfo.SessionIndex);
}

void PacketProcess::CloseSession(PacketInfo packetInfo)
{
}

void PacketProcess::Login(PacketInfo packetInfo)
{
	auto reqPkt = (NCommon::PktLogInReq*)packetInfo.pRefData;

	GUserManager.AddUser(packetInfo.SessionIndex, reqPkt->szID);

	GConnectedUserManager.SetLogin(packetInfo.SessionIndex);

	NCommon::PktLogInRes resPkt;
	GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOGIN_IN_RES, sizeof(NCommon::PktLogInRes), (char*)&resPkt);
}

void PacketProcess::LobbyList(PacketInfo packetInfo)
{
	User* user = GUserManager.FindUser(packetInfo.SessionIndex);
	if (nullptr == user)
		return;

	//로그인 상태에서만 로비요청을 할 수 있다.
	if (User::STATE::LOGIN != user->GetState())
		return;

	std::vector<Lobby>& lobbyList = GLobbyManager.GetLobbyList();

	NCommon::PktLobbyListRes resPkt;

	resPkt.LobbyCount = lobbyList.size();

	int index = 0;
	for (auto& lobby : lobbyList)
	{
		resPkt.LobbyList[index].LobbyId = lobby.GetLobbyIndex();
		resPkt.LobbyList[index].LobbyId = lobby.GetUserCount();
		resPkt.LobbyList[index].LobbyMaxUserCount = lobby.GetLobbyMaxUserCount();
		++index;
	}

	GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOBBY_LIST_RES, sizeof(NCommon::PktLobbyListRes), (char*)&resPkt);
}

void PacketProcess::LobbyEnter(PacketInfo packetInfo)
{
	auto reqPkt = (NCommon::PktLobbyEnterReq*)packetInfo.pRefData;
	NCommon::PktLobbyEnterRes resPkt;
	User* user = GUserManager.FindUser(packetInfo.SessionIndex);
	if (nullptr == user)
	{
		resPkt.SetError(NCommon::ERROR_CODE::USER_MGR_INVALID_SESSION_INDEX);
		GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
		return;
	}

	Lobby& lobby = GLobbyManager.GetLobby(reqPkt->LobbyId);

	lobby.EnterUser(user);
	resPkt.MaxUserCount = lobby.GetLobbyMaxUserCount();
	GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
}

void PacketProcess::LobbyLeave(PacketInfo packetInfo)
{
	auto reqPkt = (NCommon::PktLobbyLeaveReq*)packetInfo.pRefData;
	NCommon::PktLobbyLeaveRes resPkt;

	User* user = GUserManager.FindUser(packetInfo.SessionIndex);
	if (nullptr == user)
	{
		resPkt.SetError(NCommon::ERROR_CODE::USER_MGR_INVALID_SESSION_INDEX);
		GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
		return;
	}

	Lobby& lobby = GLobbyManager.GetLobby(user->GetLobbyIndex());

	lobby.LeaveUser(user);
	GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
}

void PacketProcess::LobbyChat(PacketInfo packetInfo)
{
	auto reqPkt = (NCommon::PktLobbyChatReq*)packetInfo.pRefData;
	NCommon::PktLobbyChatRes resPkt;

	User* user = GUserManager.FindUser(packetInfo.SessionIndex);
	if (nullptr == user)
		return;

	short lobbyIndex = user->GetLobbyIndex();
	Lobby& lobby = GLobbyManager.GetLobby(lobbyIndex);

	lobby.NotifyChat(user->GetSessionIndex(), user->GetID().c_str(), reqPkt->Msg);
	
	GTcpNetwork.SendData(packetInfo.SessionIndex, (short)NCommon::PACKET_ID::LOBBY_CHAT_RES, sizeof(NCommon::PktLobbyChatRes), (char*)&resPkt);
}