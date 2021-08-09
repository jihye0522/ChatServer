
#include "Header.h"
#include "TcpNetwork.h"
#include "PacketProcess.h"

int main()
{
	//√ ±‚»≠
	GTcpNetwork.InitServer();
	GPacketProcess.Init();

	while (true)
	{
		GTcpNetwork.Run();

		while (true)
		{
			auto packetInfo = GTcpNetwork.GetPacketInfo();

			if (packetInfo.PacketId == 0)
				break;
			else
				GPacketProcess.Process(packetInfo);
		}

		GPacketProcess.StateCheck();
	}

	return 1;
}