#include "Game.h"

HINSTANCE hInst;
RECT RectInScreen;

Map* Map1;
Image* TankBodyImage;
Image* TankTowerImage;
Image* BulletImage;
Image* MapImage;
Image* Fire1Image;
Image* Fire2Image;
Image* Fire3Image;
AnimatedImage* FireImage;
AnimatedImage* HitImage;
Image* HealImage;
Image* FirTreeImage;
Image* PlaneImage;
AnimatedImage* ExplosiveImage;
Image* HappyBirthdayImage;

Mask* TankMask;
Mask* Map1Mask;
Mask* BulletMask;
Mask* HealMask;
Mask* ExplosiveMask;

Player* Player1;

bool CollTable[5][5] =
{               /*Tank*/ /*Bullet*/ /*Map*/ /*Heal*/ /*Explosive*/
	/*Tank*/       1,       1,      1,        1,         1,
	/*Bullet*/     1,       0,      1,        0,         0,
	/*Map*/        0,       0,      0,        0,         0,
	/*Heal*/       1,       0,      1,        1,         0,
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

Bullet::Bullet(double x, double y, double angle, Player* ParPlayer):
	PointUnit(x, y, angle),
	SolidUnit(BulletMask, OT_BULLET),
	GraphicUnit(BulletImage, 95),
	ParPlayer(ParPlayer)
{}
void Bullet::SteepProc()
{
	x += 15 * dcos(angle);
	y += 15 * dsin(angle);
	//angle += rand() % 7 - 3;
	if (InScreen() == false) ManagedUnit<Bullet>::Delete();
}
void Bullet::CollProc(SolidUnit* Other)
{
	if (ParPlayer->tank != Other)
	{
		Delete();
		(new AnimatedGraphicUnit(HitImage, 97, false))->SetPoint(x, y, 180 + angle);
	}
}
Bullet::~Bullet()
{}

Tank::Tank(double x, double y, double angle, Player* player, Controller* Contr, LPSTR NickName, int KeyForward, int KeyBack, int KeyRight, int KeyLeft, int KeyFire):
	PointUnit(x, y, angle),
	GraphicUnit(TankBodyImage, 98),
	SolidPhysicUnit(TankMask, OT_TANK),
	player(player),
	Contr(Contr),
	NickName(NickName),
	KeyForward(KeyForward),
	KeyBack(KeyBack),
	KeyRight(KeyRight),
	KeyLeft(KeyLeft),
	KeyFire(KeyFire),
	HitPoints(5),
	FireReady(100),
	TowerAngle(0),
	contmask(new ContMask(mask)),
	Tower(new GraphicUnit(TankTowerImage, 96))
{}
void Tank::SteepProc()
{
	FireReady += 1;
	if (HitPoints > 5) HitPoints = 5;
	if (FireReady > 150) FireReady = 150;
	if (Contr->Keys[KeyFire] == 1 && FireReady >= 150)
	{
		new Bullet(x + dcos(angle + TowerAngle) * 60, y + dsin(angle + TowerAngle) * 60, angle + TowerAngle, player);
		(new AnimatedGraphicUnit(FireImage, 97, false))
			->SetPoint(x + dcos(angle + TowerAngle) * 60, y + dsin(angle + TowerAngle) * 60, angle + TowerAngle);
		FireReady = 0;
	}
	if (Contr->Keys[VK_RBUTTON] == 1 && player->PlaneReady == 1)
	{
		new Plane(Contr->Mouse.x, Contr->Mouse.y);
		player->PlaneReady = false;
	}
	double TargetAngle;
	TargetAngle = AngleToPoint(x, y, Contr->Mouse.x, Contr->Mouse.y);
	if (AnglesDiff(TargetAngle, angle + TowerAngle) < -2) TowerAngle -= 2;
	else if (AnglesDiff(TargetAngle, angle + TowerAngle) > 2) TowerAngle += 2;
	else TowerAngle = TargetAngle - angle;

	BYTE Mov = 0;
	if (Contr->Keys[KeyForward] == 1) Mov |= 0b10000000;
	if (Contr->Keys[KeyLeft] == 1)    Mov |= 0b01000000;
	if (Contr->Keys[KeyRight] == 1)   Mov |= 0b00100000;
	if (Contr->Keys[KeyBack] == 1)    Mov |= 0b00010000;
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
	if (Other->Type == OT_BULLET && ((Bullet*) Other)->ParPlayer != player)
	{
		HitPoints -= 1;
	}
	if (Other->Type == OT_EXPLOSIVE)
		HitPoints -= 3;
	if (HitPoints <= 0)
	{
		HitPoints = 0;
		Delete();
		player->Spawn();
		if (Other->Type == OT_BULLET && ((Bullet*) Other)->ParPlayer != player)
		{
			((Bullet*) Other)->ParPlayer->KillsCount++;
		}
		player->DeathsCount++;
		if (ManagedUnit<Box>::IsEmpty())
			new Box(x, y);
	}
}
void Tank::DrawProc()
{
	Tower->x = x;
	Tower->y = y;
	Tower->angle = angle + TowerAngle;
	DrawRect(x - 50, y - 50, x + 50, y - 45, 1, 95, 1, 1, 1);
	DrawRect(x - 50, y - 50, x + 100 * HitPoints / 5 - 50, y - 45, 1, 95, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
	DrawRect(x - 50, y - 50, x + 50, y - 45, false, 95);
	if (player == Player1)
	{
		DrawRect(-400, -300, -300, -290, true, 96, 1, 1, 1);
		DrawRect(-400, -300, 100 * FireReady / 150 - 400, -290, true, 96, 0.5, 0, 1);
		DrawRect(-400, -300, -300, -290, false, 96);
		DrawRect(-400, -280, -300, -290, true, 96, 1, 1, 1);
		DrawRect(-400, -280, 100 * HitPoints / 5 - 400, -290, true, 96, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
		DrawRect(-400, -280, -300, -290, false, 96, 0, 0, 0);

		DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 20, Contr->Mouse.x + 15, Contr->Mouse.y - 15, 1, 95, 1, 1, 1);
		DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - 20, Contr->Mouse.x + 15, Contr->Mouse.y + 15, 1, 95, 1, 1, 1);
		DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y + 20, Contr->Mouse.x - 15, Contr->Mouse.y + 15, 1, 95, 1, 1, 1);
		DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y + 20, Contr->Mouse.x - 15, Contr->Mouse.y - 15, 1, 95, 1, 1, 1);
		if (FireReady <= 75)
		{
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - FireReady * 15 / 75, Contr->Mouse.x - 15, Contr->Mouse.y + FireReady * 15 / 75, true, 95, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - FireReady * 15 / 75, Contr->Mouse.x + 15, Contr->Mouse.y + FireReady * 15 / 75, true, 95, 0.5, 0, 1);
		}
		else
		{
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 15, Contr->Mouse.x - 15, Contr->Mouse.y + 15, true, 95, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - 15, Contr->Mouse.x + 15, Contr->Mouse.y + 15, true, 95, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 20, Contr->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Contr->Mouse.y - 15, true, 95, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y - 20, Contr->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Contr->Mouse.y - 15, true, 95, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y + 20, Contr->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Contr->Mouse.y + 15, true, 95, 0.5, 0, 1);
			DrawRect(Contr->Mouse.x + 20, Contr->Mouse.y + 20, Contr->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Contr->Mouse.y + 15, true, 95, 0.5, 0, 1);
		}
		DrawRect(Contr->Mouse.x - 20, Contr->Mouse.y - 20, Contr->Mouse.x + 20, Contr->Mouse.y + 20, false, 95);
		DrawRect(Contr->Mouse.x - 15, Contr->Mouse.y - 15, Contr->Mouse.x + 15, Contr->Mouse.y + 15, false, 95);
		if (player->PlaneReady == 1)
		{
			PlaneImage->Draw(-350, -250, 96, 90, 0.5, 0.5, true);
		}
		//for (double i = -20; i <= 20; i+=0.2)
		//{
		Vector Laser = v(x, y);
		double Sin = dsin(angle + TowerAngle);
		double Cos = dcos(angle + TowerAngle);
		for (int l = 0; l < 1000; l++)
		{
			bool IsColl = false;
			ForEach(SolidUnit, Unit)
			{
				if ((BaseUnit*) Unit == (BaseUnit*)this) continue;
				if (!CollTable[OT_BULLET][Unit->Type]) continue;
				if (Unit->PixelCheck(Laser.x, Laser.y)) IsColl = true;
			}
			if (IsColl)
				break;
			Laser.x += Cos;
			Laser.y += Sin;
		}
		DrawLine(x, y, Laser.x, Laser.y, 97, 1, 0, 0, 0.7);
		DrawRect(Laser.x - 2, Laser.y - 2, Laser.x + 2, Laser.y + 2, true, 97, 1, 0, 0, 0.5);
		DrawRect(Laser.x - 1, Laser.y - 3, Laser.x + 1, Laser.y + 3, true, 97, 1, 0, 0, 0.5);
		DrawRect(Laser.x - 3, Laser.y - 1, Laser.x + 3, Laser.y + 2, true, 97, 1, 0, 0, 0.5);
		//}
	}

	TextStd->Draw(NickName, x, y + 50, 95, 1);
}
Tank::~Tank()
{
	delete contmask;
	Tower->Delete();
}

