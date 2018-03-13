#pragma once

class SolidPhysicUnit;

#include "Physic.h"
#include "Solid.h"

extern bool PhysCollTable[4][4];

TwinVector ImpactCalc(Mask* Mask1, Mask* Mask2, int x1, int y1, int x2, int y2);
void CalcAllSolidPhysicUnits();

class SolidPhysicUnit:
	public SolidUnit,
	public PhysicUnit,
	public ManagedUnit<SolidPhysicUnit>
{
public:
	SolidPhysicUnit(Mask* mask, int, Vector=v(0, 0), Vector=v(0, 0), Vector=v(0, 0), double=1);
	void CollCalc(SolidUnit* Other);
};