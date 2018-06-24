#include "Game.h"
#include <fstream>
#include <tuple>

HINSTANCE hInst;
RECT RectInScreen;
Console* GameConsole;

Map* Map1;
Image* TankBodyImage;
Image* TankTowerImage;
Image* BulletImage;
Image* Fire1Image;
Image* Fire2Image;
Image* Fire3Image;
AnimatedImage* FireImage;
AnimatedImage* HitImage;
Image* HealImage;
Image* FirTreeImage;
Image* PlaneImage;
AnimatedImage* ExplosiveImage;
Image* SightImage;
AnimatedImage* PointerImage;

Layer* MapLayer;
Layer* TanksBodyLayer;
Layer* EffectsLayer;
Layer* TanksTowerLayer;
Layer* LabelsLayer;
Layer* SkyLayer;

Mask* TankMask;
Mask* BulletMask;
Mask* HealMask;
Mask* ExplosiveMask;

Player* Player1;
Player* WathedPlayer;

int GameMode = 0;
int CamMode = 0;
int DbgPointsMode = 0;
int DbgClockMode = 0;
int Time = 0;

bool CollTable[5][5] =
{               /*Tank*/ /*Bullet*/ /*Map*/ /*Heal*/ /*Explosive*/
	/*Tank*/       1,       1,      1,        1,         1,
	/*Bullet*/     1,       0,      1,        1,         0,
	/*Map*/        0,       0,      0,        0,         0,
	/*Heal*/       1,       1,      1,        1,         1,
	/*Explosive*/  0,       0,      0,        0,         0
};

bool PhysCollTable[4][4] =
{              /*Tank*/ /*Bullet*/ /*Map*/ /*Heal*/
	/*Tank*/      1,        0,        1,       1,
	/*Bullet*/    0,        0,        0,       0,
	/*Map*/       0,        0,        0,       0,
	/*Heal*/      1,        0,        1,       1
};

#define OT_TANK 0
#define OT_BULLET 1
#define OT_MAP 2
#define OT_HEAL 3
#define OT_EXPLOSIVE 4

Bullet::Bullet(double x, double y, double angle, Player* ParPlayer) :
	PointUnit(x, y, angle),
	SolidUnit(BulletMask, OT_BULLET),
	GraphicUnit(BulletImage, TanksBodyLayer),
	ParPlayer(ParPlayer)
{}
void Bullet::SteepProc()
{
	x += 15 * dcos(angle);
	y += 15 * dsin(angle);
	//angle += rand() % 7 - 3;
}
void Bullet::CollProc(SolidUnit* Other)
{
	if (ParPlayer->tank != Other)
	{
		Delete();
		(new AnimatedGraphicUnit(HitImage, EffectsLayer, false))->SetPoint(x, y, 180 + angle);
	}
}
Bullet::~Bullet()
{}

