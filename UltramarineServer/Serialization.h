#pragma once
#include <Windows.h>

typedef char SBYTE;
typedef short SWORD;
typedef int SDWORD;
typedef unsigned long long QWORD;
typedef long long SQWORD;

struct VoidBytes;
struct BytesArray;
class BytesArraysList;

enum SerializingStage
{
	enum_SizeCalculation,
	enum_CopyingToArray,
	enum_CopyingFromArray
};

extern thread_local BytesArray* CurrentArray;
extern thread_local SerializingStage CurrentStage;

template<typename T, typename... Types> inline void Serialize(T& Val, Types&... Vals);
template<typename T> inline void Serialize(T& Val);

struct BytesArray
{
	UINT Size = 0;
	BYTE* Array = NULL;
	UINT Pointer = 0;
	BytesArray();
	BytesArray(BytesArray& Other);
	template <typename... Types> BytesArray(Types&... Vals)
	{
		CurrentArray = this;
		CurrentStage = enum_SizeCalculation;
		::Serialize(Vals...);
		Array = new BYTE[Size];
		CurrentStage = enum_CopyingToArray;
		::Serialize(Vals...);
		Pointer = 0;
	}
	void Recreate(UINT Size);
	void Recreate(void* Pointer);
	void Recreate(UINT Size, void* Pointer);
	void ResetPointer();
	template <typename... Types> void Serialize(Types&... Vals)
	{
		CurrentArray = this;
		CurrentStage = enum_CopyingToArray;
		::Serialize(Vals...);
	}
	template <typename... Types> void Deserialize(Types&... Vals)
	{
		CurrentArray = this;
		CurrentStage = enum_CopyingFromArray;
		::Serialize(Vals...);
	}
	~BytesArray()
	{
		delete[] Array;
	};
};
BytesArray* NewArray(UINT Size);
BytesArray* NewArray(UINT Size, void* Pointer);
void MovePointer(UINT Size);
void IncSize(UINT Size);
void AddBytes(void* Source, UINT Size);
void ReadBytes(void* Dest, UINT Size);

template <typename... Types> UINT Sizeof(Types&... Vals)
{
	BytesArray Array;
	CurrentArray = &Array;
	CurrentStage = enum_SizeCalculation;
	Serialize(Vals...);
	return Array.Size;
}

template <typename T, typename... Types> void Serialize(T& Val, Types&... Vals)
{
	Serialize(Val);
	Serialize(Vals...);
}

#define SizeCalculation \
(CurrentStage == enum_SizeCalculation)

#define CopyingToArray \
(CurrentStage == enum_CopyingToArray)

#define CopyingFromArray \
(CurrentStage == enum_CopyingFromArray)

template <typename T> void Serialize(T& t)
{
	if (SizeCalculation)
	{
		IncSize(sizeof t);
	}
	if (CopyingToArray)
	{
		AddBytes(&t, sizeof t);
	}
	if (CopyingFromArray)
	{
		ReadBytes(&t, sizeof t);
	}
}
//template <typename T> void Serialize(T*& str)
//{
//	if (SizeCalculation)
//	{
//		IncSize(sizeof(strlen(str)) + strlen(str) + 1);
//	}
//	if (CopyingToArray)
//	{
//		UINT Len = strlen(str) + 1;
//		AddBytes(&Len, sizeof Len);
//		AddBytes(str, Len);
//	}
//	if (CopyingFromArray)
//	{
//		UINT Len;
//		ReadBytes(&Len, sizeof Len);
//		str = new char[Len];
//		ReadBytes(str, Len);
//	}
//}
template <> void Serialize(char*& str);

struct VoidBytes
{
	UINT Size;
	VoidBytes(UINT Size):
		Size(Size)
	{}
};
void Serialize(VoidBytes& Val);