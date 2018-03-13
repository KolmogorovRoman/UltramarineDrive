#include "GameServer.h"

GameServer* Server;

RecvUnit<BaseUnit>::RecvUnit()
{}
IDManager<RecvUnit<BaseUnit>, 65536, 65536> RecvUnit<BaseUnit>::Manager;
RecvUnit<BaseUnit>::~RecvUnit()
{
	Manager.Free(ID);
}
template<> void RegisterRecvTypes<char, char>(UINT Type)
{}


SendUnit<BaseUnit>::SendUnit()
{
	ID = NextID++;
	Manager.Place(ID, this);
}
IDManager<SendUnit<BaseUnit>, 65536, 65536> SendUnit<BaseUnit>::Manager;
UINT SendUnit<BaseUnit>::NextID = 0;
SendUnit<BaseUnit>::~SendUnit()
{
	Manager.Free(ID);
	Server->SendToAll(Nec, DeleteUnit, ID);
}

GameServer::GameServer(WORD Port):
	Server(Port)
{
	RegisterRecvProc(DeleteUnit, true, [](Server::MessageInfo* Message)
	{
		UINT ID;
		Message->Array->Deserialize(ID);
		RecvUnit<BaseUnit>* Unit = RecvUnit<BaseUnit>::Manager.Free(ID);
		if (Unit != NULL)
		{
			Unit->Delete();
		}
		return true;
	});
}
void GameServer::DeleteClient(Client* Client)
{
	cout << "Client [" << Client->ID << "] disconnected." << std::endl;
	::DeleteClient(Client);
	BYTE ID = Client->ID;
	delete Client;
	Clients[ID] = NULL;
}

template<> void RegisterSendTypes<char, char>(UINT Type)
{}