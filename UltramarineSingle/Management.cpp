#include "Management.h"

std::list<BaseUnit*> BaseUnit::List;

void BaseUnit::ActivateAll()
{
	for (BaseUnit* Current : List)
	{
		Current->Active = true;
	}
}
BaseUnit::BaseUnit()
{
	NeedDelete = false;
	Active = false;
	List.push_back(this);
	Iterator = std::prev(List.end());
}
void BaseUnit::Delete()
{
	NeedDelete = true;
}
void BaseUnit::DeleteAllThatNeedDelete()
{
	for (auto i = List.begin(); i != List.end();)
	{
		if ((*i)->NeedDelete)
		{
			auto curr = i;
			++i;
			delete *curr;
		}
		else ++i;
	}
}
BaseUnit::~BaseUnit()
{
	List.erase(Iterator);
}

//DefineFuncForEach(SteepProcedUnit, SteepProc)

Syncronizer::Syncronizer()
{
	Time = std::chrono::high_resolution_clock::now();
}
void Syncronizer::Sync(std::chrono::nanoseconds TimeInterval)
{
	using namespace std::chrono_literals;
	auto EndTime = Time + TimeInterval;
	while (std::chrono::high_resolution_clock::now() < EndTime)
	{
		//std::this_thread::sleep_for(100us);
		volatile int t = 42;
		for (int i = 0; i < 10000; i++)
			sin(t);
	}
	Time = std::chrono::high_resolution_clock::now();
}