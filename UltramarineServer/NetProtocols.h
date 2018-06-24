#pragma once
#include <Windows.h>
#include <WinSock.h>
#include <thread>
#include <algorithm>
#include <condition_variable>
#include "Serialization.h"
using namespace std;

extern int SockaddrSize;
enum MessagesTypes: UINT16
{
	ConnectRequest = 0,
	ConnectSuccess,
	ConnectFail,
	DeleteUnit
};

enum MessageNetType: UINT16
{
	NoNec = 0b00,
	Nec = 0b01,
	Confirm = 0b10
};

struct StunMessageHeader:Serialazable
{
	UINT16 Type;
	UINT16 Length;
	UINT32 MagicCookie;
	UINT8 TransactionID[12];
	void Serialize();
};
struct StunAtrubteHeader:Serialazable
{
	UINT16 Type;
	UINT16 Length;
	void Serialize();
};
struct MessageHeader:Serialazable
{
	UINT16 Const = 'UD';
	MessageNetType NetType;
	UINT16 Type;
	UINT32 Number;
	UINT16 Length;
	void Serialize();
};
const UINT MessageHeaderSize = Sizeof(MessageHeader());

sockaddr_in MakeAddr(char* IP, USHORT Port);