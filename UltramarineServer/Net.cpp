#include "Net.h"

int SockaddrSize = sizeof(SOCKADDR);

void Serialize(StunMessageHeader& Header)
{
	Serialize(Header.Type, Header.Length, Header.MagicCookie, Header.TransactionID);
}
void Serialize(StunAtrubteHeader& Header)
{
	Serialize(Header.Type, Header.Length);
}
void Serialize(MessageHeader& Header)
{
	if (CopyingToArray)
	{
		Header.Type &= 0b0011111111111111;
		Header.Type |= ((UINT16) (Header.NetType) << 14);
	}
	Serialize(Header.Const, Header.Type, Header.Number, Header.Length);
	if (CopyingFromArray)
	{
		Header.NetType = (MessageNetType)(Header.Type >> 14);
		Header.Type &= 0b0011111111111111;
	}
}

sockaddr_in MakeAddr(char* IP, USHORT Port)
{
	sockaddr_in Addr;
	Addr.sin_family = AF_INET;
	if (IP != NULL) Addr.sin_addr.S_un.S_addr = inet_addr(IP);
	else Addr.sin_addr.S_un.S_addr = INADDR_ANY;
	Addr.sin_port = htons(Port);
	ZeroMemory(Addr.sin_zero, sizeof Addr.sin_zero);
	return Addr;
}

