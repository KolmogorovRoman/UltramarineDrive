#pragma once
#include "Server.h"

class GameServer;
template<class T> class RecvUnit;
template<class T> class SendUnit;
extern GameServer* Server;
extern void DeleteClient(Server::Client* Client);

class GameServer:
	public Server
{
public:
	GameServer(WORD Port);
	void DeleteClient(Client* Client) override;
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
	static bool RecvProc(Server::MessageInfo* Message);
};
template<class T> WORD RecvUnit<T>::MessageType;
template<class T> void RecvUnit<T>::Register(UINT TypeID)
{
	MessageType = TypeID;
	Server->RegisterRecvProc(TypeID, RecvProc);
}
template<class T> bool RecvUnit<T>::RecvProc(Server::MessageInfo* Message)
{
	UINT ID;
	T* RecvedObject;
	Message->Array->Deserialize(ID);
	T* ExistedObject = (T*) Manager.Get(ID);
	if (ExistedObject == NULL)
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
	bool Create;
	virtual void CreateSend() = 0;
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
	void CreateSend() override;
	void Send() override;
	static WORD MessageType;
};
template<class T> void SendUnit<T>::Register(UINT TypeID)
{
	MessageType = TypeID;
}
template<class T> void SendUnit<T>::CreateSend()
{
	Create = true;
	Server->SendToAll(Nec, MessageType, ID, *((T*)this));
}
template<class T> void SendUnit<T>::Send()
{
	Create = false;
	Server->SendToAll(NoNec, MessageType, ID, *((T*)this));
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