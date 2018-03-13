#pragma once
#include "Net.h"
#include "Management.h"

class Client
{
public:
	sockaddr_in ServerAddr;
	BYTE ID;
	struct StoredMessage
	{
		MessageHeader Header;
		BytesArray* FullMessage;
		WORD TimeToReSend;
		ListData<StoredMessage>* ThisInlist;
		StoredMessage(MessageHeader Header, BytesArray* FullMessage);
		~StoredMessage();
	};
	struct MessageInfo
	{
		MessageHeader Header;
		BytesArray* Array;
		sockaddr_in FromAddr;
	};
	struct RecvProc
	{
		function<bool(MessageInfo* Message)> Proc;
		RecvProc* Next;
	};
	RecvProc* LastRecvProc = NULL;

	IDManager<StoredMessage, 65536, 65536> MessagesManager;
	List<StoredMessage> MessagesList;
	StoredMessage* NewStoredMessage(MessageHeader Header, BytesArray* FullMessage);
	void SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr);
	void ConfirmRecvProc(MessageInfo* Message);
	static void NecResendProc(Client* This);
	bool Connected = false;
	Client(USHORT Port);
	template <typename... Types> void Connect(char* IP, USHORT Port, chrono::milliseconds WaitTime, Types&... Vals);
	template <typename... Types> void SendToServer(MessageNetType NetType, WORD Type, Types&... Vals);
	SOCKET Socket;
	UINT NextNecNumber;

	static void MainRecvProc(Client* This);
	IDManager<RecvProc, 256, 256> RecvProcsManager;
	thread* RecvThread;
	bool StunResponseWaited = false;
	mutex RecvMutex;
	condition_variable StunResponseCond;
	bool StunResponseRecved = false;
public:
	void SendTo(BYTE* Buff, int BuffLen, sockaddr_in* DestAddr);
	void SendConfirm(UINT Number, sockaddr_in* DestAddr);
public:
	Client();
	Client(char* IP, USHORT Port);
	void Bind(char* IP, USHORT Port);
	void StartRecv();
	template <typename... Types> void SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals);
	void RegisterRecvProc(WORD Type, function<bool(MessageInfo* Message)>& Proc);
	void RegisterRecvProc(WORD Type, bool Proc(MessageInfo* Message));
	void JoinRecv();
	sockaddr_in GetSelfAddr(char* StunIP, USHORT StunPort);
	~Client();
};

template <typename... Types> void Client::Connect(char* IP, USHORT Port, chrono::milliseconds WaitTime, Types&... Vals)
{
	ServerAddr = MakeAddr(IP, Port);
	sockaddr_in DestAddr = MakeAddr(IP, Port);
	connect(Socket, (sockaddr*) &DestAddr, sizeof DestAddr);
	condition_variable ConnectCond;
	mutex ConnectMutex;
	unique_lock<mutex> ConnectLock(ConnectMutex);
	RegisterRecvProc(ConnectSuccess, (function<bool(MessageInfo*)>) [&](MessageInfo* Message)
	{
		Connected = true;
		ConnectCond.notify_one();
		Message->Array->Deserialize(ID);
		//cout << "Connected";
		return true;
	});
	SendToServer(Nec, ConnectRequest, Vals...);
	ConnectCond.wait_for(ConnectLock, WaitTime);
	if (Connected) thread* NecResendThread = new thread(NecResendProc, this);
}
template <typename... Types> void Client::SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals)
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
template <typename... Types> void Client::SendToServer(MessageNetType NetType, WORD Type, Types&... Vals)
{
	SendTo(NetType, Type, &ServerAddr, Vals...);
}