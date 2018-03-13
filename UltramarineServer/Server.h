#pragma once
#include "Net.h"
#include "Management.h"
class Player;

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
		ListData<StoredMessage>* ThisInlist;
		StoredMessage(MessageHeader Header, BytesArray* FullMessage, sockaddr_in ClientAddr);
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
	RecvProc* LastRecvProc = NULL;
	SOCKET Socket;
	UINT NextNecNumber;
	static void MainRecvProc(Server* This);
	static void ClientsConnectCheckProc(Server* This);
	IDManager<RecvProc, 256, 256> RecvProcsManager;
	thread* RecvThread;
	thread* ClientsConnectCheckThread;
	bool StunResponseWaited = false;
	mutex RecvMutex;
	condition_variable StunResponseCond;
	bool StunResponseRecved = false;
	void SendTo(BYTE* Buff, int BuffLen, sockaddr_in* DestAddr);
	void SendConfirm(UINT Number, sockaddr_in* DestAddr);
public:
	void Bind(char* IP, USHORT Port);
	void StartRecv();
	void Connect(char* IP, USHORT Port);
	template <typename... Types> void SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals);
	void RegisterRecvProc(WORD Type, bool ForClientsOnly, function<bool(MessageInfo* Message)>& Proc);
	void RegisterRecvProc(WORD Type, bool ForClientsOnly, bool Proc(MessageInfo* Message));
	void JoinRecv();
	sockaddr_in GetSelfAddr(char* StunIP, USHORT StunPort);
	IDManager<StoredMessage, 65536, 65536> MessagesManager;
	List<StoredMessage> MessagesList;
	StoredMessage* NewStoredMessage(MessageHeader Header, BytesArray* FullMessage, sockaddr_in ClientAddr);
	Client* Clients[16];
	int ClientsCount = 0;
	void SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr);
	void ConfirmRecvProc(MessageInfo* Message);
	static void NecResendProc(Server* This);
	virtual void DeleteClient(Client* Client){};
	~Server();
public:
	Server(WORD Port);
	template <typename... Types> void SendToClient(MessageNetType NetType, WORD Type, BYTE ClientID, Types&... Vals);
	template <typename... Types> void SendToAll(MessageNetType NetType, WORD Type, Types&... Vals);
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
	for (int i = 0; i < ClientsCount; i++)
	{
		if (Clients[i] != NULL) SendToClient(NetType, Type, i, Vals...);
	}
}