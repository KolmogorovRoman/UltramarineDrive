#pragma once
#include <Windows.h>
#include <type_traits>
#include <string>

struct VoidBytes;
struct BytesArray;

class Serialazable
{};

enum class SerializingStage
{
	GetSize,
	CopyTo,
	CopyFrom
};
#define SizeCalculation \
(CurrentStage == SerializingStage::GetSize)
#define Serialazing \
(CurrentStage == SerializingStage::CopyTo)
#define Deserialazing \
(CurrentStage == SerializingStage::CopyFrom)

extern thread_local BytesArray* CurrentArray;
extern thread_local SerializingStage CurrentStage;

template <typename... Types> UINT Sizeof(Types&&... Vals)
{
	BytesArray Array;
	CurrentArray = &Array;
	CurrentStage = SerializingStage::GetSize;
	Serialize(Vals...);
	return Array.Size;
}

template <typename T>
typename std::enable_if<std::is_base_of<Serialazable, T>::value>::type Serialize(T& Val)
{
	Val.Serialize();
}
template <typename T>
typename std::enable_if<!std::is_base_of<Serialazable, T>::value>::type Serialize(T& Val)
{
	if (SizeCalculation) IncSize(sizeof Val);
	if (Serialazing) WriteBytes(&Val, sizeof Val);
	if (Deserialazing) ReadBytes(&Val, sizeof Val);
}
#define DECL_SERIALIZE_FOR_STRING(T) \
void Serialize(T*& Val);
DECL_SERIALIZE_FOR_STRING(char)
void Serialize(std::string& Val);

struct BytesArray
{
	UINT Size = 0;
	BYTE* Array = NULL;
	UINT Pointer = 0;
	BytesArray();
	BytesArray(BytesArray& Other);
	template <typename... Types> BytesArray(Types&&... Vals)
	{
		CurrentArray = this;
		CurrentStage = SerializingStage::GetSize;
		Serial(Vals...);
		Array = new BYTE[Size];
		CurrentStage = SerializingStage::CopyTo;
		Serial(Vals...);
		ResetPointer();
	}
	static BytesArray* New(UINT Size);
	static BytesArray* New(UINT Size, void* Pointer);
	void Recreate(UINT Size);
	void Recreate(void* Pointer);
	void Recreate(UINT Size, void* Pointer);
	void ResetPointer();
	template <typename... Types> void Serialize(Types&&... Vals)
	{
		CurrentArray = this;
		CurrentStage = SerializingStage::CopyTo;
		Serial(Vals...);
	}
	template <typename... Types> void Deserialize(Types&&... Vals)
	{
		CurrentArray = this;
		CurrentStage = SerializingStage::CopyFrom;
		Serial(Vals...);
	}
	~BytesArray()
	{
		delete[] Array;
	};
};

void MovePointer(UINT Size);
void IncSize(UINT Size);
void WriteBytes(const void* Source, UINT Size);
void ReadBytes(void* Dest, UINT Size);

template <typename T, typename... Types> void Serial(T&& Val, Types&&... Vals)
{
	Serialize(Val);
	Serial(Vals...);
}
void Serial();

struct VoidBytes:Serialazable
{
	UINT Size;
	VoidBytes(UINT Size):
		Size(Size)
	{}
	void Serialize();
};