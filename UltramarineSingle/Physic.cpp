#include "Physic.h"
#include "Graphic.h"

PhysicUnit::PhysicUnit(Vector ASpeed, Vector AImpulse, Vector AForce, double AMass)
{
	Speed=ASpeed;
	Impulse=AImpulse;
	Force=AForce;
	Mass=AMass;
	Turn=0;
}
void PhysicUnit::ApplyImpulse(TwinVector AImpulse)
{
	Impulse+=AImpulse.vct2*0.05*Mass;
	Turn+=(AImpulse.vct1.x*AImpulse.vct2.y-AImpulse.vct1.y*AImpulse.vct2.x)*0.01;
	/*//dbg
	Line(x+AImpulse.vct1.x, y+AImpulse.vct1.y, x+AImpulse.vct1.x+AImpulse.vct2.x, y+AImpulse.vct1.y+AImpulse.vct2.y, 1, 1, 1, 0);*/
}
void PhysicUnit::ApplyForce(TwinVector AForce)
{
	Impulse+=AForce.vct2;
}
void PhysicUnit::Calc()
{
	Speed=Impulse/Mass;
	x+=Speed.x;
	y+=Speed.y;
	angle+=Turn;
	Impulse=v(0, 0);
	Turn=0;
}
//DefineFuncForEach (PhysicUnit, Calc)