Map::Map(Image* image, LPCTSTR MapMask, LPCTSTR AIPoints):
	PointUnit(0, 0, 0),
	GraphicUnit(image, 99),
	SolidUnit(Map1Mask, OT_MAP)
{
	ifstream fin;
	fin.open(AIPoints);
	fin >> PointsCount;
	Net = new int*[PointsCount];
	TargetPoints = new POINT[PointsCount];
	for (int i = 0; i < PointsCount; i += 1)
	{
		fin >> TargetPoints[i].x;
		fin >> TargetPoints[i].y;
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
}

Player::Player(LPSTR NickName, Controller* Contr, double x, double y, double angle):
	NickName(NickName),
	Contr(Contr),
	PlaneReady(false),
	SpawnPoint(x, y, angle),
	KillsCount(0),
	DeathsCount(0),
	tank(NULL)
{}
void Player::Spawn()
{
	if (tank != NULL) tank->Delete();
	tank = new Tank(SpawnPoint.x, SpawnPoint.y, SpawnPoint.angle, this, Contr, NickName, 87, 83, 68, 65, 1);
	SpawnProc();
}
void Player::SpawnProc()
{
	srand(clock());
	int Point = rand() % Map1->PointsCount;
	SpawnPoint.x = Map1->TargetPoints[Point].x;
	SpawnPoint.y = Map1->TargetPoints[Point].y;
}

Bot::Bot(LPSTR NickName, double x, double y, double angle):
	Player(NickName, new Controller(), x, y, angle),
	Net(Map1->Net),
	PointsCount(Map1->PointsCount),
	NearsCounts(Map1->NearsCounts),
	TargetPoints(Map1->TargetPoints)
{}
void Bot::SteepProc()
{
	//Initialization
	tank->Contr->Keys[tank->KeyFire] = false;
	tank->Contr->Keys[tank->KeyRight] = false;
	tank->Contr->Keys[tank->KeyLeft] = false;
	tank->Contr->Keys[tank->KeyForward] = false;
	tank->Contr->Keys[tank->KeyBack] = false;
	//Searching target tank
	int MinDist = MAXINT;
	TargetTank = NULL;
	bool TargetVisible = false;
	ForEach(Tank, tank)
	{
		if (tank == tank) continue;
		if (Visible(tank->x, tank->y, tank->x, tank->y, Map1->mask, Map1->x, Map1->y))
		{
			if (!TargetVisible) MinDist = MAXINT;
			TargetVisible = true;
		}
		else if (TargetVisible) continue;
		if (DistToPoint(tank->x, tank->y, tank->x, tank->y) < MinDist)
		{
			TargetTank = tank;
			MinDist = DistToPoint(tank->x, tank->y, tank->x, tank->y);
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
	if (TargetTank != NULL)
	{
		tank->Contr->Keys[VK_RBUTTON] = true;
		tank->Contr->Mouse.x = TargetTank->x;
		tank->Contr->Mouse.y = TargetTank->y;
	}
	if (Visible(tank->x, tank->y, TargetTank->x, TargetTank->y, Map1->mask, Map1->x, Map1->y))
	{
		//Moving = false;
		if (fabs(AnglesDiff(tank->TowerAngle + tank->angle, AngleToPoint(tank->x, tank->y, TargetTank->x, TargetTank->y))) < 3)
			tank->Contr->Keys[tank->KeyFire] = true;
	}
	else //Searching path to target
	{
		int MinDist;
		MinDist = MAXINT;
		int MinPoint;
		MinPoint = -1;
		bool Back = false;
		for (int i = 0; i < PointsCount; i += 1)
		{
			if (Visible(TargetPoints[i].x, TargetPoints[i].y, TargetTank->x, TargetTank->y, Map1->mask, Map1->x, Map1->y))
			{
				if (Map1->PathesDist[CurrentTargetPoint][i] + DistToPoint(tank->x, tank->y, TargetPoints[CurrentTargetPoint].x, TargetPoints[CurrentTargetPoint].y) < MinDist)
				{
					MinDist = Map1->PathesDist[CurrentTargetPoint][i];
					MinPoint = i;
					Back = false;
				}
				if (PrevPoint != -1 && Map1->PathesDist[PrevPoint][i] + DistToPoint(tank->x, tank->y, TargetPoints[PrevPoint].x, TargetPoints[PrevPoint].y) < MinDist)
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
	TargetPoint = TargetPoints[CurrentTargetPoint];
	double TargetAngle;
	TargetAngle = AngleToPoint(tank->x, tank->y, TargetPoint.x, TargetPoint.y);
	if (AnglesDiff(TargetAngle, tank->angle) > 5 && AnglesDiff(TargetAngle, tank->angle) < 175) tank->Contr->Keys[tank->KeyLeft] = true;
	if (AnglesDiff(TargetAngle, tank->angle) < -5 && AnglesDiff(TargetAngle, tank->angle) > -175) tank->Contr->Keys[tank->KeyRight] = true;
	if (fabs(AnglesDiff(TargetAngle, tank->angle)) > 90)
	{
		tank->Contr->Keys[tank->KeyLeft] = !tank->Contr->Keys[tank->KeyLeft];
		tank->Contr->Keys[tank->KeyRight] = !tank->Contr->Keys[tank->KeyRight];
	}
	if (fabs(AnglesDiff(TargetAngle, tank->angle)) < 30 && Moving == 1) tank->Contr->Keys[tank->KeyForward] = true;
	if (fabs(AnglesDiff(TargetAngle, tank->angle)) > 150 && Moving == 1) tank->Contr->Keys[tank->KeyBack] = true;
}
void Bot::SpawnProc()
{
	int MinDist = MAXINT;
	CurrentTargetPoint = 6;
	for (int i = 0; i < PointsCount; i++)
	{
		if (DistToPoint(tank->x, tank->y, TargetPoints[i].x, TargetPoints[i].y) < MinDist)
		{
			MinDist = DistToPoint(tank->x, tank->y, TargetPoints[i].x, TargetPoints[i].y);
			CurrentTargetPoint = i;
		}
	}
	TargetPoint = TargetPoints[CurrentTargetPoint];
	PrevPoint = -1;
	PathLen = 0;
	PathPoint = 0;
	PathEnd = CurrentTargetPoint;
	srand(clock());
	int Point = rand() % Map1->PointsCount;
	SpawnPoint.x = TargetPoints[Point].x;
	SpawnPoint.y = TargetPoints[Point].y;
}

Box::Box(int x, int y):
	PointUnit(x, y, 0),
	GraphicUnit(HealImage, 99),
	SolidPhysicUnit(HealMask, OT_HEAL)
{}

Plane::Plane(int x, int y):
	GraphicUnit(PlaneImage, 93),
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

Explosive::Explosive(int x, int y):
	SolidUnit(ExplosiveMask, OT_EXPLOSIVE),
	PointUnit(x, y, 0)
{
	(new AnimatedGraphicUnit(ExplosiveImage, 80, false))->SetPoint(x, y, rand() % 360);
}
void Explosive::SteepProc()
{
	Delete();
}

void Draw()
{
	if (Contr1.Keys[VK_TAB] == 1)
	{
		DrawRect(-125, -200, 125, 200, true, 10, 0.75, 0.75, 0.75, 0.5);
		DrawRect(-125, -200, 125, 200, false, 9);
		TextStd->Draw("NickName", -123, 171, 9, false);
		DrawLine(15, -200, 15, 200, 9);
		TextStd->Draw("Kills", 17, 171, 9, false);
		DrawLine(65, -200, 65, 200, 9);
		TextStd->Draw("Deaths", 67, 171, 9, false);
		DrawLine(-125, 184, 125, 184, 9);
		int i = 0;
		ForEach(Player, player)
		{
			TextStd->Draw(player->NickName, -123, 155 - i * 16, 9, false);
			LPSTR str = new char[4];
			itoa(player->KillsCount, str, 10);
			TextStd->Draw(str, 22, 155 - i * 16, 9, false);
			itoa(player->DeathsCount, str, 10);
			TextStd->Draw(str, 67, 155 - i * 16, 9, false);
			delete[] str;
			DrawLine(-125, 168 - i * 16, 125, 168 - i * 16, 9);
			i++;
		}
	}
	char* str = new char[20];
	static int t;
	//TextStd->Draw(itoa(max(0, 5 - (clock() - Sync1->Time)), str, 10), -5, 0, 1, false);
	//TextStd->Draw(itoa(clock() - t, str, 10), -5, 20, 1, false);
	t = clock();
	delete[] str;
}
void GameInit()
{
	BeginDraw();
	TankBodyImage = new Image(L"Bitmaps\\Tank\\Body\\TankBody.bmp", L"Bitmaps\\Tank\\Body\\TankBody_Mask.bmp", 36, 24);
	TankTowerImage = new Image(L"Bitmaps\\Tank\\Tower\\TankTower.bmp", L"Bitmaps\\Tank\\Tower\\TankTower_Mask.bmp", 20, 16);
	BulletImage = new Image(L"Bitmaps\\Bullet\\Bullet.bmp", L"Bitmaps\\Bullet\\Bullet_Mask.bmp", 12, 4);
	MapImage = new Image(L"Bitmaps\\Map1\\Map1.bmp", L"Bitmaps\\Map1\\Map1_Draw_Mask.bmp", 400, 300);
	Fire1Image = new Image(L"Bitmaps\\Fire\\Fire1.bmp", L"Bitmaps\\Fire\\Fire1_Mask.bmp", 0, 16);
	Fire2Image = new Image(L"Bitmaps\\Fire\\Fire2.bmp", L"Bitmaps\\Fire\\Fire2_Mask.bmp", 0, 16);
	Fire3Image = new Image(L"Bitmaps\\Fire\\Fire3.bmp", L"Bitmaps\\Fire\\Fire3_Mask.bmp", 0, 24);
	FireImage = new AnimatedImage(3, Fire1Image, 6, Fire2Image, 6, Fire3Image, 8);
	Image* Hit1Image = new Image(L"Bitmaps\\Hit\\Hit.bmp", L"Bitmaps\\Hit\\Hit_Mask.bmp", 0, 16);
	HitImage = new AnimatedImage(1, Hit1Image, 6);
	HealImage = new Image(L"Bitmaps\\Heal\\Heal.bmp", L"Bitmaps\\Heal\\Heal_Mask.bmp", 24, 24);
	PlaneImage = new Image(L"Bitmaps\\Plane\\Plane.bmp", L"Bitmaps\\Plane\\Plane_Mask.bmp", 48, 48);
	Image* Explosive1Image = new Image(L"Bitmaps\\Explosive\\Explosive.bmp", L"Bitmaps\\Explosive\\Explosive_Draw_Mask.bmp", 32, 32);
	ExplosiveImage = new AnimatedImage(1, Explosive1Image, 6);

	TextStd = new Text(L"Bitmaps\\Font.bmp", 256, 32, 32);
	EndDraw();

	TankMask = new Mask(L"Bitmaps\\Tank\\Body\\TankBody_Mask.bmp", 36, 24);
	Map1Mask = new Mask(L"Bitmaps\\Map1\\Map1_Coll_Mask.bmp", 400, 300);
	BulletMask = new Mask(L"Bitmaps\\Bullet\\Bullet_Mask.bmp", 12, 4);
	HealMask = new Mask(L"Bitmaps\\Heal\\Heal_Mask.bmp", 24, 24);
	ExplosiveMask = new Mask(L"Bitmaps\\Explosive\\Explosive_Coll_Mask.bmp", 24, 24);

	Map1 = new Map(MapImage, L"Bitmaps\\Map1\\Map1_Coll_Mask.bmp", L"Map1_AIPoints.txt");
	//Player1 = new Player("Player", &Contr1, 320, 0, 90);
	//Bot* Bot1 = new Bot("Alpha", &Contr2, -320, 0, 270);
	//Bot* Bot2 = new Bot("Beta", &Contr3, 0, -240, 0);
	//Bot* Bot3 = new Bot("Gamma", &Contr4, 0, 100, 180);
	Player1 = new Player("Player", &Contr1, 0, -100, 90);
	Bot* Bot1 = new Bot("Alpha", -320, 0, 45);
	Bot* Bot2 = new Bot("Beta", 320, 0, 225);
	Bot* Bot3 = new Bot("Gamma", 0, 0, 90);
	Player1->Spawn();
	Bot1->Spawn();
	Bot2->Spawn();
	Bot3->Spawn();
	//Bot3->Spawn();

	/*Server server;
	server.IDAlloc(Bot1);
	server.IDAlloc(Bot2);
	server.IDAlloc(Player1);
	server.IDFree(1);*/
}