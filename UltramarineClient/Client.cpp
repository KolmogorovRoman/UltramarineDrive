#include "Client.h"

sockaddr_in Client::ServerAddr;
BYTE Client::ID;
std::map<UINT, Client::StoredMessage*> Client::Messages;
std::mutex Client::MessagesMutex;
bool Client::Connected = false;
SOCKET Client::Socket;
UINT Client::NextNecNumber;
std::map<UINT, Client::RecvProc*> Client::RecvProcs;
thread* Client::RecvThread;
bool Client::StunResponseWaited = false;
mutex Client::RecvMutex;
condition_variable Client::ConnectCond;
condition_variable Client::StunResponseCond;
bool Client::StunResponseRecved = false;
Syncronizer Client::NecResendSync;
Client::StoredMessage::StoredMessage(MessageHeader Header, BytesArray* FullMessage):
	Header(Header),
	FullMessage(FullMessage)
{}
Client::StoredMessage::~StoredMessage()
{
	delete FullMessage;
}
Client::MessageInfo::~MessageInfo()
{
	delete Array;
}
Client::StoredMessage* Client::NewStoredMessage(MessageHeader Header, BytesArray* FullMessage)
{
	StoredMessage* Message = new StoredMessage(Header, FullMessage);
	Message->TimeToReSend = 1000;
	return Message;
}
void Client::SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr)
{
	StoredMessage* NewMessage = NewStoredMessage(Header, Message);
	Messages[Header.Number] = NewMessage;
}
void Client::ConfirmRecvProc(MessageInfo* Message)
{
	StoredMessage* StoredMessage = Messages[Message->Header.Number];
	Messages.erase(Message->Header.Number);
	if (StoredMessage != NULL)
	{
		delete StoredMessage;
	}
}
void Client::NecResendProc()
{
	while (true)
	{
		MessagesMutex.lock();
		for (auto Message : Messages)
		{
			if (Message.second->TimeToReSend <= 0)
			{
				SendTo(Message.second->FullMessage->Array, Message.second->FullMessage->Size, &ServerAddr);
				Message.second->TimeToReSend = 50;
			}
			else
				Message.second->TimeToReSend--;
		}
		MessagesMutex.unlock();
		//Sleep(1);
		NecResendSync.Sync(1ms);
	}
}
void Client::SendTo(BYTE* Buff, int BuffLen, SOCKADDR_IN* DestAddr)
{
	int res=sendto(Socket, (char*) Buff, BuffLen, 0, (sockaddr*) DestAddr, sizeof sockaddr);
	int err = WSAGetLastError();
}
void Client::SendConfirm(UINT Number, sockaddr_in* DestAddr)
{
	MessageHeader Header;
	Header.NetType = Confirm;
	Header.Number = Number;
	BytesArray Message(Header);
	SendTo(Message.Array, Message.Size, DestAddr);
}
void Client::Init()
{
	WSADATA wsadata;
	WSAStartup(0x0202, &wsadata);
	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	RegisterRecvProc(ConnectSuccess, (function<bool(MessageInfo*)>) [](MessageInfo* Message)
	{
		Connected = true;
		ConnectCond.notify_one();
		Message->Array->Deserialize(ID);
		//cout << "Connected";
		return true;
	});
	RegisterRecvProc(DeleteUnit, [](Client::MessageInfo* Message)
	{
		UINT ID;
		Message->Array->Deserialize(ID);
		RecvUnit<BaseUnit>::ManagerMutex.lock();
		RecvUnit<BaseUnit>* Unit = RecvUnit<BaseUnit>::Manager[ID];
		size_t res = RecvUnit<BaseUnit>::Manager.erase(ID);
		RecvUnit<BaseUnit>::ManagerMutex.unlock();
		if (Unit != NULL)
		{
			Unit->Delete();
		}
		return true;
	});
}
void Client::MainRecvProc()
{
	while (true)
	{
		MessageInfo Message;
		Message.Array = BytesArray::New(MessageHeaderSize);
		recvfrom(Socket, (char*) Message.Array->Array, MessageHeaderSize, MSG_PEEK, (sockaddr*) &Message.FromAddr, &SockaddrSize);
		Message.Array->Deserialize(Message.Header);
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
			if (RecvProc->Proc(&Message) == false) break;
			RecvProc = RecvProc->Next;
		}
	}
}
void Client::Bind(char* IP, USHORT Port)
{
	sockaddr_in Addr = MakeAddr(IP, Port);
	int r = bind(Socket, (const sockaddr*) &Addr, (int) sizeof sockaddr);
}
void Client::StartRecv()
{
	RecvThread = new thread(MainRecvProc);
}

void Client::RegisterRecvProc(WORD Type, function<bool(MessageInfo* Message)>& Proc)
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
	Last->Next = NULL;
}
void Client::RegisterRecvProc(WORD Type, bool Proc(MessageInfo* Message))
{
	function<bool(MessageInfo*)>* funcProc = new function<bool(MessageInfo* Message)>;
	*funcProc = Proc;
	RegisterRecvProc(Type, *funcProc);
}
void Client::JoinRecv()
{
	RecvThread->join();
}
sockaddr_in Client::GetSelfAddr(char* StunIP, USHORT StunPort)
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
		//Message = *NewArray(MessageHeader.Length + Sizeof(MessageHeader));
		recvfrom(Socket, (char*) Message.Array, MessageHeader.Length + sizeof MessageHeader, 0, (sockaddr*) NULL, NULL);
		StunResponseWaited = false;
		Message.Deserialize(MessageHeader);
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
	SelfAddr.sin_port = ntohs(SelfAddr.sin_port);
	return SelfAddr;
}

RecvUnit<BaseUnit>::RecvUnit()
{}
std::map<UINT, RecvUnit<BaseUnit>*> RecvUnit<BaseUnit>::Manager;
std::mutex RecvUnit<BaseUnit>::ManagerMutex;
RecvUnit<BaseUnit>::~RecvUnit()
{
	ManagerMutex.lock();
	size_t res = Manager.erase(ID);
	ManagerMutex.unlock();
}
void RecvUnit<char>::Register(UINT& TypeID)
{}
UINT NextRecvType = MessagesTypes::DeleteUnit+1;


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
	Client::SendToServer(Nec, DeleteUnit, ID);
}
void SendUnit<char>::Register(UINT& TypeID)
{}
UINT NextSendType = MessagesTypes::DeleteUnit + 1;