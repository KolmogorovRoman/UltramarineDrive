#pragma once
#include <cstring>

typedef unsigned int UINT;

template <typename T, UINT GroupsCount, UINT GroupSize> class IDManager
{
	struct IDsGroup
	{
		T** Array;
		UINT IDsCount;
		IDsGroup()
		{
			Array = new T*[GroupSize];
			memset(Array, 0, GroupSize * (sizeof(T*)));
			IDsCount = 0;
		}
		~IDsGroup()
		{
			delete[] Array;
		}
	};
	IDsGroup** Array;
public:
	IDManager()
	{
		Array = new IDsGroup*[GroupsCount];
		memset(Array, 0, GroupsCount * (sizeof(T*)));
	}
	UINT Alloc(T* Pointer)
	{
		for (int Group = 0; Group < GroupsCount; Group++)
		{
			if (Array[Group] == NULL)
			{
				Array[Group] = new IDsGroup();
				Array[Group]->Array[0] = Pointer;
				Array[Group]->IDsCount++;
				return Group * GroupSize;
			}
			if (Array[Group]->IDsCount < GroupSize)
				for (int ID = 0; ID < GroupSize; ID++)
					if (Array[Group]->Array[ID] == 0)
					{
						Array[Group]->Array[ID] = Pointer;
						Array[Group]->IDsCount++;
						return Group * GroupSize + ID;
					}
		}
	}
	void Place(UINT ID, T* Pointer)
	{
		if (Array[ID / GroupSize] == NULL)
		{
			Array[ID / GroupSize] = new IDsGroup();
			Array[ID / GroupSize]->Array[ID % GroupSize] = Pointer;
			Array[ID / GroupSize]->IDsCount++;
		}
		else
		{
			if (Array[ID / GroupSize]->Array[ID % GroupSize] == NULL) Array[ID / GroupSize]->IDsCount++;
			Array[ID / GroupSize]->Array[ID % GroupSize] = Pointer;
		}
	}
	T* Free(UINT ID)
	{
		if (Array[ID / GroupSize] == NULL)
			return NULL;
		if (Array[ID / GroupSize]->Array[ID % GroupSize] != NULL)
			Array[ID / GroupSize]->IDsCount--;
		T* Ret = Array[ID / GroupSize]->Array[ID % GroupSize];
		if (Array[ID / GroupSize]->IDsCount == 0)
		{
			delete Array[ID / GroupSize];
			Array[ID / GroupSize] = NULL;
		}
		else
		{
			Ret = Array[ID / GroupSize]->Array[ID % GroupSize];
			Array[ID / GroupSize]->Array[ID % GroupSize] = NULL;
		}
		return Ret;
	}
	T* Get(UINT ID)
	{
		if (Array[ID / GroupSize] == NULL) return NULL;
		else return Array[ID / GroupSize]->Array[ID % GroupSize];
	}
};