Tank::Tank(PointUnit Point, Player* player, TankController* Contr, string NickName) :
	PointUnit(Point),
	GraphicUnit(TankBodyImage, TanksBodyLayer),
	SolidPhysicUnit(TankMask, OT_TANK),
	player(player),
	Contr(Contr),
	NickName(NickName),
	HitPoints(5),
	FireReady(100),
	TowerAngle(0),
	TowerSpeed(0),
	contmask(new ContMask(mask)),
	Tower(new GraphicUnit(TankTowerImage, TanksTowerLayer))
{}
void Tank::SteepProc()
{
	Contr->Check();
	FireReady += 1;
	if (HitPoints > 5) HitPoints = 5;
	if (FireReady > 150) FireReady = 150;
	if (Contr->KeyFirePressed && FireReady >= 150)
	{
		new Bullet(x + dcos(angle + TowerAngle) * 60, y + dsin(angle + TowerAngle) * 60, angle + TowerAngle, player);
		(new AnimatedGraphicUnit(FireImage, EffectsLayer, false))
			->SetPoint(x + dcos(angle + TowerAngle) * 60, y + dsin(angle + TowerAngle) * 60, angle + TowerAngle);
		FireReady = 0;
	}
	if (Contr->KeyPlanePressed && player->PlaneReady == 1)
	{
		new Plane(Contr->Mouse.x, Contr->Mouse.y);
		player->PlaneReady = false;
	}
	double TargetAngle;
	TargetAngle = AngleToPoint(x, y, Contr->Mouse.x, Contr->Mouse.y);
	if (AnglesDiff(TargetAngle, angle + TowerAngle) < -2) TowerAngle -= 2;
	else if (AnglesDiff(TargetAngle, angle + TowerAngle) > 2) TowerAngle += 2;
	else TowerAngle = TargetAngle - angle;
	/*if (AnglesDiff(TargetAngle, angle + TowerAngle) < -2.5*TowerSpeed*TowerSpeed && TowerSpeed > -2) TowerSpeed -= 0.2;
	else if (AnglesDiff(TargetAngle, angle + TowerAngle) > 2.5*TowerSpeed*TowerSpeed && TowerSpeed < 2) TowerSpeed += 0.2;
	else if (TowerSpeed >= 0.2) TowerSpeed -= 0.2;
	else if (TowerSpeed <= -0.2) TowerSpeed += 0.2;
	else TowerSpeed = 0;
	TowerAngle += TowerSpeed;*/

	BYTE Mov = 0;
	if (Contr->KeyForwardPressed) Mov |= 0b10000000;
	if (Contr->KeyLeftPressed)    Mov |= 0b01000000;
	if (Contr->KeyRightPressed)   Mov |= 0b00100000;
	if (Contr->KeyBackPressed)    Mov |= 0b00010000;
	if ((Mov >> 4) == 0b0000) Mov = 0b0000;
	if ((Mov >> 4) == 0b0001) Mov = 0b0011;
	if ((Mov >> 4) == 0b0010) Mov = 0b1001;
	if ((Mov >> 4) == 0b0011) Mov = 0b0001;
	if ((Mov >> 4) == 0b0100) Mov = 0b0110;
	if ((Mov >> 4) == 0b0101) Mov = 0b0010;
	if ((Mov >> 4) == 0b0110) Mov = 0b0000;
	if ((Mov >> 4) == 0b0111) Mov = 0b0011;
	if ((Mov >> 4) == 0b1000) Mov = 0b1100;
	if ((Mov >> 4) == 0b1001) Mov = 0b1111;
	if ((Mov >> 4) == 0b1010) Mov = 0b1000;
	if ((Mov >> 4) == 0b1011) Mov = 0b1000;
	if ((Mov >> 4) == 0b1100) Mov = 0b0100;
	if ((Mov >> 4) == 0b1101) Mov = 0b0100;
	if ((Mov >> 4) == 0b1110) Mov = 0b1100;
	if ((Mov >> 4) == 0b1111) Mov = 0b1111;
	if (Mov == 0b1000) ApplyImpulse(TurnTV(tv(0, 4, 16, 0), angle));
	if (Mov == 0b0100) ApplyImpulse(TurnTV(tv(0, -4, 16, 0), angle));
	if (Mov == 0b0010) ApplyImpulse(TurnTV(tv(0, 4, -12, 0), angle));
	if (Mov == 0b0001) ApplyImpulse(TurnTV(tv(0, -4, -12, 0), angle));
	if (Mov & 0b1000) ApplyImpulse(TurnTV(tv(0, 6, 18, 0), angle));
	if (Mov & 0b0100) ApplyImpulse(TurnTV(tv(0, -6, 18, 0), angle));
	if (Mov & 0b0010) ApplyImpulse(TurnTV(tv(0, 6, -16, 0), angle));
	if (Mov & 0b0001) ApplyImpulse(TurnTV(tv(0, -6, -16, 0), angle));

	if (HitPoints <= 0)
	{
		Die();
	}

	delete contmask;
	contmask = new ContMask(mask);
}
void Tank::CollProc(SolidUnit* Other)
{
	if (Other->Type == OT_HEAL)
	{
		if (HitPoints < 5)
		{
			HitPoints += 2;
			Other->Delete();
		}
		else if (player->PlaneReady == 0)
		{
			player->PlaneReady = 1;
			Other->Delete();
		}
	}
	if (Other->Type == OT_BULLET && ((Bullet*)Other)->ParPlayer != player)
	{
		HitPoints -= 1;
	}
	if (Other->Type == OT_EXPLOSIVE)
		HitPoints -= 3;
	if (HitPoints <= 0)
	{
		if (Other->Type == OT_BULLET && ((Bullet*)Other)->ParPlayer != player)
		{
			((Bullet*)Other)->ParPlayer->KillsCount++;
		}
		if (Other->Type == OT_BULLET && player == WathedPlayer)
			WathedPlayer = ((Bullet*)Other)->ParPlayer;
		Die();
	}
}
void Tank::DrawProc()
{
	Tower->x = x;
	Tower->y = y;
	Tower->angle = angle + TowerAngle;
	DrawRect(x - 50, y - 50, x + 50, y - 45, 1, LabelsLayer, 1, 1, 1);
	DrawRect(x - 50, y - 50, x + 100 * HitPoints / 5 - 50, y - 45, true, LabelsLayer, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
	DrawRect(x - 50, y - 50, x + 50, y - 45, false, LabelsLayer);
	if (player == WathedPlayer)
	{
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 1, 1, 1);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2, 100 * FireReady / 150 - MainCamera.Width / 2, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 0.5, 0, 1);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, false, InterfaceLayer);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2 + 20, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 1, 1, 1);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2 + 20, 100 * HitPoints / 5 - MainCamera.Width / 2, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2 + 20, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, false, InterfaceLayer, 0, 0, 0);

		DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 20, Contr->Mouse.x + 15, Contr->Mouse.y - 15, true, LabelsLayer, 1, 1, 1);
		DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - 20, Contr->Mouse.x + 15, Contr->Mouse.y + 15, true, LabelsLayer, 1, 1, 1);
		DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y + 20, Contr->Mouse.x - 15, Contr->Mouse.y + 15, true, LabelsLayer, 1, 1, 1);
		DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y + 20, Contr->Mouse.x - 15, Contr->Mouse.y - 15, true, LabelsLayer, 1, 1, 1);
		if (FireReady <= 75)
		{
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - FireReady * 15 / 75, Contr->Mouse.x - 15, Contr->Mouse.y + FireReady * 15 / 75, true, LabelsLayer, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - FireReady * 15 / 75, Contr->Mouse.x + 15, Contr->Mouse.y + FireReady * 15 / 75, true, LabelsLayer, 0.5, 0, 1);
		}
		else
		{
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 15, Contr->Mouse.x - 15, Contr->Mouse.y + 15, true, LabelsLayer, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - 15, Contr->Mouse.x + 15, Contr->Mouse.y + 15, true, LabelsLayer, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 20, Contr->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Contr->Mouse.y - 15, true, LabelsLayer, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - 20, Contr->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Contr->Mouse.y - 15, true, LabelsLayer, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y + 20, Contr->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Contr->Mouse.y + 15, true, LabelsLayer, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y + 20, Contr->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Contr->Mouse.y + 15, true, LabelsLayer, 0.5, 0, 1);
		}
		DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 20, Contr->Mouse.x + 20, Contr->Mouse.y + 20, false, LabelsLayer);
		DrawRect(Contr->Mouse.x - 15, Contr->Mouse.y - 15, Contr->Mouse.x + 15, Contr->Mouse.y + 15, false, LabelsLayer);
		if (player->PlaneReady == 1)
		{
			PlaneImage->Draw(-MainCamera.Width / 2 + 50, -MainCamera.Height / 2 + 50, InterfaceLayer, 90, 0.5, 0.5, true);
		}
		//for (double i = -20; i <= 20; i+=0.2)
		//{
		Vector Laser = v(x, y);
		double Sin = dsin(angle + TowerAngle);
		double Cos = dcos(angle + TowerAngle);
		for (int l = 0; l < sqrt(Map1->image->Width*Map1->image->Width + Map1->image->Height*Map1->image->Height); l++)
		{
			bool IsColl = false;
			ForEach(SolidUnit, Unit)
			{
				if ((BaseUnit*)Unit == (BaseUnit*)this) continue;
				if (!CollTable[OT_BULLET][Unit->Type]) continue;
				if (Unit->PixelCheck(Laser.x, Laser.y)) IsColl = true;
			}
			if (IsColl)
				break;
			Laser.x += Cos;
			Laser.y += Sin;
		}
		//DrawLine(x, y, Laser.x, Laser.y, 97, 1, 0, 0, 0.7);
		/*DrawRect(Laser.x - 2, Laser.y - 2, Laser.x + 2, Laser.y + 2, true, 97, 1, 0, 0, 0.5);
		DrawRect(Laser.x - 1, Laser.y - 3, Laser.x + 1, Laser.y + 3, true, 97, 1, 0, 0, 0.5);
		DrawRect(Laser.x - 3, Laser.y - 1, Laser.x + 3, Laser.y + 2, true, 97, 1, 0, 0, 0.5);*/
		SightImage->Draw(Laser.x, Laser.y, LabelsLayer, angle + TowerAngle);
	}

	TextStd->Draw(NickName, x, y + 50, LabelsLayer, 1);
}
void Tank::Die()
{
	if (NeedDelete) return;
	HitPoints = 0;
	Delete();
	player->DeathsCount++;
	if (ManagedUnit<Box>::IsEmpty())
		new Box(x, y);
	player->tank = NULL;
}
Tank::~Tank()
{
	delete contmask;
	Tower->Delete();
}

