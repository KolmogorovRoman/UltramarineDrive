#pragma once
#include "Client.h"

class GameClient;
template<class T> class RecvUnit;
template<class T> class SendUnit;
extern GameClient* Client;

class GameClient:
	public Client
{
public:
	GameClient(WORD Port);
};

template<> class RecvUnit<BaseUnit>:
	public ManagedUnit<RecvUnit<BaseUnit>>
{
public:
	RecvUnit();
	UINT ID;
	static IDManager<RecvUnit<BaseUnit>, 65536, 65536> Manager;
	virtual void Recved() {}
	~RecvUnit();
};

template<class T> class RecvUnit:
	public RecvUnit<BaseUnit>
{
public:
	static void Register(UINT TypeID);
	static WORD MessageType;
	static bool RecvProc(Client::MessageInfo* Message);
};
template<class T> WORD RecvUnit<T>::MessageType;
template<class T> void RecvUnit<T>::Register(UINT TypeID)
{
	MessageType = TypeID;
	Client->RegisterRecvProc(TypeID, RecvProc);
}
template<class T> bool RecvUnit<T>::RecvProc(Client::MessageInfo* Message)
{
	UINT ID;
	T* RecvedObject;
	Message->Array->Deserialize(ID);
	T* ExistedObject=(T*)Manager.Get(ID);
	if (ExistedObject==NULL)
	{
		RecvedObject = new T;
		Manager.Place(ID, (RecvUnit<BaseUnit>*) RecvedObject);
		((RecvUnit<BaseUnit>*) RecvedObject)->ID = ID;
		//cout<<Create<<endl;
	}
	else
		RecvedObject = ExistedObject;
	Message->Array->Deserialize(*RecvedObject);
	RecvedObject->Recved();
	return true;
}

template<class T, class... Types> void RegisterRecvTypes(UINT TypeID)
{
	RecvUnit<T>::Register(TypeID);
	RegisterRecvTypes<Types..., char>(TypeID + 1);
}
template<> void RegisterRecvTypes<char, char>(UINT TypeID);


template<> class SendUnit<BaseUnit>:
	public ManagedUnit<SendUnit<BaseUnit>>
{
public:
	SendUnit();
	virtual void Send() = 0;
	UINT ID;
	static IDManager<SendUnit<BaseUnit>, 65536, 65536> Manager;
	static UINT NextID;
	~SendUnit<BaseUnit>();
};

template<class T> class SendUnit:
	public SendUnit<BaseUnit>
{
public:
	static void Register(UINT TypeID);
	void Send() override;
	static WORD MessageType;
};
template<class T> void SendUnit<T>::Register(UINT TypeID)
{
	MessageType = TypeID;
}
template<class T> void SendUnit<T>::Send()
{
	Client->SendToServer(NoNec, MessageType, ID, *((T*)this));
}
template<class T> WORD SendUnit<T>::MessageType;

template<class T, class... Types> void RegisterSendTypes(UINT Type)
{
	SendUnit<T>::Register(Type);
	RegisterSendTypes<Types..., char>(Type + 1);
}
template<> void RegisterSendTypes<char, char>(UINT Type);