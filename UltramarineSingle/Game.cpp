#include "Game.h"

HINSTANCE hInst;
RECT RectInScreen;

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
Image* ApacheBodyImage;
Image* ApacheHelixImage;
Image* AtgmImage;
AnimatedImage* AtgmTrackImage;
Image* ApacheSightImage;

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

Bullet::Bullet(double x, double y, double angle, Player* ParPlayer):
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
	if (x<-Map1->image->RealWidth / 2 || x>Map1->image->RealWidth / 2 ||
		y<-Map1->image->RealHeight / 2 || y>Map1->image->RealHeight / 2)
		Delete();
}
void Bullet::CollProc(SolidUnit* Other)
{
	if (ParPlayer == NULL || ParPlayer->tank != Other)
	{
		Delete();
		(new AnimatedGraphicUnit(HitImage, EffectsLayer, false))->SetPoint(x, y, 180 + angle);
	}
}
Bullet::~Bullet()
{}

Tank::Tank(PointUnit Point, Player* player, TankController* Contr, string NickName):
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
		if (Other->Type == OT_BULLET && ((Bullet*)Other)->ParPlayer != NULL && ((Bullet*)Other)->ParPlayer != player)
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
	//NickName
	MainFont->Draw(NickName, x, y + 50, LabelsLayer, true, true);
	//HitPoints
	DrawRect(x - 50, y - 50, x + 50, y - 45, 1, LabelsLayer, 1, 1, 1);
	DrawRect(x - 50, y - 50, x + 100 * HitPoints / 5 - 50, y - 45, 1, LabelsLayer, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
	DrawRect(x - 50, y - 50, x + 50, y - 45, false, LabelsLayer);

	if (player == WathedPlayer)
	{
		//FireReady
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 1, 1, 1);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2, 100 * FireReady / 150 - MainCamera.Width / 2, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 0.5, 0, 1);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, false, InterfaceLayer);
		//HitPoints
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2 + 20, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 1, 1, 1);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2 + 20, 100 * HitPoints / 5 - MainCamera.Width / 2, -MainCamera.Height / 2 + 10, true, InterfaceLayer, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
		DrawRect(-MainCamera.Width / 2, -MainCamera.Height / 2 + 20, -MainCamera.Width / 2 + 100, -MainCamera.Height / 2 + 10, false, InterfaceLayer, 0, 0, 0);
		//Cursor
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
		//Sight
		double CurrLaserLen = sqrt(Map1->image->Width*Map1->image->Width + Map1->image->Height*Map1->image->Height);
		double Sin = dsin(angle + TowerAngle);
		double Cos = dcos(angle + TowerAngle);
		Vector Laser{ x + CurrLaserLen*Cos, y + CurrLaserLen*Sin };
		ForEach(SolidUnit, Unit)
		{
			if ((BaseUnit*)Unit == (BaseUnit*)this) continue;
			if (!CollTable[OT_BULLET][Unit->Type]) continue;
			Vector CurrLaser = v(x, y);
			for (int l = 0; l < CurrLaserLen; l++)
			{
				if (Unit->PixelCheck(CurrLaser.x, CurrLaser.y))
				{
					Laser = CurrLaser;
					CurrLaserLen = CurrLaser.Len();
					break;
				}
				CurrLaser.x += Cos;
				CurrLaser.y += Sin;
			}
		}
		//DrawLine(x, y, Laser.x, Laser.y, 97, 1, 0, 0, 0.7);
		SightImage->Draw(Laser.x, Laser.y, LabelsLayer, angle + TowerAngle);
		//Plane
		if (player->PlaneReady)
		{
			PlaneImage->Draw(-MainCamera.Width / 2 + 50, -MainCamera.Height / 2 + 50, InterfaceLayer, 90, 0.5, 0.5, true);
		}
	}
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
void Parse(Console* console, istream & stream, Tank*& tank)
{
	string* NickName = new string();
	stream >> *NickName;
	tank = (Tank*)NickName;
}
void Calculate(Tank*& tank)
{
	string NickName = *((string*)tank);
	tank = NULL;
	ForEach(Tank, t)
	{
		if (t->NickName == NickName)
		{
			tank = t;
			break;
		}
	}
	if (tank == NULL) throw "Tank not found"s;
}

