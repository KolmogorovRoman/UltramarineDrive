#include "Client.h"

Client::Client(USHORT Port)
{
	WSADATA wsadata;
	WSAStartup(0x0202, &wsadata);
	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	NextNecNumber = 0;
	Bind(NULL, Port);
	StartRecv();
}
Client::StoredMessage::StoredMessage(MessageHeader Header, BytesArray* FullMessage):
	Header(Header),
	FullMessage(FullMessage)
{}
Client::StoredMessage::~StoredMessage()
{
	ThisInlist->Remove();
	delete FullMessage;
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
	MessagesManager.Place(Header.Number, NewMessage);
	NewMessage->ThisInlist = MessagesList.Add(NewMessage);
}
void Client::ConfirmRecvProc(MessageInfo* Message)
{
	StoredMessage* StoredMessage = MessagesManager.Free(Message->Header.Number);
	if (StoredMessage != NULL)
	{
		StoredMessage->ThisInlist->Remove();
		delete StoredMessage;
	}
}
void Client::NecResendProc(Client* This)
{
	while (true)
	{
		ForList(This->MessagesList, Message)
		{
			if (Message->TimeToReSend <= 0)
			{
				This->SendTo(Message->FullMessage->Array, Message->FullMessage->Size, &This->ServerAddr);
				Message->TimeToReSend = 50;
			}
			else
				Message->TimeToReSend--;
		}
		Sleep(1);
	}
}
void Client::SendTo(BYTE* Buff, int BuffLen, SOCKADDR_IN* DestAddr)
{
	sendto(Socket, (char*) Buff, BuffLen, 0, (sockaddr*) DestAddr, sizeof sockaddr);
}
void Client::SendConfirm(UINT Number, sockaddr_in* DestAddr)
{
	MessageHeader Header;
	Header.NetType = Confirm;
	Header.Number = Number;
	BytesArray Message(Header);
	SendTo(Message.Array, Message.Size, DestAddr);
}
Client::Client()
{
	WSADATA wsadata;
	WSAStartup(0x0202, &wsadata);
	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	NextNecNumber = 0;
}
Client::Client(char* IP, USHORT Port)
{
	WSADATA wsadata;
	WSAStartup(0x0202, &wsadata);
	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	NextNecNumber = 0;
	Bind(IP, Port);
	StartRecv();
}
void Client::MainRecvProc(Client* This)
{
	while (true)
	{
		MessageInfo Message;
		Message.Array = NewArray(MessageHeaderSize);
		recvfrom(This->Socket, (char*) Message.Array->Array, MessageHeaderSize, MSG_PEEK, (sockaddr*) &Message.FromAddr, &SockaddrSize);
		Message.Array->Deserialize(Message.Header);
		if (Message.Header.Const != 'UD')
		{
			if (This->StunResponseWaited)
			{
				This->StunResponseRecved = true;
				This->StunResponseCond.notify_one();
			}
			else
			{
				recvfrom(This->Socket, NULL, 0, 0, NULL, NULL);
			}
			continue;
		}
		if (Message.Header.NetType == Confirm)
		{
			recvfrom(This->Socket, NULL, 0, 0, NULL, NULL);
			This->ConfirmRecvProc(&Message);
			continue;
		}
		Message.Array->Recreate(Message.Header.Length);
		recvfrom(This->Socket, (char*) Message.Array->Array, Message.Header.Length, 0, (sockaddr*) &Message.FromAddr, &SockaddrSize);
		if (Message.Header.NetType == Nec)
		{
			This->SendConfirm(Message.Header.Number, &Message.FromAddr);
		}

		RecvProc* RecvProc = This->RecvProcsManager.Get(Message.Header.Type);
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
	RecvThread = new thread(MainRecvProc, this);
}

void Client::RegisterRecvProc(WORD Type, function<bool(MessageInfo* Message)>& Proc)
{
	RecvProc* Last = RecvProcsManager.Get(Type);
	if (Last == NULL)
	{
		Last = new RecvProc();
		RecvProcsManager.Place(Type, Last);
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
		BytesArray Message = *NewArray(Sizeof(MessageHeader));
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
Client::~Client()
{
	closesocket(Socket);
	delete RecvThread;
	WSACleanup();
}