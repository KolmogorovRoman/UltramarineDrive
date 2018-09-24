#pragma once
class Tank;
class Map;
class Bullet;
class Hit;
class Box;
class Player;
class Bot;
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
void DeleteClient(Server::Client* Client);

class Bullet:
	public ManagedUnit<Bullet>,
	public SolidUnit,
	public GraphicUnit,
	public SteepProcedUnit,
	public SendUnit<Bullet>,
	Serialazable
{
public:
	Player* ParPlayer;
	Bullet(double x, double y, double angle, Player* ParPlayer);
	void SteepProc();
	void CollProc(SolidUnit* Other);
	void Serialize();
	~Bullet();
};
class Tank:
	public ManagedUnit<Tank>,
	public GraphicUnit,
	public SolidPhysicUnit,
	public SteepProcedUnit,
	public SendUnit<Tank>,
	Serialazable
{
public:
	TankController* Contr;
	GraphicUnit* Tower;
	Player* player;
	int UserID;
	BYTE HitPoints;
	BYTE FireReady;
	string NickName;
	int NickLen;
	double TowerAngle;
	Tank(PointUnit Point, Player* player, TankController* Contr, string NickName);
	void SteepProc();
	void CollProc(SolidUnit* Other);
	void DrawProc();
	void Die();
	ContMask* contmask;
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
	int ChunksWidth, ChunksHeight;
	bool* ChunksFree;
	bool ChunkFree(int x, int y);

	int PointsCount;
	PointUnit* TargetPoints;
	int* NearsCounts;
	int** Net;
	int*** Pathes;
	int** PathesLen;
	int** PathesDist;
};
class Player:
	public ManagedUnit<Player>,
	public SendUnit<Player>,
	public virtual SteepProcedUnit,
	public Serialazable
{
public:
	string NickName;
	TankController* Contr;
	Tank* tank;
	Server::Client* Client;
	int SpawnPointIndex = -1;
	bool PlaneReady;
	Player(string NickName, TankController* Contr);
	void Spawn();
	virtual void SpawnProc();
	virtual PointUnit SelectSpawnPoint();
	void SteepProc() override;
	void Serialize();
	~Player();
	int WinsCount;
	int KillsCount;
	int DeathsCount;
};
class Bot:
	public ManagedUnit<Bot>,
	public Player,
	public virtual SteepProcedUnit
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
	public SolidPhysicUnit,
	public SendUnit<Box>,
	public Serialazable
{
public:
	Box(int x, int y);
	void Serialize();
};
class Plane:
	public GraphicUnit,
	public ManagedUnit<Plane>,
	public SteepProcedUnit,
	public SendUnit<Plane>,
	public Serialazable
{
public:
	int Time;
	double TurnSpeed;
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
	SendUnit<TankController>,
	Serialazable
{
public:
	bool KeyForwardPressed;
	bool KeyBackPressed;
	bool KeyRightPressed;
	bool KeyLeftPressed;
	bool KeyFirePressed;
	bool KeyPlanePressed;
	POINT Mouse;
	void Serialize();
};