Map::Map(string MapDir):
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
		/*while (!Script.eof())
		{
			string Command;
			std::getline(Script, Command);
			GameConsole->Execute(Command);
		}*/
		GameConsole->Execute(Script);
	}
}

Player::Player(string NickName, TankController* Contr):
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
istream& operator >> (istream& stream, Player*& player)
{
	string NickName;
	stream >> NickName;
	player = NULL;
	ForEach(Player, p)
	{
		if (p->NickName == NickName)
		{
			player = p;
			break;
		}
	}
	if (player == NULL) throw "Player not found"s;
	return stream;
}

Bot::Bot(string NickName):
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
		if (tank == Unit || Unit->player->Team == Team && Team != 0) continue;
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
			if (tank == Unit || Unit->player->Team == Team && Team != 0) continue;
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

Box::Box(int x, int y):
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

Plane::Plane(int x, int y):
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

Explosive::Explosive(int x, int y):
	SolidUnit(ExplosiveMask, OT_EXPLOSIVE),
	PointUnit(x, y, 0)
{
	(new AnimatedGraphicUnit(ExplosiveImage, EffectsLayer, false))->SetPoint(x, y, rand() % 360);
}
void Explosive::SteepProc()
{
	Delete();
}

TankController::TankController():
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

Apache::Apache(PointUnit Point, Controller* Contr):
	PointUnit(Point),
	Contr(Contr),
	GraphicUnit(ApacheBodyImage, LabelsLayer)
{
	Helix = new GraphicUnit(ApacheHelixImage, LabelsLayer);
	Helix->SetPoint(x, y, angle);
}
void Apache::SteepProc()
{
	if (FireReady < 5)
		FireReady++;
	double MinDist;
	MinDist = 150;
	Target = NULL;
	ForEach(Player, p)
	{
		if (p->tank == NULL) continue;
		double d = DistToPoint(Contr->Mouse.x, Contr->Mouse.y, p->tank->x, p->tank->y);
		if (d < MinDist)
		{
			Target = p;
			MinDist = d;
		}
	}
	double Angle = AnglesDiff(angle, AngleToPoint(x, y, Contr->Mouse.x, Contr->Mouse.y));
	if (Angle > Velosity*HelixVelosity / 10)
	{
		angle -= Velosity*HelixVelosity / 10;
	}
	else if (Angle < -Velosity*HelixVelosity / 10)
	{
		angle += Velosity*HelixVelosity / 10;
	}
	else
	{
		angle = AngleToPoint(x, y, Contr->Mouse.x, Contr->Mouse.y);
	}
	if (Contr->Keys[VK_UP])
	{
		//x += ForwardSpeed*dcos(angle)*HelixVelosity / 10;
		//y += ForwardSpeed*dsin(angle)*HelixVelosity / 10;
		y += ForwardSpeed*HelixVelosity / 10;
	}
	if (Contr->Keys[VK_DOWN])
	{
		//x -= ForwardSpeed*dcos(angle)*HelixVelosity / 10;
		//y -= ForwardSpeed*dsin(angle)*HelixVelosity / 10;
		y -= ForwardSpeed*HelixVelosity / 10;
	}
	if (Contr->Keys[VK_RIGHT])
	{
		//x += ForwardSpeed*dcos(angle - 90)*HelixVelosity / 10;
		//y += ForwardSpeed*dsin(angle - 90)*HelixVelosity / 10;
		x += ForwardSpeed*HelixVelosity / 10;
	}
	if (Contr->Keys[VK_LEFT])
	{
		//x += ForwardSpeed*dcos(angle + 90)*HelixVelosity / 10;
		//y += ForwardSpeed*dsin(angle + 90)*HelixVelosity / 10;
		x -= ForwardSpeed*HelixVelosity / 10;
	}
	if (Contr->Keys[VK_SHIFT] && HelixVelosity < 20)
	{
		if (HelixVelosity != 10.0)
			HelixVelosity += 0.1;
		else if (Contr->Keys[VK_SHIFT] > 5)
			HelixVelosity += 0.1;
	}
	if (Contr->Keys[VK_CONTROL] && HelixVelosity > 0)
	{
		HelixVelosity -= 0.1;
	}
	if (Contr->Keys[1] > 0 && FireReady >= 5)
	{
		double d = DistToPoint(x, y, Contr->Mouse.x, Contr->Mouse.y);
		if (d < 200) d = 200;
		double a = AngleToPoint(x + dcos(angle + b) * 30, y + dsin(angle + b) * 30, x + d*dcos(angle), y + d*dsin(angle));
		new Bullet(x + dcos(angle + b) * 30, y + dsin(angle + b) * 30, a, NULL);
		FireReady = 0;
		b = -b;
	}
	if (Contr->Keys[2] == 1 && Target != NULL)
	{
		new Atgm(Target, x + dcos(angle + a) * 40, y + dsin(angle + a) * 40, angle);
		a = -a;
	}

}
void Apache::DrawProc()
{
	Helix->SetPoint(x, y, Helix->angle + HelixVelosity);
	Helix->Draw();
	MainFont->Draw(to_string(HelixVelosity), MainCamera.Width / 2 - 50, -MainCamera.Height / 2 + MainFont->Common.LineHeight, InterfaceLayer);
	ApacheSightImage->Draw(Contr->Mouse.x, Contr->Mouse.y, LabelsLayer, 0);
	if (Target != NULL && Target->tank != NULL)
	{
		double x = Target->tank->x;
		double y = Target->tank->y;
		double w = Target->tank->image->RealWidth / 2 + 5;
		double h = Target->tank->image->RealHeight / 2 + 5;
		POINT p = TurnedRectSize(w, h, Target->tank->angle);
		w = p.x;
		h = p.y;
		DrawLine(x - w, y + h, x - 10, y + h, LabelsLayer);
		DrawLine(x + 10, y + h, x + w, y + h, LabelsLayer);
		DrawLine(x + w, y + h, x + w, y + 10, LabelsLayer);
		DrawLine(x + w, y - 10, x + w, y - h, LabelsLayer);
		DrawLine(x + w, y - h, x + 10, y - h, LabelsLayer);
		DrawLine(x - 10, y - h, x - w, y - h, LabelsLayer);
		DrawLine(x - w, y - h, x - w, y - 10, LabelsLayer);
		DrawLine(x - w, y + 10, x - w, y + h, LabelsLayer);
	}

	/*double CurrLaserLen = sqrt(Map1->image->Width*Map1->image->Width + Map1->image->Height*Map1->image->Height);
	double Sin = dsin(angle);
	double Cos = dcos(angle);
	Vector Laser{ x + CurrLaserLen*Cos + dcos(angle + 90) * 30, y + CurrLaserLen*Sin + dsin(angle + 90) * 30 };
	ForEach(SolidUnit, Unit)
	{
		if ((BaseUnit*)Unit == (BaseUnit*)this) continue;
		if (!CollTable[OT_BULLET][Unit->Type]) continue;
		Vector CurrLaser = v(x + dcos(angle + 90) * 30, y + dsin(angle + 90) * 30);
		for (int l = 0; l < CurrLaserLen; l++)
		{
			if (Unit->PixelCheck(CurrLaser.x, CurrLaser.y))
			{
				Laser = CurrLaser;
				CurrLaserLen = CurrLaser.Len();
				break;
			}
			CurrLaser.x += Cos;
			CurrLaser.y += Sin;
		}
	}
	SightImage->Draw(Laser.x, Laser.y, LabelsLayer, angle);

	CurrLaserLen = sqrt(Map1->image->Width*Map1->image->Width + Map1->image->Height*Map1->image->Height);
	Sin = dsin(angle);
	Cos = dcos(angle);
	Laser.x = x + CurrLaserLen*Cos + dcos(angle - 90) * 30;
	Laser.y = y + CurrLaserLen*Sin + dsin(angle - 90) * 30;
	ForEach(SolidUnit, Unit)
	{
		if ((BaseUnit*)Unit == (BaseUnit*)this) continue;
		if (!CollTable[OT_BULLET][Unit->Type]) continue;
		Vector CurrLaser = v(x - dcos(angle + 90) * 30, y - dsin(angle + 90) * 30);
		for (int l = 0; l < CurrLaserLen; l++)
		{
			if (Unit->PixelCheck(CurrLaser.x, CurrLaser.y))
			{
				Laser = CurrLaser;
				CurrLaserLen = CurrLaser.Len();
				break;
			}
			CurrLaser.x += Cos;
			CurrLaser.y += Sin;
		}
	}
	SightImage->Draw(Laser.x, Laser.y, LabelsLayer, angle);*/

}
Apache::~Apache()
{
	Helix->Delete();
}