Map::Map(string MapDir) :
	PointUnit(0, 0, 0),
	GraphicUnit(new Image("Resources\\" + MapDir + "\\.bmp"), MapLayer),
	SolidUnit(new Mask("Resources\\" + MapDir + "\\Coll_Mask.bmp", image->RealWidth / 2, image->RealHeight / 2), OT_MAP)
{
	ifstream fin;
	fin.open("Resources\\" + MapDir + "\\Points.txt");
	fin >> PointsCount;
	Net = new int*[PointsCount];
	TargetPoints = new PointUnit[PointsCount];
	for (int i = 0; i < PointsCount; i += 1)
	{
		fin >> TargetPoints[i].x;
		fin >> TargetPoints[i].y;
		//fin >> TargetPoints[i].angle;
		//TargetPoints[i].angle = atan2(TargetPoints[i].y, TargetPoints[i].x) * 180 / PI;
	}
	NearsCounts = new int[PointsCount];
	for (int i = 0; i < PointsCount; i += 1)
	{
		fin >> NearsCounts[i];
		Net[i] = new int[NearsCounts[i]];
		for (int i1 = 0; i1 < NearsCounts[i]; i1 += 1)
		{
			fin >> Net[i][i1];
		}
	}
	for (int i = 0; i < PointsCount; i += 1)
	{
		PointUnit NearPoint = TargetPoints[Net[i][rand() % NearsCounts[i]]];
		TargetPoints[i].angle = atan2(NearPoint.y - TargetPoints[i].y, NearPoint.x - TargetPoints[i].x) * 180 / PI;
	}
	Pathes = new int**[PointsCount];
	PathesLen = new int*[PointsCount];
	PathesDist = new int*[PointsCount];
	for (int i = 0; i < PointsCount; i += 1)
	{
		Pathes[i] = new int*[PointsCount];
		PathesLen[i] = new int[PointsCount];
		PathesDist[i] = new int[PointsCount];
		for (int i1 = 0; i1 < PointsCount; i1 += 1)
		{
			fin >> PathesLen[i][i1];
			Pathes[i][i1] = new int[PathesLen[i][i1]];
			for (int i2 = 0; i2 < PathesLen[i][i1]; i2 += 1)
			{
				fin >> Pathes[i][i1][i2];
			}
			fin >> PathesDist[i][i1];
		}
	}
	ifstream Script("Resources\\" + MapDir + "\\Script.txt");
	if (Script.is_open())
	{
		while (!Script.eof())
		{
			string Command;
			std::getline(Script, Command);
			GameConsole->Execute(Command);
		}
	}
}

