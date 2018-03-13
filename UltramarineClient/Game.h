#pragma once
class Tank;
class Map;
class Bullet;
class Hit;
class Box;
class Plane;
class Explosive;
class TankController;

#include "Drive.h"
#include "resource.h"
using namespace std;

extern Controller Contr1;
extern HINSTANCE hInst;
extern RECT RectInScreen;

void GameInit();

class Bullet:
	public ManagedUnit<Bullet>,
	public SolidUnit,
	public GraphicUnit,
	public SteepProcedUnit,
	public RecvUnit<Bullet>
{
public:
	Bullet();
	void SteepProc() override;
	void CollProc(SolidUnit* Other) override;
	void Recved() override;
	~Bullet();
};
void Serialize(Bullet& Bullet);
class Tank:
	public ManagedUnit<Tank>,
	public GraphicUnit,
	//public SolidPhysicUnit,
	public SteepProcedUnit,
	public RecvUnit<Tank>,
	public SolidUnit
{
public:
	GraphicUnit* Tower;
	int HitPoints;
	int FireReady;
	LPSTR NickName;
	int NickLen;
	BYTE ClientID;
	double TowerAngle;
	Tank();
	void DrawProc() override;
	~Tank();
};
void Serialize(Tank& Tank);
class Map:
	public ManagedUnit<Map>,
	public GraphicUnit,
	public SolidUnit
{
public:
	Map(Image* image, LPCTSTR Mask, LPCTSTR AIPoints);

	int PointsCount;
	POINT* TargetPoints;
	int* NearsCounts;
	int** Net;
	int*** Pathes;
	int** PathesLen;
	int** PathesDist;
};
class Box:
	public ManagedUnit<Box>,
	public GraphicUnit,
	public SolidPhysicUnit
{
public:
	Box(int x, int y);
};
class Plane:
	public GraphicUnit,
	public ManagedUnit<Plane>,
	public SteepProcedUnit
{
public:
	int Time;
	double TurnSpeed;
	Plane(int x, int y);
	void SteepProc();
};
class Explosive:
	public ManagedUnit<Explosive>,
	public SolidUnit,
	public SteepProcedUnit
{
public:
	Explosive(int x, int y);
	void SteepProc();
};
class TankController:
	public SendUnit<TankController>,
	public SteepProcedUnit
{
public:
	int KeyForwardCode;
	int KeyBackCode;
	int KeyRightCode;
	int KeyLeftCode;
	int KeyFireCode;
	int KeyPlaneCode;
	bool KeyForwardPressed;
	bool KeyBackPressed;
	bool KeyRightPressed;
	bool KeyLeftPressed;
	bool KeyFirePressed;
	bool KeyPlanePressed;
	POINT Mouse;
	TankController(int KeyForwardCode, int KeyBackCode, int KeyRightCode, int KeyLeftCode, int KeyFireCode, int KeyPlaneCode);
	void SteepProc() override;
};
void Serialize(TankController& TankController);