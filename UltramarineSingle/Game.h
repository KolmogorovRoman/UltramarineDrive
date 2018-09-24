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
class TankController;
class Apache;
class Atgm;

#include "Drive.h"
#include "Chat.h"
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
	TankController* Contr;
	GraphicUnit* Tower;
	Player* player;
	int UserID;
	int HitPoints;
	int FireReady;
	string NickName;
	int NickLen;
	double TowerAngle;
	double TowerSpeed;
	Tank(PointUnit Point, Player* player, TankController* Contr, string NickName);
	void SteepProc() override;
	void CollProc(SolidUnit* Other) override;
	void DrawProc() override;
	void Die();
	ContMask* contmask;
	~Tank();
};
stringstream& operator >> (stringstream& stream, Tank*& tank);
class Map:
	public ManagedUnit<Map>,
	public GraphicUnit,
	public SolidUnit
{
public:
	Map(string ResourcesDir);

	struct Point:
		public PointUnit
	{
		std::list<Point*> Nears;
	};
	std::list<PointUnit> Points;
	int PointsCount;
	PointUnit* TargetPoints;
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
	string NickName;
	int Team;
	TankController* Contr;
	Tank* tank;
	int SpawnPointIndex = -1;
	bool PlaneReady;
	Player(string NickName, TankController* Contr);
	void Spawn();
	virtual void SpawnProc();
	virtual PointUnit SelectSpawnPoint();
	int WinsCount;
	int KillsCount;
	int DeathsCount;
};
stringstream& operator >> (stringstream& stream, Player*& tank);
class Bot:
	public ManagedUnit<Bot>,
	public Player,
	public SteepProcedUnit
{
public:
	Bot(string NickName);
	void SteepProc() override;
	void SpawnProc() override;
	PointUnit SelectSpawnPoint() override;
private:
	bool Moving;
	PointUnit TargetPoint;
	Tank* TargetTank;
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
	void CollProc(SolidUnit* Other) override;
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
class TankController
{
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
};
class Apache:
	public ManagedUnit<Apache>,
	public GraphicUnit,
	public SteepProcedUnit
{
public:
	GraphicUnit* Helix;
	double HelixVelosity = 0;
	Controller* Contr;
	double ForwardSpeed = 1.8;
	double Velosity = 1.5;
	Player* Target = NULL;
	double a = 90;
	double b = 90;
	int FireReady = 0;
	Apache(PointUnit Point, Controller* Contr);
	void SteepProc() override;
	void DrawProc() override;
	~Apache();
};
class Atgm:
	public ManagedUnit<Apache>,
	public GraphicUnit,
	public SteepProcedUnit
{
public:
	int Time = 0;
	Player* Target;
	double Speed = 2;
	double RotSpeed = 2;
	Atgm(Player* Target, double x, double y, double angle);
	void SteepProc() override;
};