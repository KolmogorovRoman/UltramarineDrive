#include "Serialization.h"

thread_local BytesArray* CurrentArray;
thread_local SerializingStage CurrentStage;

const WORD BytesOrderConst = 0x5544;

BytesArray::BytesArray()
{}
BytesArray::BytesArray(BytesArray& Other):
	Size(Other.Size),
	Array(Other.Array),
	Pointer(Other.Pointer)
{}
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

BytesArray* NewArray(UINT Size)
{
	BytesArray* Array = new BytesArray;
	Array->Size = Size;
	Array->Array = new BYTE[Array->Size];
	return Array;
}
BytesArray* NewArray(UINT Size, void* Pointer)
{
	BytesArray* Array = new BytesArray;
	Array->Size = Size;
	Array->Array = new BYTE[Array->Size];
	return Array;
	memcpy(Array->Array, Pointer, Size);
}

void MovePointer(UINT Size)
{
	CurrentArray->Pointer += Size;
}
void IncSize(UINT Size)
{
	CurrentArray->Size += Size;
}
void AddBytes(void* Source, UINT Size)
{
	//if (*((BYTE*) &(BytesOrderConst)) == 0x55)
	{
		memcpy(&(CurrentArray->Array[CurrentArray->Pointer]), Source, Size);
	}
	/*else
		for (int i = 0, j = Size - 1; i < Size; i++, j--)
			CurrentArray->Array[CurrentArray->Pointer + i] = *(((BYTE*) Source) + j);*/
	MovePointer(Size);
}
void ReadBytes(void* Dest, UINT Size)
{
	//if (*((BYTE*) &(BytesOrderConst)) == 0x55)
		memcpy(Dest, &CurrentArray->Array[CurrentArray->Pointer], Size);
	/*else
		for (int i = 0, j = Size - 1; i < Size; i++, j--)
			*(((BYTE*) Dest) + j) = CurrentArray->Array[CurrentArray->Pointer + i];*/
	MovePointer(Size);
}

template <> void Serialize(char*& str)
{
	if (SizeCalculation)
	{
		IncSize(sizeof(strlen(str)) + strlen(str) + 1);
	}
	if (CopyingToArray)
	{
		UINT Len = strlen(str) + 1;
		AddBytes(&Len, sizeof Len);
		AddBytes(str, Len);
	}
	if (CopyingFromArray)
	{
		UINT Len;
		ReadBytes(&Len, sizeof Len);
		str = new char[Len];
		ReadBytes(str, Len);
	}
}
void Serialize(VoidBytes& Val)
{
	if (SizeCalculation)
	{
		IncSize(Val.Size);
	}
	if (CopyingToArray || CopyingFromArray)
	{
		MovePointer(Val.Size);
	}
}