Player::Player(string NickName, TankController* Contr) :
	NickName(NickName),
	Contr(Contr),
	PlaneReady(false),
	KillsCount(0),
	WinsCount(0),
	DeathsCount(0),
	tank(NULL)
{}
void Player::Spawn()
{
	if (tank != NULL) tank->Delete();
	tank = new Tank(SelectSpawnPoint(), this, Contr, NickName);
	SpawnProc();
}
PointUnit Player::SelectSpawnPoint()
{
	srand(clock() + rand());
	while (true)
	{
		SpawnPointIndex = rand() % Map1->PointsCount;
		ForEach(Player, p)
		{
			if (p != this && SpawnPointIndex == p->SpawnPointIndex) SpawnPointIndex = -1;
		}
		if (SpawnPointIndex >= 0)
			break;
	}
	return PointUnit(Map1->TargetPoints[SpawnPointIndex]);
}
void Player::SpawnProc()
{
	if (this == Player1)
	{
		(new AnimatedGraphicUnit(PointerImage, LabelsLayer, false))->SetPoint(tank->x, tank->y, 0);
		WathedPlayer = this;
	}
}

Bot::Bot(string NickName) :
	Player(NickName, new TankController())
{}
void Bot::SteepProc()
{
	if (tank == NULL) return;
	//Initialization
	Contr->KeyFirePressed = false;
	Contr->KeyRightPressed = false;
	Contr->KeyLeftPressed = false;
	Contr->KeyForwardPressed = false;
	Contr->KeyBackPressed = false;
	//Searching target tank
	int MinDist = MAXINT;
	TargetTank = NULL;
	bool TargetVisible = false;
	ForEach(Tank, Unit)
	{
		if (tank == Unit) continue;
		if (Visible(tank->x, tank->y, Unit->x, Unit->y, Map1->mask, Map1->x, Map1->y))
		{
			if (!TargetVisible) MinDist = MAXINT;
			TargetVisible = true;
		}
		else if (TargetVisible) continue;
		if (DistToPoint(tank->x, tank->y, Unit->x, Unit->y) < MinDist)
		{
			TargetTank = Unit;
			MinDist = DistToPoint(tank->x, tank->y, Unit->x, Unit->y);
		}
	}
	if (TargetTank == NULL)
	{
		int MinDist = MAXINT;
		ForEach(Tank, Unit)
		{
			if (Unit != tank && DistToPoint(tank->x, tank->y, Unit->x, Unit->y) < MinDist)
			{
				TargetTank = Unit;
				MinDist = DistToPoint(tank->x, tank->y, Unit->x, Unit->y);
			}
		}
	}
	if (TargetTank == NULL) return;
	Contr->KeyPlanePressed = true;
	Contr->Mouse.x = TargetTank->x;
	Contr->Mouse.y = TargetTank->y;
	if (Visible(tank->x, tank->y, TargetTank->x, TargetTank->y, Map1->mask, Map1->x, Map1->y))
	{
		//Moving = false;
		if (fabs(AnglesDiff(tank->TowerAngle + tank->angle, AngleToPoint(tank->x, tank->y, TargetTank->x, TargetTank->y))) < 3)
			Contr->KeyFirePressed = true;
	}
	else //Searching path to target
	{
		int MinDist;
		MinDist = MAXINT;
		int MinPoint;
		MinPoint = -1;
		bool Back = false;
		for (int i = 0; i < Map1->PointsCount; i += 1)
		{
			if (Visible(Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, TargetTank->x, TargetTank->y, Map1->mask, Map1->x, Map1->y))
			{
				if (Map1->PathesDist[CurrentTargetPoint][i] + DistToPoint(tank->x, tank->y, Map1->TargetPoints[CurrentTargetPoint].x, Map1->TargetPoints[CurrentTargetPoint].y) < MinDist)
				{
					MinDist = Map1->PathesDist[CurrentTargetPoint][i];
					MinPoint = i;
					Back = false;
				}
				if (PrevPoint != -1 && Map1->PathesDist[PrevPoint][i] + DistToPoint(tank->x, tank->y, Map1->TargetPoints[PrevPoint].x, Map1->TargetPoints[PrevPoint].y) < MinDist)
				{
					MinDist = Map1->PathesDist[PrevPoint][i];
					MinPoint = i;
					Back = true;
				}
			}
		}
		if (MinPoint == -1) MinPoint = 5;
		PathPoint = 0;
		if (Back == false)
		{
			Path = Map1->Pathes[CurrentTargetPoint][MinPoint];
			PathLen = Map1->PathesLen[CurrentTargetPoint][MinPoint];
		}
		if (Back == true)
		{
			CurrentTargetPoint = PrevPoint;
			Path = Map1->Pathes[PrevPoint][MinPoint];
			PathLen = Map1->PathesLen[PrevPoint][MinPoint];
		}
	}
	//Moving
	if (DistToPoint(tank->x, tank->y, TargetPoint.x, TargetPoint.y) <= 15)
	{
		if (PathPoint < PathLen)
		{
			PrevPoint = CurrentTargetPoint;
			CurrentTargetPoint = Path[PathPoint++];
		}
		Moving = false;
	}
	else Moving = 1;
	TargetPoint = Map1->TargetPoints[CurrentTargetPoint];
	double TargetAngle;
	TargetAngle = AngleToPoint(tank->x, tank->y, TargetPoint.x, TargetPoint.y);
	if (AnglesDiff(TargetAngle, tank->angle) > 5 && AnglesDiff(TargetAngle, tank->angle) < 175) Contr->KeyLeftPressed = true;
	if (AnglesDiff(TargetAngle, tank->angle) < -5 && AnglesDiff(TargetAngle, tank->angle) > -175) Contr->KeyRightPressed = true;
	if (fabs(AnglesDiff(TargetAngle, tank->angle)) > 90)
	{
		Contr->KeyLeftPressed = !Contr->KeyLeftPressed;
		Contr->KeyRightPressed = !Contr->KeyRightPressed;
	}
	if (fabs(AnglesDiff(TargetAngle, tank->angle)) < 30 && Moving == 1) Contr->KeyForwardPressed = true;
	if (fabs(AnglesDiff(TargetAngle, tank->angle)) > 150 && Moving == 1) Contr->KeyBackPressed = true;
}
PointUnit Bot::SelectSpawnPoint()
{
	return Player::SelectSpawnPoint();
}
void Bot::SpawnProc()
{
	int MinDist = MAXINT;
	CurrentTargetPoint = 6;
	for (int i = 0; i < Map1->PointsCount; i++)
	{
		if (DistToPoint(tank->x, tank->y, Map1->TargetPoints[i].x, Map1->TargetPoints[i].y) < MinDist)
		{
			MinDist = DistToPoint(tank->x, tank->y, Map1->TargetPoints[i].x, Map1->TargetPoints[i].y);
			CurrentTargetPoint = i;
		}
	}
	TargetPoint = Map1->TargetPoints[CurrentTargetPoint];
	PrevPoint = -1;
	PathLen = 0;
	PathPoint = 0;
	PathEnd = CurrentTargetPoint;
}

