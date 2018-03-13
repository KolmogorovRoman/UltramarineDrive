#pragma once
class Tank;
class Map;
class Bullet;
class Bot;
class Hit;
class Box;
class Player;
class Plane;
class Explosive;

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
	public SteepProcedUnit
{
public:
	Player* ParPlayer;
	Bullet(double x, double y, double angle, Player* ParPlayer);
	void SteepProc();
	void CollProc(SolidUnit* Other);
	~Bullet();
};
class Tank:
	public ManagedUnit<Tank>,
	public GraphicUnit,
	public SolidPhysicUnit,
	public SteepProcedUnit
{
public:
	Controller* Contr;
	GraphicUnit* Tower;
	Player* player;
	int UserID;
	int KeyForward;
	int KeyBack;
	int KeyRight;
	int KeyLeft;
	int KeyFire;
	int HitPoints;
	int FireReady;
	LPSTR NickName;
	int NickLen;
	double TowerAngle;
	Tank(double x, double y, double angle, Player* player, Controller* Cont, LPSTR NickName, int KeyForward, int KeyBack, int KeyRight, int KeyLeft, int KeyFire);
	void SteepProc();
	void CollProc(SolidUnit* Other);
	void DrawProc();
	ContMask* contmask;
	~Tank();
};
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
class Player:
	public ManagedUnit<Player>
{
public:
	LPSTR NickName;
	PointUnit SpawnPoint;
	Controller* Contr;
	Tank* tank;
	bool PlaneReady;
	Player(LPSTR NickName, Controller* Contr, double x, double y, double angle);
	void Spawn();
	virtual void SpawnProc();
	int KillsCount;
	int DeathsCount;
};
class Bot:
	public ManagedUnit<Bot>,
	public Player,
	public SteepProcedUnit
{
public:
	Bot(LPSTR NickName, double x, double y, double angle);
	void SteepProc();
	void SpawnProc();
private:
	bool Moving;
	POINT TargetPoint;
	Tank* TargetTank;
	int** Net;
	int PointsCount;
	POINT* TargetPoints;
	int* NearsCounts;
	int CurrentTargetPoint;
	int NearPoint;
	int PrevPoint;
	int* Path;
	int PathEnd;
	int PathLen;
	int PathPoint;
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