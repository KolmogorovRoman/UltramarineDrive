#include "GameClient.h"

GameClient* MainClient;

RecvUnit<BaseUnit>::RecvUnit()
{}
IDManager<RecvUnit<BaseUnit>, 65536, 65536> RecvUnit<BaseUnit>::Manager;
RecvUnit<BaseUnit>::~RecvUnit()
{
	Manager.Free(ID);
}
void RecvUnit<char>::Register(UINT TypeID)
{}

GameClient::GameClient(WORD Port):
	Client(Port)
{
	RegisterRecvProc(DeleteUnit, [](Client::MessageInfo* Message)
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

SendUnit<BaseUnit>::SendUnit()
{
	ID = Manager.Alloc(this);
}
IDManager<SendUnit<BaseUnit>, 65536, 65536> SendUnit<BaseUnit>::Manager;
SendUnit<BaseUnit>::~SendUnit()
{
	Manager.Free(ID);
	MainClient->SendToServer(Nec, DeleteUnit, ID);
}
void SendUnit<char>::Register(UINT TypeID)
{}