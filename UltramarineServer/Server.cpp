#include "Server.h"

SOCKET Server::Socket;
UINT Server::NextNecNumber = 0;
std::map<UINT, Server::RecvProc*> Server::RecvProcs;
thread* Server::RecvThread;
thread* Server::ClientsConnectCheckThread;
bool Server::StunResponseWaited;
mutex Server::RecvMutex;
condition_variable Server::StunResponseCond;
bool Server::StunResponseRecved;
std::map<UINT, Server::StoredMessage*> Server::Messages;
Server::Client* Server::Clients[256];
CRITICAL_SECTION Server::MessagesListCS;
Syncronizer Server::NecResendSync;
Server::Client::Client(sockaddr_in Addr, BYTE ID):
	Addr(Addr), ID(ID), Player(NULL), TimeToLastRecv(0)
{}
Server::Client::~Client()
{
	//delete Player;
}
void Server::Init(WORD Port)
{
	WSADATA wsadata;
	WSAStartup(0x0202, &wsadata);
	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	NextNecNumber = 0;
	for (int i = 0; i < 256; i++)
	{
		Clients[i] = NULL;
	}
	Bind(NULL, Port);
	InitializeCriticalSection(&MessagesListCS);
	RegisterRecvProc(ConnectRequest, false, (function<bool(MessageInfo*)>) [&](MessageInfo* Message)
	{
		for (int i = 0; i < 256; i++)
		{
			if (Clients[i] != NULL
				&&
				Clients[i]->Addr.sin_addr.S_un.S_addr == Message->FromAddr.sin_addr.S_un.S_addr
				&&
				Clients[i]->Addr.sin_port == Message->FromAddr.sin_port)
				return false;
		}
		BYTE NewID = 0;
		for (int i = 0; i < 256; i++)
		{
			if (Clients[i] == NULL)
			{
				NewID = i;
				break;
			}
		}
		Clients[NewID] = new Client(Message->FromAddr, NewID);
		Message->Client = Clients[NewID];
		SendToClient(Nec, ConnectSuccess, NewID, NewID);
		string NickName;
		auto MessagePointer = Message->Array->Pointer;
		Message->Array->Deserialize(NickName);
		Message->Array->Pointer = MessagePointer;
		cout << "Client [" << (int) NewID << ":" << NickName << "] connected." << std::endl;
		return true;
	});
}
Server::StoredMessage::StoredMessage(MessageHeader Header, BytesArray* FullMessage, sockaddr_in ClientAddr, WORD TimeToResend):
	Header(Header),
	FullMessage(FullMessage),
	ClientAddr(ClientAddr),
	TimeToReSend(TimeToReSend)
{}
Server::StoredMessage::~StoredMessage()
{
	if (FullMessage != NULL) delete FullMessage;
}
void Server::SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr)
{
	EnterCriticalSection(&MessagesListCS);
	StoredMessage* NewMessage = new StoredMessage(Header, Message, ClientAddr, 1000);
	Messages[Header.Number] = NewMessage;
	LeaveCriticalSection(&MessagesListCS);
}
void Server::ConfirmRecvProc(MessageInfo* Message)
{
	EnterCriticalSection(&MessagesListCS);
	StoredMessage* storedMessage = Messages[Message->Header.Number];
	Messages.erase(Message->Header.Number);
	if (storedMessage != NULL)
	{
		delete storedMessage;
	}
	LeaveCriticalSection(&MessagesListCS);
}
void Server::NecResendProc()
{
	while (true)
	{
		EnterCriticalSection(&MessagesListCS);
		for (auto Message : Messages)
		{
			if (Message.second->TimeToReSend <= 0)
			{
				SendTo(Message.second->FullMessage->Array, Message.second->FullMessage->Size, &Message.second->ClientAddr);
				Message.second->TimeToReSend = 50;
			}
			else
				Message.second->TimeToReSend--;
		}
		LeaveCriticalSection(&MessagesListCS);
		//Sleep(1);
		NecResendSync.Sync(1ms);
	}
}
void Server::SendTo(BYTE* Buff, int BuffLen, SOCKADDR_IN* DestAddr)
{
	sendto(Socket, (char*) Buff, BuffLen, 0, (sockaddr*) DestAddr, sizeof sockaddr);
}
void Server::SendConfirm(UINT Number, sockaddr_in* DestAddr)
{
	MessageHeader Header;
	Header.NetType = Confirm;
	Header.Number = Number;
	BytesArray Message(Header);
	SendTo(Message.Array, Message.Size, DestAddr);
}
void Server::MainRecvProc()
{
	while (true)
	{
		MessageInfo Message;
		Message.Array = BytesArray::New(MessageHeaderSize);
		int res = recvfrom(Socket, (char*) Message.Array->Array, MessageHeaderSize, MSG_PEEK, (sockaddr*) &Message.FromAddr, &SockaddrSize);
		int err = WSAGetLastError();
		if (err == 10054)
			continue;
		Message.Array->Deserialize(Message.Header);
		Message.Client = NULL;
		for (int i = 0; i < 256; i++)
		{
			if (Clients[i] != NULL
				&&
				Clients[i]->Addr.sin_addr.S_un.S_addr == Message.FromAddr.sin_addr.S_un.S_addr
				&&
				Clients[i]->Addr.sin_port == Message.FromAddr.sin_port
				)
			{
				Message.Client = Clients[i];
				Message.Client->TimeToLastRecv = 0;
				break;
			}
		}
		if (Message.Header.Const != 'UD')
		{
			if (StunResponseWaited)
			{
				StunResponseRecved = true;
				StunResponseCond.notify_one();
			}
			else
			{
				recvfrom(Socket, NULL, 0, 0, NULL, NULL);
			}
			continue;
		}
		if (Message.Header.NetType == Confirm)
		{
			recvfrom(Socket, NULL, 0, 0, NULL, NULL);
			ConfirmRecvProc(&Message);
			continue;
		}
		Message.Array->Recreate(Message.Header.Length);
		recvfrom(Socket, (char*) Message.Array->Array, Message.Header.Length, 0, (sockaddr*) &Message.FromAddr, &SockaddrSize);
		if (Message.Header.NetType == Nec)
		{
			SendConfirm(Message.Header.Number, &Message.FromAddr);
		}

		RecvProc* RecvProc = RecvProcs[Message.Header.Type];
		while (RecvProc != NULL)
		{
			if (RecvProc->ForClientsOnly && Message.Client == NULL) break;
			if (RecvProc->Proc(&Message) == false) break;
			RecvProc = RecvProc->Next;
		}
	}
}
void Server::ClientsConnectCheckProc()
{
	while (true)
	{
		for (int i = 0; i < 256; i++)
		{
			if (Clients[i] != NULL)
			{
				Clients[i]->TimeToLastRecv++;
				if (Clients[i]->TimeToLastRecv >= 1000)
				{
					DeleteClient(Clients[i]);
				}
			}
		}
		Sleep(1);
	}
}
void Server::Bind(char* IP, USHORT Port)
{
	sockaddr_in Addr = MakeAddr(IP, Port);
	int r = bind(Socket, (const sockaddr*) &Addr, (int) sizeof sockaddr);
}
void Server::StartRecv()
{
	RecvThread = new thread(MainRecvProc);
	RecvThread->detach();

	ClientsConnectCheckThread = new thread(ClientsConnectCheckProc);
	ClientsConnectCheckThread->detach();

	thread* NecResendThread = new thread(NecResendProc);
	NecResendThread->detach();
}
void Server::Connect(char* IP, USHORT Port)
{
	sockaddr_in DestAddr = MakeAddr(IP, Port);
	connect(Socket, (sockaddr*) &DestAddr, sizeof DestAddr);
}
void Server::RegisterRecvProc(WORD Type, bool ForClientsOnly, function<bool(MessageInfo* Message)>& Proc)
{
	RecvProc* Last = RecvProcs[Type];
	if (Last == NULL)
	{
		Last = new RecvProc();
		RecvProcs[Type] = Last;
	}
	else
	{
		Last->Next = new RecvProc();
		Last = Last->Next;
	}
	Last->Proc = Proc;
	Last->ForClientsOnly = ForClientsOnly;
	Last->Next = NULL;
}
void Server::RegisterRecvProc(WORD Type, bool ForClientsOnly, bool Proc(MessageInfo* Message))
{
	function<bool(MessageInfo* Message)>* funcProc = new function<bool(MessageInfo* Message)>;
	*funcProc = Proc;
	RegisterRecvProc(Type, ForClientsOnly, *funcProc);
}
void Server::JoinRecv()
{
	RecvThread->join();
}
sockaddr_in Server::GetSelfAddr(char* StunIP, USHORT StunPort)
{
	sockaddr_in StunAddr = MakeAddr(StunIP, StunPort);
	sockaddr_in SelfAddr;
	ZeroMemory(&SelfAddr, sizeof SelfAddr);
	StunMessageHeader Header;
	Header.Type = 0x0001;
	Header.Length = 0;
	Header.MagicCookie = 0x2112A442;
	for (int i = 0; i < 12; i++)
	{
		Header.TransactionID[i] = rand() % 256;
	}
	BytesArray Message(Header);
	SendTo((BYTE*) Message.Array, Message.Size, &StunAddr);
	StunResponseWaited = true;
	auto RecvFunc = [&]()->void
	{
		StunMessageHeader MessageHeader;
		BytesArray Message = *BytesArray::New(Sizeof(MessageHeader));
		unique_lock<mutex> Lock(RecvMutex);
		while (!StunResponseRecved)
		{
			StunResponseCond.wait(Lock);
		}
		StunResponseRecved = false;
		recvfrom(Socket, (char*) Message.Array, Sizeof(MessageHeader), MSG_PEEK, (sockaddr*) NULL, NULL);
		Message.Deserialize(MessageHeader);
		Message.Recreate(MessageHeader.Length + Sizeof(MessageHeader));
		recvfrom(Socket, (char*) Message.Array, MessageHeader.Length + sizeof MessageHeader, 0, (sockaddr*) NULL, NULL);
		StunResponseWaited = false;
		//Message.Deserialize(MessageHeader);
		ZeroMemory(&(SelfAddr.sin_zero), sizeof SelfAddr.sin_zero);
		while (Message.Pointer < Message.Size)
		{
			StunAtrubteHeader CurrentHeader;
			Message.Deserialize(CurrentHeader);
			if (CurrentHeader.Type == 0x0001 || CurrentHeader.Type == 0x0020)
			{
				Message.Pointer += 2;
				Message.Deserialize(SelfAddr.sin_port, SelfAddr.sin_addr);
				if (CurrentHeader.Type == 0x0020)
				{
					SelfAddr.sin_port ^= 0x2112;
					SelfAddr.sin_addr.S_un.S_addr ^= 0x2112A442;
				}
				SelfAddr.sin_family = AF_INET;
				break;
			}
			else
			{
				Message.Pointer += CurrentHeader.Length;
			}
		}
	};
	thread Thread(RecvFunc);
	Thread.join();
	SelfAddr.sin_addr.S_un.S_addr = ntohl(SelfAddr.sin_addr.S_un.S_addr);
	//SelfAddr.sin_port = ntohs(SelfAddr.sin_port);
	return SelfAddr;
}
void Server::DeleteClient(Client* Client)
{
	cout << "Client [" << (int) Client->ID << "] disconnected." << std::endl;
	BYTE ID = Client->ID;
	delete Client;
	Clients[ID] = NULL;
}


RecvUnit<BaseUnit>::RecvUnit()
{}
std::map<UINT, RecvUnit<BaseUnit>*> RecvUnit<BaseUnit>::Manager;
RecvUnit<BaseUnit>::~RecvUnit()
{
	Manager.erase(ID);
}
void RecvUnit<char>::Register(UINT& TypeID)
{}
UINT NextRecvType = MessagesTypes::DeleteUnit + 1;


SendUnit<BaseUnit>::SendUnit()
{
	ID = NextID++;
	Manager[ID] = this;
}
std::map<UINT, SendUnit<BaseUnit>*> SendUnit<BaseUnit>::Manager;
UINT SendUnit<BaseUnit>::NextID = 0;
SendUnit<BaseUnit>::~SendUnit()
{
	Manager.erase(ID);
	Server::SendToAll(Nec, DeleteUnit, ID);
}
void SendUnit<char>::Register(UINT& TypeID)
{}
UINT NextSendType = MessagesTypes::DeleteUnit + 1;