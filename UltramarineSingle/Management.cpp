#include "Management.h"

BaseUnit* BaseUnit::First;
BaseUnit* BaseUnit::Last = StaticConstructor();
IDManager<BaseUnit, 65536, 65536> BaseUnit::Manager;
BaseUnit* BaseUnit::StaticConstructor()
{
	First = (BaseUnit*) malloc(sizeof BaseUnit);
	Last = (BaseUnit*) malloc(sizeof BaseUnit);
	First->Next = Last;
	First->Prev = NULL;
	Last->Prev = First;
	Last->Next = NULL;
	return Last;
}
void BaseUnit::ActivateAll()
{
	for (BaseUnit* Current = First->Next; Current != Last; Current = Current->Next)
	{
		Current->Active = true;
	}
}
BaseUnit::BaseUnit()
{
	NeedDelete = false;
	Active = false;
	Next = Last;
	Prev = Last->Prev;
	Prev->Next = this;
	Last->Prev = this;

	ID = Manager.Alloc(this);
}
void BaseUnit::Delete()
{
	NeedDelete = true;
}
void BaseUnit::DeleteAllThatNeedDelete()
{
	BaseUnit* CurrentNext;
	for (BaseUnit* Current = First->Next; Current != Last; Current = CurrentNext)
	{
		CurrentNext = Current->Next;
		if (Current->NeedDelete)
			delete Current;
	}
}
BaseUnit::~BaseUnit()
{
	Prev->Next = Next;
	Next->Prev = Prev;

	Manager.Free(ID);
}

BaseUnit* UnitID(UINT ID)
{
	return BaseUnit::Manager.Get(ID);
}

//DefineFuncForEach(SteepProcedUnit, SteepProc)

Syncronizer::Syncronizer()
{
	Time = clock();
}
void Syncronizer::Sync(int TimeInterval)
{
	SleepTime = max(0, TimeInterval - (clock() - Time));
	//std::cout << SleepTime<<std::endl;
	Sleep(SleepTime);
	Time = clock();
}