#include "NetProtocols.h"

int SockaddrSize = sizeof(SOCKADDR);

UINT NextMessageType = MessagesTypes::DeleteUnit + 1;

void StunMessageHeader::Serialize()
{
	Serial(Type, Length, MagicCookie, TransactionID);
}
void StunAtrubteHeader::Serialize()
{
	Serial(Type, Length);
}
void MessageHeader::Serialize()
{
	if (Serialazing)
	{
		Type &= 0b0011111111111111;
		Type |= ((UINT16) (NetType) << 14);
	}
	Serial(Const, Type, Number, Length);
	if (Deserialazing)
	{
		NetType = (MessageNetType) (Type >> 14);
		Type &= 0b0011111111111111;
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