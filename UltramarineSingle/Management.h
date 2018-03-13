#pragma once
#include <time.h>
#include <Windows.h>
#include <iostream>
#include "IDManagement.h"
template <class T> class ManagedUnit;
class BaseUnit;
class SteepProcedUnit;
template <class T> class ListData;
template <class T> class List;

#define ForList(List, Iterator) for (auto IteratorVar=List.First->Next; IteratorVar!=List.Last; IteratorVar=IteratorVar->Next) for (auto Iterator=IteratorVar->Data; Iterator!=NULL; Iterator=NULL)
#define CallList(List) ForList(List, Iterator) Iterator->

#define ForEach(Class, Iterator) for (Class* Iterator=(Class*)ManagedUnit<Class>::First->Next; Iterator!=(Class*)ManagedUnit<Class>::Last; Iterator=(Class*)Iterator->ManagedUnit<Class>::Next) if (Iterator->Active)
#define CallEach(Class) ForEach(Class, Iterator) Iterator->

class BaseUnit
{
public:
	bool Active;
	bool NeedDelete;
	static BaseUnit* StaticConstructor();
	BaseUnit* Next;
	BaseUnit* Prev;
	static BaseUnit* First;
	static BaseUnit* Last;
	static IDManager<BaseUnit, 65536, 65536> Manager;
	UINT ID;
	BaseUnit();
	static void ActivateAll();
	static void DeleteAllThatNeedDelete();
	void Delete();
	virtual ~BaseUnit();
};
BaseUnit* UnitID(UINT ID);

template <class T> class ManagedUnit: virtual public BaseUnit
{
public:
	ManagedUnit<T>* Next;
	ManagedUnit<T>* Prev;
	static ManagedUnit<T>* First;
	static ManagedUnit<T>* Last;
	static ManagedUnit<T>* StaticConstructor();

	ManagedUnit();
	static bool IsEmpty();
	virtual ~ManagedUnit();
};
template <class T> ManagedUnit<T>* ManagedUnit<T>::First;
template <class T> ManagedUnit<T>* ManagedUnit<T>::Last = ManagedUnit<T>::StaticConstructor();
template <class T> ManagedUnit<T>* ManagedUnit<T>::StaticConstructor()
{
	ManagedUnit<T>::First = (ManagedUnit<T>*) malloc(sizeof ManagedUnit<T>);
	ManagedUnit<T>::Last = (ManagedUnit<T>*) malloc(sizeof ManagedUnit<T>);
	ManagedUnit<T>::First->Next = ManagedUnit<T>::Last;
	ManagedUnit<T>::Last->Prev = ManagedUnit<T>::First;
	ManagedUnit<T>::First->Prev = NULL;
	ManagedUnit<T>::Last->Next = NULL;
	return ManagedUnit<T>::Last;
};
template <class T> ManagedUnit<T>::ManagedUnit()
{
	Active = false;
	Next = Last;
	Prev = Last->Prev;
	Prev->Next = this;
	Last->Prev = this;
}
template <class T> bool ManagedUnit<T>::IsEmpty()
{
	if (First->Next == Last) return true;
	else return false;
}
template <class T> ManagedUnit<T>::~ManagedUnit()
{
	Prev->Next = Next;
	Next->Prev = Prev;
}

class SteepProcedUnit:
	public ManagedUnit<SteepProcedUnit>
{
public:
	virtual void SteepProc() {};
};

template <class T> class ListData
{
public:
	ListData<T>* Next;
	ListData<T>* Prev;
	T* Data;

	ListData();
	ListData(T* Data);
	void Remove();
	~ListData();
};
template <class T> ListData<T>::ListData()
{
	Data = NULL;
	Prev = NULL;
	Next = NULL;
}
template <class T> ListData<T>::ListData(T* AData)
{
	Data = AData;
	Prev = NULL;
	Next = NULL;
}
template <class T> void ListData<T>::Remove()
{
	Next->Prev = Prev;
	Prev->Next = Next;
}
template <class T> ListData<T>::~ListData()
{
	delete Data;
}

template <class T> class List
{
public:
	ListData<T>* First;
	ListData<T>* Last;

	List();
	ListData<T>* Add(T* Data);
	void Remove(T* Data);
	bool IsEmpty();
	void Clear();
	~List();
};
template <class T> List<T>::List()
{
	First = new ListData<T>();
	Last = new ListData<T>();
	First->Next = Last;
	Last->Prev = First;
	First->Prev = NULL;
	Last->Next = NULL;
}
template <class T> ListData<T>* List<T>::Add(T* Data)
{
	ListData<T>* Added = new ListData<T>(Data);
	Added->Next = Last;
	Added->Prev = Last->Prev;
	Added->Prev->Next = Added;
	Last->Prev = Added;
	return Added;
}
template <class T> void List<T>::Remove(T* Data)
{
	ForList(this, ListData)
	{
		if (ListData == Data)
		{
			ListData->Prev->Next = ListData->Next;
			ListData->Next->Prev = ListData->Prev;
		}
	}
}
template <class T> bool List<T>::IsEmpty()
{
	if (First->Next == Last) return true;
	else return false;
}
template <class T> void List<T>::Clear()
{
	ListData<T>* Finded = First->Next;
	ListData<T>* Next;
	while (Finded != Last)
	{
		Next = Finded->Next;
		delete Finded;
		Finded = Next;
	}
	First->Next = Last;
	Last->Prev = First;
}
template <class T> List<T>::~List()
{
	Clear();
	delete First;
	delete Last;
}

class Syncronizer
{
public:
	int Time;
	unsigned int SleepTime;
	Syncronizer();
	void Sync(int TimeInterval);
};