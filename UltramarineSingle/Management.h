#pragma once
#include <Windows.h>
#include <iostream>
#include <list>
#include <thread>
template <class T> class ManagedUnit;
class BaseUnit;
class SteepProcedUnit;
template <class T> class ListData;

#define CallList(List) for(auto Iterator : List) Iterator->

#define ForEach(Class, Iterator) for (auto ManagedUnitIterator : ManagedUnit<Class>::List) for(Class* Iterator = (Class*) ManagedUnitIterator; Iterator!=NULL; Iterator=NULL) if (Iterator->Active)
#define CallEach(Class) ForEach(Class, Iterator) Iterator->

class BaseUnit
{
public:
	bool Active;
	bool NeedDelete;
	static std::list<BaseUnit*> List;
	std::list<BaseUnit*>::iterator Iterator;
	BaseUnit();
	static void ActivateAll();
	static void DeleteAllThatNeedDelete();
	void Delete();
	virtual ~BaseUnit();
};

template <class T> class ManagedUnit: virtual public BaseUnit
{
public:
	static std::list<ManagedUnit<T>*> List;
	std::_List_iterator<std::_List_val<std::_List_simple_types<ManagedUnit<T>*>>> Iterator;

	ManagedUnit();
	static bool IsEmpty();
	virtual ~ManagedUnit();
};
template <class T> std::list<ManagedUnit<T>*> ManagedUnit<T>::List;
template <class T> ManagedUnit<T>::ManagedUnit()
{
	List.push_back(this);
	Iterator = std::prev(List.end());
}
template <class T> bool ManagedUnit<T>::IsEmpty()
{
	return List.empty();
}
template <class T> ManagedUnit<T>::~ManagedUnit()
{
	List.erase(Iterator);
}

class SteepProcedUnit:
	public ManagedUnit<SteepProcedUnit>
{
public:
	virtual void SteepProc() {};
};

class Syncronizer
{
public:
	std::chrono::time_point<std::chrono::high_resolution_clock> Time;
	std::chrono::nanoseconds SleepTime;
	Syncronizer();
	void Sync(std::chrono::nanoseconds TimeInterval);
};