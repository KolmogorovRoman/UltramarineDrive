#pragma once
#include "NetProtocols.h"
#include "Management.h"
#include <map>

class Client
{
public:
	static sockaddr_in ServerAddr;
	static BYTE ID;
	struct StoredMessage
	{
		MessageHeader Header;
		BytesArray* FullMessage;
		WORD TimeToReSend;
		std::list<StoredMessage*>::iterator ThisInlist;
		StoredMessage(MessageHeader Header, BytesArray* FullMessage);
		~StoredMessage();
	};
	struct MessageInfo
	{
		MessageHeader Header;
		BytesArray* Array;
		sockaddr_in FromAddr;
		~MessageInfo();
	};
	struct RecvProc
	{
		function<bool(MessageInfo* Message)> Proc;
		RecvProc* Next;
	};

	static std::map<UINT, StoredMessage*> Messages;
	static std::mutex MessagesMutex;
	static StoredMessage* NewStoredMessage(MessageHeader Header, BytesArray* FullMessage);
	static void SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr);
	static void ConfirmRecvProc(MessageInfo* Message);
	static void NecResendProc();
	static bool Connected;
	template <typename... Types> static void Connect(char* IP, USHORT Port, chrono::milliseconds WaitTime, Types&&... Vals);
	template <typename... Types> static void SendToServer(MessageNetType NetType, WORD Type, Types&&... Vals);
	static SOCKET Socket;
	static UINT NextNecNumber;
	static void MainRecvProc();
	static std::map<UINT, RecvProc*> RecvProcs;
	static thread* RecvThread;
	static bool StunResponseWaited;
	static mutex RecvMutex;
	static condition_variable ConnectCond;
	static condition_variable StunResponseCond;
	static bool StunResponseRecved;
	static Syncronizer NecResendSync;
public:
	static void SendTo(BYTE* Buff, int BuffLen, sockaddr_in* DestAddr);
	static void SendConfirm(UINT Number, sockaddr_in* DestAddr);
public:
	static void Init();
	static void Bind(char* IP, USHORT Port);
	static void StartRecv();
	template <typename... Types> static void SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&&... Vals);
	static void RegisterRecvProc(WORD Type, function<bool(MessageInfo* Message)>& Proc);
	static void RegisterRecvProc(WORD Type, bool Proc(MessageInfo* Message));
	static void JoinRecv();
	static sockaddr_in GetSelfAddr(char* StunIP, USHORT StunPort);
};
template<class T> class RecvUnit;
template<class T> class SendUnit;

template <typename... Types> void Client::Connect(char* IP, USHORT Port, chrono::milliseconds WaitTime, Types&&... Vals)
{
	ServerAddr = MakeAddr(IP, Port);
	sockaddr_in DestAddr = MakeAddr(IP, Port);
	connect(Socket, (sockaddr*) &DestAddr, sizeof DestAddr);
	mutex ConnectMutex;
	unique_lock<mutex> ConnectLock(ConnectMutex);
	SendToServer(Nec, ConnectRequest, Vals...);
	ConnectCond.wait_for(ConnectLock, WaitTime);
	if (Connected) thread* NecResendThread = new thread(NecResendProc);
}
template <typename... Types> void Client::SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&&... Vals)
{
	BytesArray* FullMessage = new BytesArray(VoidBytes(MessageHeaderSize), Vals...);
	MessageHeader Header;
	Header.NetType = NetType;
	Header.Type = Type;
	if (NetType == Nec)
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
template <typename... Types> void Client::SendToServer(MessageNetType NetType, WORD Type, Types&&... Vals)
{
	SendTo(NetType, Type, &ServerAddr, Vals...);
}

template<> class RecvUnit<BaseUnit>:
	public ManagedUnit<RecvUnit<BaseUnit>>
{
public:
	RecvUnit();
	UINT ID;
	static std::map<UINT, RecvUnit<BaseUnit>*> Manager;
	static std::mutex ManagerMutex;
	virtual void Recved() {}
	~RecvUnit();
};
template<class T> class RecvUnit:
	public RecvUnit<BaseUnit>
{
public:
	static void Register(UINT& TypeID);
	static WORD MessageType;
	static bool RecvProc(Client::MessageInfo* Message);
};
template<class T> WORD RecvUnit<T>::MessageType;
template<class T> void RecvUnit<T>::Register(UINT& TypeID)
{
	MessageType = TypeID;
	Client::RegisterRecvProc(TypeID++, RecvProc);
}
template<class T> bool RecvUnit<T>::RecvProc(Client::MessageInfo* Message)
{
	UINT ID;
	T* RecvedObject;
	Message->Array->Deserialize(ID);
	T* ExistedObject = (T*) Manager[ID];
	if (ExistedObject == NULL)
	{
		RecvedObject = new T;
		Manager[ID]= (RecvUnit<BaseUnit>*) RecvedObject;
		RecvedObject->RecvUnit<T>::ID = ID;
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
	void Send() override;
	static WORD MessageType;
};
template<class T> void SendUnit<T>::Register(UINT& TypeID)
{
	MessageType = TypeID++;
}
template<class T> void SendUnit<T>::Send()
{
	Client::SendToServer(NoNec, MessageType, ID, *((T*)this));
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