Atgm::Atgm(Player* Target, double x, double y, double angle):
	Target(Target),
	PointUnit(x, y, angle),
	GraphicUnit(AtgmImage, LabelsLayer)
{}
void Atgm::SteepProc()
{
	Time++;
	/*if (Time >= 500)
	{
		Delete();
		return;
	}*/
	if (Speed < 5) Speed += 0.2;
	if (Target != NULL && Target->tank != NULL)
	{
		if (DistToPoint(x, y, Target->tank->x, Target->tank->y) < 25)
		{
			new Explosive(x, y);
			Delete();
		}
		double Angle = AnglesDiff(angle, AngleToPoint(x, y, Target->tank->x, Target->tank->y));
		if (Angle > RotSpeed)
		{
			angle -= RotSpeed;
			if (Speed > 5) Speed -= 0.1;
		}
		else if (Angle < -RotSpeed)
		{
			angle += RotSpeed;
			if (Speed > 5) Speed -= 0.1;
		}
		else
		{
			angle = AngleToPoint(x, y, Target->tank->x, Target->tank->y);
			if (Speed < 10) Speed += 0.2;
			else if (Speed < 15) Speed += 0.1;
		}
	}
	x += Speed * dcos(angle);
	y += Speed * dsin(angle);
	(new AnimatedGraphicUnit(AtgmTrackImage, EffectsLayer, false))->SetPoint(x, y, angle + rand() % 6 - 3);
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
			MainFont->Draw("NickName", -125, 200, InterfaceLayer);
			DrawLine(15, -200, 15, 200, InterfaceLayer);
			MainFont->Draw("Kills", 15, 200, InterfaceLayer);
			DrawLine(65, -200, 65, 200, InterfaceLayer);
			MainFont->Draw("Deaths", 65, 200, InterfaceLayer);
			int y = 200 - MainFont->Common.LineHeight;
			DrawLine(-125, y, 125, y, InterfaceLayer);
			ForEach(Player, player)
			{
				if (player->tank == NULL)
					MainFont->Draw("*DEAD* " + player->NickName, -125, y, InterfaceLayer);
				else
					MainFont->Draw(player->NickName, -125, y, InterfaceLayer);
				MainFont->Draw(to_string(player->KillsCount), 15, y, InterfaceLayer);
				MainFont->Draw(to_string(player->DeathsCount), 65, y, InterfaceLayer);
				y -= MainFont->Common.LineHeight;
				DrawLine(-125, y, 125, y, InterfaceLayer);
			}
		}
		if (GameMode == 1)
		{
			DrawRect(-150, -200, 150, 200, true, InterfaceBackLayer, 0.75, 0.75, 0.75, 0.5);
			DrawRect(-150, -200, 150, 200, false, InterfaceLayer);
			MainFont->Draw("NickName", -150, 171, InterfaceLayer);
			DrawLine(-10, -200, -10, 200, InterfaceLayer);
			MainFont->Draw("Wins", -10, 171, InterfaceLayer);
			DrawLine(40, -200, 40, 200, InterfaceLayer);
			MainFont->Draw("Kills", 40, 171, InterfaceLayer);
			DrawLine(90, -200, 90, 200, InterfaceLayer);
			MainFont->Draw("Deaths", 90, 171, InterfaceLayer);
			int y = 200 - MainFont->Common.LineHeight;
			DrawLine(-150, y, 150, y, InterfaceLayer);
			ForEach(Player, player)
			{
				if (player->tank == NULL)
					MainFont->Draw("*DEAD* " + player->NickName, -150, y, InterfaceLayer);
				else
					MainFont->Draw(player->NickName, -150, y, InterfaceLayer);
				MainFont->Draw(to_string(player->WinsCount), -10, y, InterfaceLayer);
				MainFont->Draw(to_string(player->KillsCount), 40, y, InterfaceLayer);
				MainFont->Draw(to_string(player->DeathsCount), 90, y, InterfaceLayer);
				y -= MainFont->Common.LineHeight;
				DrawLine(-150, y, 150, y, InterfaceLayer);
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

struct BlockExpression:
	public Expression
{
	BlockExpression(BaseCommand* command):
		Expression(command)
	{}
	list<Expression*> Expressions;
	string Execute() override;
};

string BlockExpression::Execute()
{
	for (Expression* expr : Expressions)
	{
		expr->Execute();
	}
	return ""s;
}

struct BlockCommand:
	public BaseCommand
{
	BlockCommand():
		BaseCommand("begin")
	{}
	Expression* GetExpression(Console* console, istream& Args) override;
};

Expression* BlockCommand::GetExpression(Console* console, istream& s)
{
	BlockExpression* expr = new BlockExpression(this);
	while (s)
	{
		Expression* e = console->GetExpression(s);
		if (e->basecommand->Name == "end") break;
		expr->Expressions.push_back(e);
	}
	return expr;
}

void InitConsole()
{
	BeginCommands()
	{
		GameConsole->RegisterCommand(new BlockCommand());
	}
	AddCommand(void, "heal", Arg<Tank*> tank)
	{
		tank.SetDefault(Player1->tank);
		tank->HitPoints = 5;
	}
	AddCommand(void, "kill", Arg<Tank*> tank)
	{
		if (*tank != NULL) tank->Die();
	}
	AddCommand(void, "wath", Arg<Player*> player)
	{
		player.SetDefault(Player1);
		WathedPlayer = *player;
	}
	AddCommand(void, "plane", Arg<Player*> player)
	{
		player.SetDefault(Player1);
		(*player)->PlaneReady = true;
	}
	AddCommand(void, "mode", int mode)
	{
		if (mode == 0 || mode == 1)
			GameMode = mode;
		else throw "Mode "s + to_string(mode) + " is not exist"s;
	}
	AddCommand(void, "cammode", int mode)
	{
		if (mode == 0)
		{
			CamMode = 0;
			MainCamera.SetPoint(0, 0, 0);
		}
		else if (mode == 1 || mode == 2)
		{
			CamMode = mode;
		}
		else throw "Cammode "s + to_string(mode) + " is not exist"s;
	}
	AddCommand(void, "scale", double scale)
	{
		MainCamera.Scale = scale;
	}
	AddCommand(void, "fullscreen", bool fulscreen)
	{
		if (fulscreen)
			NeedFullscreen = true;
		else
			NeedWindowed = true;
	}
	AddCommand(void, "map", string map)
	{
		if (access(("Resources\\" + map).c_str(), 0) != 0)
			throw "Directry Resources\\" + map + " not found";
		if (access(("Resources\\" + map + "\\.bmp").c_str(), 0) != 0)
			throw"File Resources\\" + map + "\\.bmp not found";
		if (access(("Resources\\" + map + "\\Coll_Mask.bmp").c_str(), 0) != 0)
			throw"File Resources\\" + map + "\\Coll_Mask.bmp not found";
		if (access(("Resources\\" + map + "\\Points.txt").c_str(), 0) != 0)
			throw "File Resources\\" + map + "\\Points.txt not found";
		ForEach(Player, player)
		{
			if (player->tank != NULL)
				player->tank->Delete();
			player->Delete();
			player->Active = false;
		}
		Map1->Delete();
		Map1 = new Map(map);
		Player1 = new Player("Player", Player1->Contr);
		WathedPlayer = Player1;
	}
	AddCommand(void, "newbot", string Name, Arg<int> Team)
	{
		Team.SetDefault(0);
		(new Bot(Name))->Team = *Team;
	}
	AddCommand(void, "kick", Player* player)
	{
		if (player->tank != NULL)
			player->tank->Delete();
		player->Delete();
	}
	AddCommand(void, "exit")
	{
		Closed = true;
	}
	AddCommand(void, "dbgpoints", int mode)
	{
		if (mode >= 0 && mode <= 2)
			DbgPointsMode = mode;
		else throw "DbgPointsMode " + to_string(mode) + " is not exist";
	}
	AddCommand(void, "dbgclock", int mode)
	{
		if (mode >= 0 && mode <= 2)
			DbgClockMode = mode;
		else throw "DbgClockMode " + to_string(mode) + " is not exist";
	}
	AddCommand(void, "apache")
	{
		new Apache(PointUnit(0, 0, 0), &Contr1);
	}
	AddCommand(void, "loadimages")
	{
		LoadImages();
	}
	AddCommand(bool, "o", int i)
	{
		return i % 2 == 1;
	}
	AddCommand(void, "if", Expression* cond, Expression* expr)
	{
		if (cond->Execute() == "true"s) expr->Execute();
	}
	AddCommand(void, "for", int n, Expression* expr)
	{
		for (int i = 0; i < n; i++)
			expr->Execute();
	}
	/*AddCommand(void, "begin", stringstream* s)
	{
		list<Expression*> Expressions;
		while (*s)
		{
			Expression* expr = GameConsole->GetExpression(*s);
			if (expr->basecommand->Name == "end") break;
			Expressions.push_back(expr);
		}
		for (Expression* e : Expressions)
		{
			e->Execute();
		}
	}*/
	AddCommand(void, "end")
	{}
	AddCommand(void, "alias", string name, Expression* expr)
	{
		GameConsole->RegisterCommand(name, (function<string()>)[&, expr]()->string
		{
			return expr->Execute();
		});
	}
	EndCommands()
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
	//TankBodyImage = new Image("Resources\\Unused\\Humwee\\Body\\.bmp");
	//TankTowerImage = new Image("Resources\\Unused\\Humwee\\Tower\\.bmp");
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
	//TextStd = new Text(L"Resources\\Font.bmp", 256, 32, 32);
	ApacheBodyImage = new Image("Resources\\Apache\\Body\\.bmp");
	ApacheHelixImage = new Image("Resources\\Apache\\Helix\\.bmp");
	AtgmImage = new Image("Resources\\Apache\\Atgm\\.bmp");
	Image* AtgmTrack1Image = new Image("Resources\\Apache\\ATGM\\Track1.bmp");
	Image* AtgmTrack2Image = new Image("Resources\\Apache\\ATGM\\Track2.bmp");
	Image* AtgmTrack3Image = new Image("Resources\\Apache\\ATGM\\Track3.bmp");
	AtgmTrackImage = new AnimatedImage(3, AtgmTrack1Image, 1, AtgmTrack2Image, 4, AtgmTrack3Image, 5);
	ApacheSightImage = new Image("Resources\\Apache\\Sight\\.bmp");
}
void LoadMasks()
{
	TankMask = new Mask("Resources\\Tank\\Body\\Mask.bmp", 36, 24);
	//TankMask = new Mask("Resources\\Unused\\Humwee\\Body\\Mask.bmp", 36, 24);
	BulletMask = new Mask("Resources\\Bullet\\Mask.bmp", 12, 4);
	HealMask = new Mask("Resources\\Heal\\Mask.bmp", 24, 24);
	ExplosiveMask = new Mask("Resources\\Explosive\\Coll_Mask.bmp", 40, 40);
}
void GameInit()
{
	//new Chat(MainFont);
	InitConsole();

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
void CameraChanged()
{
	if (GameConsole != NULL) GameConsole->UpdateInputRect();
}