Box::Box(int x, int y) :
	PointUnit(x, y, 0),
	GraphicUnit(HealImage, TanksBodyLayer),
	SolidPhysicUnit(HealMask, OT_HEAL)
{}
void Box::CollProc(SolidUnit* Other)
{
	if (Other->Type == OT_BULLET || Other->Type == OT_EXPLOSIVE)
	{
		new Explosive(x, y);
		Delete();
	}
}

Plane::Plane(int x, int y) :
	GraphicUnit(PlaneImage, SkyLayer),
	Time(0)
{
	TurnSpeed = rand() % 180 - 90;
	angle = rand() % 360;
	angle -= TurnSpeed;
	this->x = x - 8 * 75 / PI / (TurnSpeed) * 180 * 2 * dsin(TurnSpeed / 2)*dcos(angle + TurnSpeed / 2);
	this->y = y - 8 * 75 / PI / (TurnSpeed) * 180 * 2 * dsin(TurnSpeed / 2)*dsin(angle + TurnSpeed / 2);
	TurnSpeed /= 75;
}
void Plane::SteepProc()
{
	Time++;
	angle += TurnSpeed;
	x += 8 * dcos(angle);
	y += 8 * dsin(angle);
	if (Time == 75)
		//if (Time % 5 == 0)
	{
		new Explosive(x, y);
	}
	if (!InScreen() && Time > 75)
		Delete();
}

Explosive::Explosive(int x, int y) :
	SolidUnit(ExplosiveMask, OT_EXPLOSIVE),
	PointUnit(x, y, 0)
{
	(new AnimatedGraphicUnit(ExplosiveImage, EffectsLayer, false))->SetPoint(x, y, rand() % 360);
}
void Explosive::SteepProc()
{
	Delete();
}

TankController::TankController() :
	Contr(NULL)
{}
TankController::TankController(Controller* Contr, int KeyForwardCode, int KeyBackCode, int KeyRightCode, int KeyLeftCode, int KeyFireCode, int KeyPlaneCode) :
	Contr(Contr),
	KeyForwardCode(KeyForwardCode),
	KeyBackCode(KeyBackCode),
	KeyRightCode(KeyRightCode),
	KeyLeftCode(KeyLeftCode),
	KeyFireCode(KeyFireCode),
	KeyPlaneCode(KeyPlaneCode)
{}
void TankController::Check()
{
	if (Contr == NULL) return;
	KeyForwardPressed = Contr->Keys[KeyForwardCode] > 0;
	KeyBackPressed = Contr->Keys[KeyBackCode] > 0;
	KeyRightPressed = Contr->Keys[KeyRightCode] > 0;
	KeyLeftPressed = Contr->Keys[KeyLeftCode] > 0;
	KeyFirePressed = Contr->Keys[KeyFireCode] > 0;
	KeyPlanePressed = Contr->Keys[KeyPlaneCode] > 0;
	Mouse = Contr->Mouse;
}

