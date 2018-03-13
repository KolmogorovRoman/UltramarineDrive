#pragma once
#include <Windows.h>
#include <WinSock.h>
#include <thread>
#include <algorithm>
#include <condition_variable>
#include "Serialization.h"
#include "IDManagement.h"
using namespace std;

extern int SockaddrSize;
enum MessagesTypes: UINT16
{
	ConnectRequest = 0,
	ConnectSuccess,
	ConnectFail,
	DeleteUnit,
	ReservedCount
};

enum MessageNetType: UINT16
{
	NoNec = 0b00,
	Nec = 0b01,
	Confirm = 0b10
};

struct StunMessageHeader
{
	UINT16 Type;
	UINT16 Length;
	UINT32 MagicCookie;
	UINT8 TransactionID[12];
};
void Serialize(StunMessageHeader& Header);
struct StunAtrubteHeader
{
	UINT16 Type;
	UINT16 Length;
};
void Serialize(StunAtrubteHeader& Header);
struct MessageHeader
{
	UINT16 Const = 'UD';
	MessageNetType NetType;
	UINT16 Type;
	UINT32 Number;
	UINT16 Length;
};
void Serialize(MessageHeader& Header);
const UINT MessageHeaderSize = Sizeof(MessageHeader());

sockaddr_in MakeAddr(char* IP, USHORT Port);

//class Net
//{
//	SOCKET Socket;
//	UINT NextNecNumber;
//	static void MainRecvProc(Net* This);
//	IDManager<function<void(sockaddr_in Addr, BytesArray* Array)>, 256, 256> RecvProcsManager;
//	thread* RecvThread;
//	bool StunResponseWaited = false;
//	mutex RecvMutex;
//	condition_variable StunResponseCond;
//	bool StunResponseRecved = false;
//protected:
//	void SendTo(BYTE* Buff, int BuffLen, sockaddr_in* DestAddr);
//	void SendConfirm(UINT Number, sockaddr_in* DestAddr);
//	virtual void SendNecProc(MessageHeader Header, BytesArray* Message, sockaddr_in ClientAddr) = 0;
//	virtual void ConfirmRecvProc(MessageHeader Header, sockaddr_in ClientAddr) = 0;
//public:
//	Net();
//	Net(char* IP, USHORT Port);
//	void Bind(char* IP, USHORT Port);
//	void StartRecv();
//	void Connect(char* IP, USHORT Port);
//	template <typename... Types> void SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals);
//	void RegisterRecvProc(WORD Type, function<void(sockaddr_in Addr, BytesArray* Array)>& Proc);
//	void RegisterRecvProc(WORD Type, void Proc(sockaddr_in Addr, BytesArray* Array));
//	void JoinRecv();
//	sockaddr_in GetSelfAddr(char* StunIP, USHORT StunPort);
//	void SendNatPmp(USHORT Port, UINT Time);
//	~Net();
//};
//template <typename... Types> void Net::SendTo(MessageNetType NetType, WORD Type, sockaddr_in* DestAddr, Types&... Vals)
//{
//	BytesArray* FullMessage = new BytesArray(VoidBytes(MessageHeaderSize), Vals...);
//	MessageHeader Header;
//	Header.NetType=NetType; 
//Header.Type=Type;
//	if (Header.NetType == Nec)
//		Header.Number = NextNecNumber++;
//	else Header.Number = 0;
//	Header.Length = FullMessage->Size;
//	FullMessage->Serialize(Header);
//	SendTo(FullMessage->Array, FullMessage->Size, DestAddr);
//	if (NetType == Nec)
//		SendNecProc(Header, FullMessage, *DestAddr);
//	else
//		delete FullMessage;
//}