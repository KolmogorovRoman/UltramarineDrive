#pragma once
class Player;
class Tank;
class Map;
class Bullet;
class Hit;
class Box;
class Plane;
class Explosive;
class TankController;

#include "Drive.h"
using namespace std;

extern Controller Contr1;
extern HINSTANCE hInst;
extern RECT RectInScreen;
extern Console* GameConsole;

void GameInit();

class Bullet:
	public ManagedUnit<Bullet>,
	public SolidUnit,
	public GraphicUnit,
	public SteepProcedUnit,
	public RecvUnit<Bullet>,
	Serialazable
{
public:
	Bullet();
	void SteepProc() override;
	void CollProc(SolidUnit* Other) override;
	void Recved() override;
	void Serialize();
	~Bullet();
};
class Player:
	public ManagedUnit<Player>,
	public RecvUnit<Player>,
	public Serialazable
{
public:
	string NickName;
	Tank* tank=NULL;
	bool PlaneReady;
	int KillsCount;
	int DeathsCount;
	int WinsCount;
	Player();
	void Serialize();
};
class Tank:
	public ManagedUnit<Tank>,
	public GraphicUnit,
	//public SolidPhysicUnit,
	public SteepProcedUnit,
	public RecvUnit<Tank>,
	public SolidUnit,
	Serialazable
{
public:
	TankController* Contr;
	GraphicUnit* Tower;
	Player* player;
	bool PlaneReady;
	BYTE HitPoints;
	BYTE FireReady;
	string NickName;
	int NickLen;
	double TowerAngle;
	Tank();
	void DrawProc() override;
	void Serialize();
	~Tank();
};
class Map:
	public ManagedUnit<Map>,
	public GraphicUnit,
	public SolidUnit
{
public:
	Map(string ResourcesDir);

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
	public SolidPhysicUnit,
	public RecvUnit<Box>,
	public Serialazable
{
public:
	Box();
	Box(int x, int y);
	void Serialize();
	void Recved() override;
};
class Plane:
	public GraphicUnit,
	public ManagedUnit<Plane>,
	public SteepProcedUnit,
	public RecvUnit<Plane>,
	public Serialazable
{
public:
	int Time;
	double TurnSpeed;
	Plane();
	Plane(int x, int y);
	void SteepProc();
	void Serialize();
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
	public SteepProcedUnit,
	Serialazable
{
public:
	int KeyForwardCode;
	int KeyBackCode;
	int KeyRightCode;
	int KeyLeftCode;
	int KeyFireCode;
	int KeyPlaneCode;
	Controller* Contr;
public:
	bool KeyForwardPressed;
	bool KeyBackPressed;
	bool KeyRightPressed;
	bool KeyLeftPressed;
	bool KeyFirePressed;
	bool KeyPlanePressed;
	POINT Mouse;
	TankController();
	TankController(Controller* Contr, int KeyForwardCode, int KeyBackCode, int KeyRightCode, int KeyLeftCode, int KeyFireCode, int KeyPlaneCode);
	void Check();
	void Serialize();
};