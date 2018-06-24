#pragma once
#include <type_traits>
#include "Client.h"

class GameClient;
template<class T> class RecvUnit;
template<class T> class SendUnit;
extern GameClient* MainClient;

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
	MainClient->RegisterRecvProc(TypeID, RecvProc);
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
		RecvedObject->RecvUnit<T>::ID = ID;
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
	if (std::is_same<T, char>::value) return;
	RecvUnit<T>::Register(TypeID);
	RegisterRecvTypes<Types..., char>(TypeID + 1);
}
template<> class RecvUnit<char>
{
public:
	static void Register(UINT TypeID);
};


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
	MainClient->SendToServer(NoNec, MessageType, ID, *((T*)this));
}
template<class T> WORD SendUnit<T>::MessageType;

template<class T, class... Types> void RegisterSendTypes(UINT Type)
{
	if (std::is_same<T, char>::value) return;
	SendUnit<T>::Register(Type);
	RegisterSendTypes<Types..., char>(Type + 1);
}
template<> class SendUnit<char>
{
public:
	static void Register(UINT TypeID);
};