void Draw()
{
	if (Contr1.Keys[VK_TAB] >= 1)
	{
		glTranslatef(MainCamera.x, MainCamera.y, 0);
		if (GameMode == 0)
		{
			DrawRect(-125, -200, 125, 200, true, InterfaceBackLayer, 0.75, 0.75, 0.75, 0.5);
			DrawRect(-125, -200, 125, 200, false, InterfaceLayer);
			TextStd->Draw("NickName", -123, 171, InterfaceLayer, false);
			DrawLine(15, -200, 15, 200, InterfaceLayer);
			TextStd->Draw("Kills", 17, 171, InterfaceLayer, false);
			DrawLine(65, -200, 65, 200, InterfaceLayer);
			TextStd->Draw("Deaths", 67, 171, InterfaceLayer, false);
			DrawLine(-125, 184, 125, 184, InterfaceLayer);
			int i = 0;
			ForEach(Player, player)
			{
				if (player->tank == NULL)
					TextStd->Draw("*DEAD* " + player->NickName, -123, 155 - i * 16, InterfaceLayer, false);
				else
					TextStd->Draw(player->NickName, -123, 155 - i * 16, InterfaceLayer, false);
				TextStd->Draw(to_string(player->KillsCount), 17, 155 - i * 16, InterfaceLayer, false);
				TextStd->Draw(to_string(player->DeathsCount), 67, 155 - i * 16, InterfaceLayer, false);
				DrawLine(-125, 168 - i * 16, 125, 168 - i * 16, InterfaceLayer);
				i++;
			}
		}
		if (GameMode == 1)
		{
			DrawRect(-150, -200, 150, 200, true, InterfaceBackLayer, 0.75, 0.75, 0.75, 0.5);
			DrawRect(-150, -200, 150, 200, false, InterfaceLayer);
			TextStd->Draw("NickName", -148, 171, InterfaceLayer, false);
			DrawLine(-10, -200, -10, 200, InterfaceLayer);
			TextStd->Draw("Wins", -8, 171, InterfaceLayer, false);
			DrawLine(40, -200, 40, 200, InterfaceLayer);
			TextStd->Draw("Kills", 42, 171, InterfaceLayer, false);
			DrawLine(90, -200, 90, 200, InterfaceLayer);
			TextStd->Draw("Deaths", 92, 171, InterfaceLayer, false);
			DrawLine(-150, 184, 150, 184, InterfaceLayer);
			int i = 0;
			ForEach(Player, player)
			{
				if (player->tank == NULL)
					TextStd->Draw("*DEAD* " + player->NickName, -148, 155 - i * 16, InterfaceLayer, false);
				else
					TextStd->Draw(player->NickName, -148, 155 - i * 16, InterfaceLayer, false);
				TextStd->Draw(to_string(player->WinsCount), -8, 155 - i * 16, InterfaceLayer, false);
				TextStd->Draw(to_string(player->KillsCount), 42, 155 - i * 16, InterfaceLayer, false);
				TextStd->Draw(to_string(player->DeathsCount), 92, 155 - i * 16, InterfaceLayer, false);
				DrawLine(-150, 168 - i * 16, 150, 168 - i * 16, InterfaceLayer);
				i++;
			}
		}
		if (DbgPointsMode == 1)
			for (int i = 0; i < Map1->PointsCount; i++)
			{
				PointerImage->Draw(0, Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, LabelsLayer, 0, 0.5, 0.5);
				for (int j = 0; j < Map1->NearsCounts[i]; j++)
					if (j > i)
						DrawLine(Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, Map1->TargetPoints[j].x, Map1->TargetPoints[j].y, LabelsLayer, 1, 0, 1);
			}
		if (DbgClockMode == 1)
		{
			DrawLine(-250, 0, -250 + 50 * dcos(Time), 50 * dsin(Time), InterfaceLayer, 0, 1, 0);
			DrawLine(-250, 0, -250 + 50 * dcos(clock() * 60 / 1000), 50 * dsin(clock() * 60 / 1000), InterfaceLayer, 0, 0, 1);
		}
	}
	if (DbgPointsMode == 2)
		for (int i = 0; i < Map1->PointsCount; i++)
		{
			PointerImage->Draw(0, Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, LabelsLayer, 0, 0.5, 0.5);
			for (int j = 0; j < Map1->NearsCounts[i]; j++)
				if (Map1->Net[i][j] > i)
					DrawLine(Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, Map1->TargetPoints[Map1->Net[i][j]].x, Map1->TargetPoints[Map1->Net[i][j]].y, LabelsLayer, 1, 0, 1);
		}
	if (DbgClockMode == 2)
	{
		DrawLine(-250, 0, -250 + 50 * dcos(Time), 50 * dsin(Time), InterfaceLayer, 0, 1, 0);
		DrawLine(-250, 0, -250 + 50 * dcos(clock() * 60 / 1000), 50 * dsin(clock() * 60 / 1000), InterfaceLayer, 0, 0, 1);
	}
}