//void Net::SendTo(BYTE* Buff, int BuffLen, SOCKADDR_IN* DestAddr)
//{
//	sendto(Socket, (char*) Buff, BuffLen, 0, (sockaddr*) DestAddr, sizeof sockaddr);
//}
//void Net::SendConfirm(UINT Number, sockaddr_in* DestAddr)
//{
//	MessageHeader Header;
//	Header.NetType=Confirm;
//	Header.Number = Number;
//	BytesArray Message(Header);
//	SendTo(Message.Array, Message.Size, DestAddr);
//}
//Net::Net()
//{
//	WSADATA wsadata;
//	WSAStartup(0x0202, &wsadata);
//	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	NextNecNumber = 0;
//}
//Net::Net(char* IP, USHORT Port)
//{
//	WSADATA wsadata;
//	WSAStartup(0x0202, &wsadata);
//	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	NextNecNumber = 0;
//	Bind(IP, Port);
//	StartRecv();
//}
//void Net::MainRecvProc(Net* This)
//{
//	while (true)
//	{
//		BytesArray* Array = NewArray(MessageHeaderSize);
//		sockaddr_in FromAddr;
//		MessageHeader Header;
//		recvfrom(This->Socket, (char*) Array->Array, MessageHeaderSize, MSG_PEEK, (sockaddr*) &FromAddr, &SockaddrSize);
//		Array->Deserialize(Header);
//		if (Header.Const != 'UD')
//		{
//			if (This->StunResponseWaited)
//			{
//				This->StunResponseRecved = true;
//				This->StunResponseCond.notify_one();
//			}
//			else
//			{
//				recvfrom(This->Socket, NULL, 0, 0, NULL, NULL);
//			}
//			continue;
//		}
//		if (Header.NetType == Confirm)
//		{
//			recvfrom(This->Socket, NULL, 0, 0, NULL, NULL);
//			This->ConfirmRecvProc(Header, FromAddr);
//			continue;
//		}
//		Array->Recreate(Header.Length);
//		recvfrom(This->Socket, (char*) Array->Array, Header.Length, 0, (sockaddr*) &FromAddr, &SockaddrSize);
//		if (Header.NetType == Nec)
//		{
//			This->SendConfirm(Header.Number, &FromAddr);
//		}
//		WORD Type = Header.Type();
//		function<void(sockaddr_in, BytesArray*)>* RecvProc = This->RecvProcsManager.Get(Type);
//		if (RecvProc!=NULL) (*RecvProc)(FromAddr, Array);
//	}
//}
//void Net::Bind(char* IP, USHORT Port)
//{
//	sockaddr_in Addr = MakeAddr(IP, Port);
//	int r = bind(Socket, (const sockaddr*) &Addr, (int) sizeof sockaddr);
//}
//void Net::StartRecv()
//{
//	RecvThread = new thread(MainRecvProc, this);
//}
//void Net::Connect(char* IP, USHORT Port)
//{
//	sockaddr_in DestAddr = MakeAddr(IP, Port);
//	connect(Socket, (sockaddr*) &DestAddr, sizeof DestAddr);
//}
//void Net::RegisterRecvProc(WORD Type, function<void(sockaddr_in Addr, BytesArray* Array)>& Proc)
//{
//	if (RecvProcsManager.Get(Type)!=NULL) throw new exception("Proc already registered");
//	RecvProcsManager.Place(Type, &Proc);
//}
//void Net::RegisterRecvProc(WORD Type, void Proc(sockaddr_in Addr, BytesArray* Array))
//{
//	function<void(sockaddr_in Addr, BytesArray* Array)>* funcProc = new function<void(sockaddr_in Addr, BytesArray* Array)>;
//	*funcProc = Proc;
//	RegisterRecvProc(Type, *funcProc);
//}
//void Net::JoinRecv()
//{
//	RecvThread->join();
//}
//sockaddr_in Net::GetSelfAddr(char* StunIP, USHORT StunPort)
//{
//	sockaddr_in StunAddr = MakeAddr(StunIP, StunPort);
//	sockaddr_in SelfAddr;
//	ZeroMemory(&SelfAddr, sizeof SelfAddr);
//	StunMessageHeader Header;
//	Header.Type = 0x0001;
//	Header.Length = 0;
//	Header.MagicCookie = 0x2112A442;
//	for (int i = 0; i < 12; i++)
//	{
//		Header.TransactionID[i] = rand() % 256;
//	}
//	BytesArray Message(Header);
//	SendTo((BYTE*) Message.Array, Message.Size, &StunAddr);
//	StunResponseWaited = true;
//	auto RecvFunc = [&]()->void
//	{
//		StunMessageHeader MessageHeader;
//		BytesArray Message = *NewArray(Sizeof(MessageHeader));
//		unique_lock<mutex> Lock(RecvMutex);
//		while (!StunResponseRecved)
//		{
//			StunResponseCond.wait(Lock);
//		}
//		StunResponseRecved = false;
//		recvfrom(Socket, (char*) Message.Array, Sizeof(MessageHeader), MSG_PEEK, (sockaddr*) NULL, NULL);
//		Message.Deserialize(MessageHeader);
//		Message.Recreate(MessageHeader.Length + Sizeof(MessageHeader));
//		//Message = *NewArray(MessageHeader.Length + Sizeof(MessageHeader));
//		recvfrom(Socket, (char*) Message.Array, MessageHeader.Length + sizeof MessageHeader, 0, (sockaddr*) NULL, NULL);
//		StunResponseWaited = false;
//		Message.Deserialize(MessageHeader);
//		ZeroMemory(&(SelfAddr.sin_zero), sizeof SelfAddr.sin_zero);
//		while (Message.Pointer < Message.Size)
//		{
//			StunAtrubteHeader CurrentHeader;
//			Message.Deserialize(CurrentHeader);
//			if (CurrentHeader.Type == 0x0001 || CurrentHeader.Type == 0x0020)
//			{
//				Message.Pointer += 2;
//				Message.Deserialize(SelfAddr.sin_port, SelfAddr.sin_addr);
//				if (CurrentHeader.Type == 0x0020)
//				{
//					SelfAddr.sin_port ^= 0x2112;
//					SelfAddr.sin_addr.S_un.S_addr ^= 0x2112A442;
//				}
//				SelfAddr.sin_family = AF_INET;
//				break;
//			}
//			else
//			{
//				Message.Pointer += CurrentHeader.Length;
//			}
//		}
//	};
//	thread Thread(RecvFunc);
//	Thread.join();
//	SelfAddr.sin_addr.S_un.S_addr = ntohl(SelfAddr.sin_addr.S_un.S_addr);
//	SelfAddr.sin_port = ntohs(SelfAddr.sin_port);
//	return SelfAddr;
//}
//void Net::SendNatPmp(USHORT Port, UINT Time)
//{
//	sockaddr_in TempAddr = MakeAddr(NULL, 5351);
//	SOCKET TempSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	bind(TempSocket, (const sockaddr*) &TempAddr, (int) sizeof TempAddr);
//	sockaddr_in DestAddr = MakeAddr("192.168.1.1", 5351);
//	BYTE Arr[4] = {0, 1, 0, 0};
//	Port = htons(Port);
//	Time = htonl(Time);
//	BytesArray Message(Arr, Port, Port, Time);
//	sendto(TempSocket, (char*) Message.Array, Message.Size, 0, (sockaddr*) &DestAddr, sizeof DestAddr);
//	recvfrom(TempSocket, (char*) Message.Array, Message.Size, MSG_PEEK, (sockaddr*) NULL, NULL);
//	closesocket(TempSocket);
//}
//Net::~Net()
//{}