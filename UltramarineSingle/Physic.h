#pragma once
#include "Misc.h"
#include "Management.h"

class PhysicUnit;

//DeclarateFuncForEach (PhysicUnit, Calc)

class PhysicUnit:
	public virtual PointUnit,
	public ManagedUnit<PhysicUnit>
{
public:
	Vector Speed, Impulse, Force;
	double Mass, Inercia;
	Vector Moving;
	double Turn;
	PhysicUnit(Vector=v(0, 0), Vector=v(0, 0), Vector=v(0, 0), double=1);
	void ApplyImpulse(TwinVector AImpulse);
	void ApplyForce(TwinVector AForce);
	void Calc();
};