void Console::Regisrer()
{
	GameConsole->RegisterCommand("heal", [&](string Args)
	{
		bool Finded = false;
		if (Args == "")
		{
			Player1->tank->HitPoints = 5;
			Finded = true;
		}
		ForEach(Tank, tank)
		{
			if (tank->NickName == Args)
			{
				tank->HitPoints = 5;
				Finded = true;
			}
		}
		if (!Finded) GameConsole->SetMessage("Player not found");
	});
	GameConsole->RegisterCommand("wath", [&](string Args)
	{
		bool Finded = false;
		ForEach(Player, player)
		{
			if (player->NickName == Args || (Args == "" && player->tank != NULL))
			{
				WathedPlayer = player;
				Finded = true;
				break;
			}
		}
		if (!Finded) GameConsole->SetMessage("Player not found");
	});
	GameConsole->RegisterCommand("kill", [&](string Args)
	{
		bool Finded = false;
		ForEach(Tank, tank)
		{
			if (tank->NickName == Args)
			{
				tank->Die();
				Finded = true;
			}
		}
		if (!Finded) GameConsole->SetMessage("Player not found");
	});
	GameConsole->RegisterCommand("plane", [&](string Args)
	{
		bool Finded = false;
		if (Args == "")
		{
			Player1->PlaneReady = true;
			Finded = true;
		}
		ForEach(Tank, tank)
		{
			if (tank->NickName == Args)
			{
				tank->player->PlaneReady = true;
				Finded = true;
			}
		}
		if (!Finded) GameConsole->SetMessage("Player not found");
	});
	GameConsole->RegisterCommand("mode", [&](string Args)
	{
		int NewMode;
		try
		{
			NewMode = stoi(Args);
		}
		catch (...)
		{
			GameConsole->SetMessage("Invalid argument: must be integer");
			return;
		}
		if (NewMode == 0 || NewMode == 1)
			GameMode = NewMode;
		else GameConsole->SetMessage("Mode " + to_string(NewMode) + " is not exist");
	});
	GameConsole->RegisterCommand("cammode", [&](string Args)
	{
		int NewMode;
		try
		{
			NewMode = stoi(Args);
		}
		catch (...)
		{
			GameConsole->SetMessage("Invalid argument: must be integer");
			return;
		}
		if (NewMode == 0)
		{
			CamMode = 0;
			MainCamera.SetPoint(0, 0, 0);
		}
		else if (NewMode == 1 || NewMode == 2)
		{
			CamMode = NewMode;
		}
		else GameConsole->SetMessage("CamMode " + to_string(NewMode) + " is not exist");
	});
	GameConsole->RegisterCommand("scale", [&](string Args)
	{
		double Scale;
		try
		{
			Scale = stod(Args);
		}
		catch (...)
		{
			GameConsole->SetMessage("Invalid argument: must be double");
			return;
		}
		MainCamera.Scale = Scale;
	});
	GameConsole->RegisterCommand("fullscreen", [&](string Args)
	{
		int NewMode;
		try
		{
			NewMode = stoi(Args);
		}
		catch (...)
		{
			GameConsole->SetMessage("Invalid argument: must be integer");
			return;
		}
		if (NewMode == 0)
		{
			NeedWindowed = true;
		}
		else if (NewMode == 1)
		{
			NeedFullscreen = true;
		}
		else GameConsole->SetMessage("Argument must be 0 or 1");
	});
	GameConsole->RegisterCommand("map", [&](string Args)
	{
		if (access(("Resources\\" + Args).c_str(), 0) != 0)
		{
			GameConsole->SetMessage("Directry Resources\\" + Args + " not found");
			return;
		}
		if (access(("Resources\\" + Args + "\\.bmp").c_str(), 0) != 0)
		{
			GameConsole->SetMessage("File Resources\\" + Args + "\\.bmp not found");
			return;
		}
		if (access(("Resources\\" + Args + "\\Coll_Mask.bmp").c_str(), 0) != 0)
		{
			GameConsole->SetMessage("File Resources\\" + Args + "\\Coll_Mask.bmp not found");
			return;
		}
		if (access(("Resources\\" + Args + "\\Points.txt").c_str(), 0) != 0)
		{
			GameConsole->SetMessage("File Resources\\" + Args + "\\Points.txt not found");
			return;
		}
		ForEach(Player, player)
		{
			if (player->tank != NULL)
				player->tank->Delete();
			player->Delete();
			player->Active = false;
		}
		Map1->Delete();
		Map1 = new Map(Args);
		Player1 = new Player("Player", Player1->Contr);
		WathedPlayer = Player1;
	});
	GameConsole->RegisterCommand("newbot", [&](string Args)
	{
		(new Bot(Args));
	});
	GameConsole->RegisterCommand("delbot", [&](string Args)
	{
		bool Finded = false;
		ForEach(Bot, bot)
		{
			if (bot->NickName == Args || Args == "")
			{
				if (bot->tank != NULL)
					bot->tank->Delete();
				bot->Delete();
				Finded = true;
			}
		}
		if (!Finded) GameConsole->SetMessage("Bot not found");
	});
	GameConsole->RegisterCommand("exit", [&](string Args)
	{
		Closed = true;
	});
	GameConsole->RegisterCommand("dbgpoints", [&](string Args)
	{
		int NewMode;
		try
		{
			NewMode = stoi(Args);
		}
		catch (...)
		{
			GameConsole->SetMessage("Invalid argument: must be integer");
			return;
		}
		if (NewMode >= 0 || NewMode <= 2)
			DbgPointsMode = NewMode;
		else GameConsole->SetMessage("DbgPointsMode " + to_string(NewMode) + " is not exist");
	});
	GameConsole->RegisterCommand("dbgclock", [&](string Args)
	{
		int NewMode;
		try
		{
			NewMode = stoi(Args);
		}
		catch (...)
		{
			GameConsole->SetMessage("Invalid argument: must be integer");
			return;
		}
		if (NewMode >= 0 || NewMode <= 2)
			DbgClockMode = NewMode;
		else GameConsole->SetMessage("DbgClockMode " + to_string(NewMode) + " is not exist");
	});
	GameConsole->RegisterCommand("login", [&](string Args)
	{
		ifstream ifile("Resources//Players.txt", ios_base::in);
		while (!ifile.eof())
		{
			string NickName;
			ifile >> NickName;
			if (NickName == Args)
			{
				Player1->NickName = NickName;
				Player1->tank->NickName = NickName;
				ifile >> Player1->KillsCount >> Player1->DeathsCount;
				return;
			}
			else
			{
				ifile >> string() >> string();
			}
		}
		ifile.close();
		ofstream ofile("Resources//Players.txt", ios_base::out | ios_base::app);
		Player1->NickName = Args;
		Player1->tank->NickName = Args;
		ofile << Args << " " << Player1->KillsCount << " " << Player1->DeathsCount << '\n';
	});
	GameConsole->RegisterCommand("save", [&](string Args)
	{
		ifstream ifile("Resources//Players.txt", ios_base::in);
		list<tuple<string, int, int>> Records;
		while (!ifile.eof())
		{
			string NickName;
			int KillsCount;
			int DeathsCount;
			ifile >> NickName;
			if (NickName == "") break;
			if (NickName == Player1->NickName)
			{
				KillsCount = Player1->KillsCount;
				DeathsCount = Player1->DeathsCount;
				ifile >> string() >> string();
			}
			else
			{
				ifile >> KillsCount >> DeathsCount;
			}
			Records.emplace_back(tuple<string, int, int>(NickName, KillsCount, DeathsCount));
		}
		ifile.close();
		ofstream ofile("Resources//Players.txt", ios_base::out);
		for (tuple<string, int, int> Record : Records)
		{
			ofile << get<0>(Record) << " " << get<1>(Record) << " " << get<2>(Record) << '\n';
		}
	});
}
void InitLayers()
{
	MapLayer = Layer::Higher(LowestLayer, false);
	TanksBodyLayer = Layer::Higher(MapLayer, false);
	TanksTowerLayer = Layer::Higher(TanksBodyLayer, false);
	EffectsLayer = Layer::Lower(TanksTowerLayer, false);
	SkyLayer = Layer::Higher(TanksTowerLayer, false);
	LabelsLayer = Layer::Lower(SkyLayer, false);
}
void LoadImages()
{
	TankBodyImage = new Image("Resources\\Tank\\Body\\.bmp");
	TankTowerImage = new Image("Resources\\Tank\\Tower\\.bmp");
	BulletImage = new Image("Resources\\Bullet\\.bmp");
	Fire1Image = new Image("Resources\\Fire\\1.bmp");
	Fire2Image = new Image("Resources\\Fire\\2.bmp");
	Fire3Image = new Image("Resources\\Fire\\3.bmp");
	FireImage = new AnimatedImage(3, Fire1Image, 6, Fire2Image, 6, Fire3Image, 8);
	Image* Hit1Image = new Image("Resources\\Hit\\.bmp");
	HitImage = new AnimatedImage(1, Hit1Image, 6);
	HealImage = new Image("Resources\\Heal\\.bmp");
	PlaneImage = new Image("Resources\\Plane\\.bmp");
	Image* Explosive1Image = new Image("Resources\\Explosive\\.bmp");
	ExplosiveImage = new AnimatedImage(1, Explosive1Image, 6);
	SightImage = new Image("Resources\\Tank\\Sight\\.bmp");
	PointerImage = new AnimatedImage(1, new Image("Resources\\Pointer\\.bmp"), 40);
	TextStd = new Text(L"Resources\\Font.bmp", 256, 32, 32);
}
void LoadMasks()
{
	TankMask = new Mask("Resources\\Tank\\Body\\Mask.bmp", 36, 24);
	BulletMask = new Mask("Resources\\Bullet\\Mask.bmp", 12, 4);
	HealMask = new Mask("Resources\\Heal\\Mask.bmp", 24, 24);
	ExplosiveMask = new Mask("Resources\\Explosive\\Coll_Mask.bmp", 40, 40);
}
void GameInit()
{
	GameConsole = new Console();
	GameConsole->Regisrer();

	Map1 = new Map("Map1");

	Player1 = new Player("Player", new TankController(&Contr1, 'W', 'S', 'D', 'A', 1, 2));
	Player1->Spawn();

	WathedPlayer = Player1;
}
void SteepProc()
{
	Time++;
	if (GameMode == 0)
	{
		ForEach(Player, p)
			if (p->tank == NULL) p->Spawn();
	}
	if (GameMode == 1)
	{
		if (ManagedUnit<Tank>::List.size() == 1)
			ForEach(Tank, t) t->player->WinsCount++;
		if (ManagedUnit<Tank>::List.size() <= 1)
		{
			CallEach(Bullet) Delete();
			CallEach(Box) Delete();
			CallEach(Player) Spawn();
			WathedPlayer = Player1;
		}
	}
}
void SetCamera()
{
	if (WathedPlayer == NULL || WathedPlayer->tank == NULL)
		return;
	if (CamMode == 1)
		MainCamera.SetPoint((WathedPlayer->tank->x * 2 + Contr1.Mouse.x) / 3, (WathedPlayer->tank->y * 2 + Contr1.Mouse.y) / 3, 0);
	if (CamMode == 2)
		MainCamera.SetPoint((WathedPlayer->tank->x * 2 + Contr1.Mouse.x) / 3, (WathedPlayer->tank->y * 2 + Contr1.Mouse.y) / 3, -WathedPlayer->tank->angle + 90);
}