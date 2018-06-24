#pragma once
#include "NetProtocols.h"
#include "Management.h"
#include <map>
class Player;
template<class T> class RecvUnit;
template<class T> class SendUnit;

class Server
{
public:
	class Client
	{
	public:
		sockaddr_in Addr;
		BYTE ID;
		UINT TimeToLastRecv;
		Player* Player;
		Client(sockaddr_in Addr, BYTE ID);
		~Client();
	};
	struct StoredMessage
	{
		MessageHeader Header;
		BytesArray* FullMessage;
		WORD TimeToReSend;
		sockaddr_in ClientAddr;
		StoredMessage(MessageHeader Header, BytesArray* FullMessage, sockaddr_in ClientAddr, WORD TimeToResend);
		~StoredMessage();
	};
	struct MessageInfo
	{
		MessageHeader Header;
		BytesArray* Array;
		sockaddr_in FromAddr;
		Client* Client;
	};
	struct RecvProc
	{
		bool ForClientsOnly;
		function<bool(MessageInfo* Message)> Proc;
		RecvProc* Next;
	};
	static SOCKET Socket;
	static UINT NextNecNumber;
	static void MainRecvProc();
	static void ClientsConnectCheckProc();
	static std::map<UINT, RecvProc*> RecvProcs;
	static thread* RecvThread;
	static thread* ClientsConnectCheckThread;
	static bool StunResponseWaited;
	static mutex RecvMutex;
	static condition_variable StunResponseCond;
	static bool StunResponseRecved;
	static void SendTo(BYTE* Buff, int BuffLen, sockaddr_in* DestAddr);
	static void SendConfirm(UINT Number, sockaddr_in* DestAddr);
public:
	static void Init(WORD Port);
	static void Bind(char* IP, USHORT Port);
	static void StartRecv();
	static void Connect(char* IP, USHORT Port);
	template <typename... Types> static void SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals);
	static void RegisterRecvProc(WORD Type, bool ForClientsOnly, function<bool(MessageInfo* Message)>& Proc);
	static void RegisterRecvProc(WORD Type, bool ForClientsOnly, bool Proc(MessageInfo* Message));
	static void JoinRecv();
	static sockaddr_in GetSelfAddr(char* StunIP, USHORT StunPort);
	static std::map<UINT, StoredMessage*> Messages;
	static Client* Clients[256];
	static void SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr);
	static void ConfirmRecvProc(MessageInfo* Message);
	static void NecResendProc();
	static void DeleteClient(Client* Client);
	static CRITICAL_SECTION MessagesListCS;
	static Syncronizer NecResendSync;
public:
	Server(WORD Port);
	template <typename... Types> static void SendToClient(MessageNetType NetType, WORD Type, BYTE ClientID, Types&... Vals);
	template <typename... Types> static void SendToAll(MessageNetType NetType, WORD Type, Types&... Vals);
};
template <typename... Types> void Server::SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals)
{
	BytesArray* FullMessage = new BytesArray(VoidBytes(MessageHeaderSize), Vals...);
	MessageHeader Header;
	Header.NetType = NetType;
	Header.Type = Type;
	if (Header.NetType == Nec)
		Header.Number = NextNecNumber++;
	else Header.Number = 0;
	Header.Length = FullMessage->Size;
	FullMessage->Serialize(Header);
	SendTo(FullMessage->Array, FullMessage->Size, DestAddr);
	if (NetType == Nec)
		SendNecProc(Header, FullMessage, *DestAddr);
	else
		delete FullMessage;
}
template <typename... Types> void Server::SendToClient(MessageNetType NetType, WORD Type, BYTE ClientID, Types&... Vals)
{
	SendTo(NetType, Type, &Clients[ClientID]->Addr, Vals...);
}
template <typename... Types> void Server::SendToAll(MessageNetType NetType, WORD Type, Types&... Vals)
{
	for (int i = 0; i < 256; i++)
	{
		if (Clients[i] != NULL) SendToClient(NetType, Type, i, Vals...);
	}
}


template<> class RecvUnit<BaseUnit>:
	public ManagedUnit<RecvUnit<BaseUnit>>
{
public:
	RecvUnit();
	UINT ID;
	static std::map<UINT, RecvUnit<BaseUnit>*> Manager;
	virtual void Recved() {}
	~RecvUnit();
};
template<class T> class RecvUnit:
	public RecvUnit<BaseUnit>
{
public:
	static void Register(UINT& TypeID);
	static WORD MessageType;
	static bool RecvProc(Server::MessageInfo* Message);
};
template<class T> WORD RecvUnit<T>::MessageType;
template<class T> void RecvUnit<T>::Register(UINT& TypeID)
{
	MessageType = TypeID;
	Server->RegisterRecvProc(TypeID++, RecvProc);
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
template<class T, class... Types> void RegisterRecvTypes(UINT& TypeID)
{
	if (std::is_same<T, char>::value) return;
	RecvUnit<T>::Register(TypeID);
	RegisterRecvTypes<Types..., char>(TypeID);
}
template<> class RecvUnit<char>
{
public:
	static void Register(UINT& TypeID);
};
extern UINT NextRecvType;


template<> class SendUnit<BaseUnit>:
	public ManagedUnit<SendUnit<BaseUnit>>
{
public:
	SendUnit();
	bool Create;
	virtual void CreateSend() = 0;
	virtual void Send() = 0;
	UINT ID;
	static std::map<UINT, SendUnit<BaseUnit>*> Manager;
	static UINT NextID;
	~SendUnit<BaseUnit>();
};
template<class T> class SendUnit:
	public SendUnit<BaseUnit>
{
public:
	static void Register(UINT& TypeID);
	void CreateSend() override;
	void Send() override;
	static WORD MessageType;
};
template<class T> void SendUnit<T>::Register(UINT& TypeID)
{
	MessageType = TypeID++;
}
template<class T> void SendUnit<T>::CreateSend()
{
	Create = true;
	Server::SendToAll(Nec, MessageType, ID, *((T*)this));
}
template<class T> void SendUnit<T>::Send()
{
	Create = false;
	Server::SendToAll(NoNec, MessageType, ID, *((T*)this));
}
template<class T> WORD SendUnit<T>::MessageType;
template<class T, class... Types> void RegisterSendTypes(UINT& Type)
{
	if (std::is_same<T, char>::value) return;
	SendUnit<T>::Register(Type);
	RegisterSendTypes<Types..., char>(Type);
}
template<> class SendUnit<char>
{
public:
	static void Register(UINT& TypeID);
};
extern UINT NextSendType;