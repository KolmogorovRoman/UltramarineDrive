#include "Serialization.h"

thread_local BytesArray* CurrentArray;
thread_local SerializingStage CurrentStage;

const WORD BytesOrderConst = 0x5544;

BytesArray::BytesArray()
{
	Reverse = !IsBigEndian();
}
BytesArray::BytesArray(BytesArray& Other) :
	Size(Other.Size),
	Array(Other.Array),
	Pointer(Other.Pointer),
	Reverse(Other.Reverse)
{}
BytesArray* BytesArray::New(UINT Size)
{
	BytesArray* Array = new BytesArray;
	Array->Size = Size;
	Array->Array = new BYTE[Array->Size];
	return Array;
}
BytesArray* BytesArray::New(UINT Size, void* Pointer)
{
	BytesArray* Array = new BytesArray;
	Array->Size = Size;
	Array->Array = new BYTE[Array->Size];
	memcpy(Array->Array, Pointer, Size);
	return Array;
}
void BytesArray::Recreate(UINT Size)
{
	this->Size = Size;
	delete[] Array;
	Array = new BYTE[Size];
}
void BytesArray::Recreate(void* Pointer)
{
	delete[] Array;
	Array = new BYTE[Size];
	memcpy(Array, Pointer, Size);
}
void BytesArray::Recreate(UINT Size, void* Pointer)
{
	this->Size = Size;
	delete[] Array;
	Array = new BYTE[Size];
	memcpy(Array, Pointer, Size);
}
void BytesArray::ResetPointer()
{
	Pointer = 0;
}

void MovePointer(UINT Size)
{
	CurrentArray->Pointer += Size;
}
void IncSize(UINT Size)
{
	CurrentArray->Size += Size;
}
void WriteBytes(const void* Source, UINT Size)
{
	if (!CurrentArray->Reverse)
	{
		memcpy(&(CurrentArray->Array[CurrentArray->Pointer]), Source, Size);
	}
	else
		for (int i = 0, j = Size - 1; i < Size; i++, j--)
			CurrentArray->Array[CurrentArray->Pointer + i] = *(((BYTE*) Source) + j);
	MovePointer(Size);
}
void ReadBytes(void* Dest, UINT Size)
{
	if (!CurrentArray->Reverse)
		memcpy(Dest, &CurrentArray->Array[CurrentArray->Pointer], Size);
	else
		for (int i = 0, j = Size - 1; i < Size; i++, j--)
			*(((BYTE*) Dest) + j) = CurrentArray->Array[CurrentArray->Pointer + i];
	MovePointer(Size);
}

#define DEF_SERIALIZE_FOR_STRING(T) \
void Serialize(T*& str) \
{ \
	if (SizeCalculation) IncSize(sizeof(strlen(str)) + strlen(str) + 1); \
	if (Serialazing) \
	{ \
		UINT Len = strlen(str) + 1; \
		WriteBytes(&Len, sizeof (Len)); \
		WriteBytes(str, Len); \
	} \
	if (Deserialazing) \
	{ \
		UINT Len; \
		ReadBytes(&Len, sizeof Len); \
		str = new T[Len]; \
		ReadBytes(str, Len); \
	} \
}
DEF_SERIALIZE_FOR_STRING(char)
void Serialize(string& Val)
{
	if (SizeCalculation) IncSize(sizeof(Val.size()) + Val.size() * sizeof(char));
	if (Serialazing)
	{
		UINT Len = Val.size() * sizeof(char);
		WriteBytes(&Len, sizeof(Len));
		WriteBytes(Val.c_str(), Len);
	}
	if (Deserialazing)
	{
		UINT Len;
		ReadBytes(&Len, sizeof Len * sizeof(char));
		char* str = new char[Len];
		ReadBytes(str, Len * sizeof(char));
		Val.assign(str, Len);
	}
}

void Serial() {}

void VoidBytes::Serialize()
{
	if (SizeCalculation)
	{
		IncSize(Size);
	}
	if (Serialazing || Deserialazing)
	{
		MovePointer(Size);
	}
}

bool IsLittleEndian()
{
	return *((BYTE*) &(BytesOrderConst)) == 0x44;
}
bool IsBigEndian()
{
	return *((BYTE*) &(BytesOrderConst)) == 0x55;
}