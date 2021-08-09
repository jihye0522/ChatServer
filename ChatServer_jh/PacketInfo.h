
#pragma once


struct RecvPacketInfo
{
	int SessionIndex = 0;
	short PacketId = 0;
	short PacketBodySize = 0;
	char* pRefData = 0;
};


#pragma pack(push, 1)
struct PacketHeader
{
	short TotalSize;
	short Id;
	unsigned char Reserve;
};

const int PACKET_HEADER_SIZE = sizeof(PacketHeader);


struct PktNtfSysCloseSession : PacketHeader
{
	int SockFD;
};
#pragma pack(pop)

enum class SOCKET_CLOSE_CASE : short
{
	SESSION_POOL_EMPTY = 1,
	SELECT_ERROR = 2,
	SOCKET_RECV_ERROR = 3,
	SOCKET_RECV_BUFFER_PROCESS_ERROR = 4,
	SOCKET_SEND_ERROR = 5,
	FORCING_CLOSE = 6,
};
