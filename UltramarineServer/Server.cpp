#include "Server.h"

Server::Client::Client(sockaddr_in Addr, BYTE ID):
	Addr(Addr), ID(ID), Player(NULL), TimeToLastRecv(0)
{}
Server::Client::~Client()
{
	//delete Player;
}
Server::Server(WORD Port)
{
	WSADATA wsadata;
	WSAStartup(0x0202, &wsadata);
	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	NextNecNumber = 0;
	Bind(NULL, Port);
	thread* NecResendThread = new thread(NecResendProc, this);
	NecResendThread->detach();
	RegisterRecvProc(ConnectRequest, false, (function<bool(MessageInfo*)>) [&](MessageInfo* Message)
	{
		for (int i = 0; i < ClientsCount; i++)
		{
			if (Clients[i]->Addr.sin_addr.S_un.S_addr == Message->FromAddr.sin_addr.S_un.S_addr && Clients[i]->Addr.sin_port == Message->FromAddr.sin_port)
				return false;
		}
		Clients[ClientsCount] = new Client(Message->FromAddr, ClientsCount);
		Message->Client = Clients[ClientsCount];
		SendToClient(Nec, ConnectSuccess, ClientsCount, ClientsCount);
		cout << "Client [" << ClientsCount << "] connected." << std::endl;
		ClientsCount++;
		return true;
	});
	StartRecv();
}
Server::StoredMessage::StoredMessage(MessageHeader Header, BytesArray* FullMessage, sockaddr_in ClientAddr):
	Header(Header),
	FullMessage(FullMessage),
	ClientAddr(ClientAddr)
{}
Server::StoredMessage::~StoredMessage()
{
	if (FullMessage != NULL) delete FullMessage;
}
Server::StoredMessage* Server::NewStoredMessage(MessageHeader Header, BytesArray* FullMessage, sockaddr_in ClientAddr)
{
	StoredMessage* Message = new StoredMessage(Header, FullMessage, ClientAddr);
	Message->TimeToReSend = 1000;
	return Message;
}
void Server::SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr)
{
	StoredMessage* NewMessage = NewStoredMessage(Header, Message, ClientAddr);
	MessagesManager.Place(Header.Number, NewMessage);
	MessagesList.push_back(NewMessage);
	NewMessage->ThisInlist = std::prev(MessagesList.end());
}
void Server::ConfirmRecvProc(MessageInfo* Message)
{
	StoredMessage* StoredMessage = MessagesManager.Free(Message->Header.Number);
	if (StoredMessage != NULL)
	{
		MessagesList.erase(StoredMessage->ThisInlist);
		delete StoredMessage;
	}
}
void Server::NecResendProc(Server* This)
{
	while (true)
	{
		for (auto Message : This->MessagesList)
		{
			if (Message->TimeToReSend <= 0)
			{
				This->SendTo(Message->FullMessage->Array, Message->FullMessage->Size, &Message->ClientAddr);
				Message->TimeToReSend = 50;
			}
			else
				Message->TimeToReSend--;
		}
		Sleep(1);
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
void Server::MainRecvProc(Server* This)
{
	while (true)
	{
		MessageInfo Message;
		Message.Array = NewArray(MessageHeaderSize);
		int res = recvfrom(This->Socket, (char*) Message.Array->Array, MessageHeaderSize, MSG_PEEK, (sockaddr*) &Message.FromAddr, &SockaddrSize);
		int err = WSAGetLastError();
		if (err == 10054)
			continue;
		Message.Array->Deserialize(Message.Header);
		Message.Client = NULL;
		for (int i = 0; i < This->ClientsCount; i++)
		{
			if (This->Clients[i] != NULL
				&&
				This->Clients[i]->Addr.sin_addr.S_un.S_addr == Message.FromAddr.sin_addr.S_un.S_addr
				&&
				This->Clients[i]->Addr.sin_port == Message.FromAddr.sin_port
				)
			{
				Message.Client = This->Clients[i];
				Message.Client->TimeToLastRecv = 0;
				break;
			}
		}
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
			if (RecvProc->ForClientsOnly && Message.Client == NULL) break;
			if (RecvProc->Proc(&Message) == false) break;
			RecvProc = RecvProc->Next;
		}
	}
}
void Server::ClientsConnectCheckProc(Server* This)
{
	while (true)
	{
		for (int i = 0; i < This->ClientsCount; i++)
		{
			if (This->Clients[i] != NULL)
			{
				This->Clients[i]->TimeToLastRecv++;
				if (This->Clients[i]->TimeToLastRecv >= 1000)
				{
					/*cout << "Client [" << i << "] disconnected." << std::endl;
					delete This->Clients[i];
					This->Clients[i] = NULL;*/
					This->DeleteClient(This->Clients[i]);
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
	RecvThread = new thread(MainRecvProc, this);
	ClientsConnectCheckThread = new thread(ClientsConnectCheckProc, this);
	ClientsConnectCheckThread->detach();
}
void Server::Connect(char* IP, USHORT Port)
{
	sockaddr_in DestAddr = MakeAddr(IP, Port);
	connect(Socket, (sockaddr*) &DestAddr, sizeof DestAddr);
}
void Server::RegisterRecvProc(WORD Type, bool ForClientsOnly, function<bool(MessageInfo* Message)>& Proc)
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
Server::~Server()
{
	closesocket(Socket);
}