#include "Game.h"
#include <iostream>

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

TankController* Controller;

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

Bullet::Bullet():
	PointUnit(50, 0, 0),
	SolidUnit(BulletMask, OT_BULLET),
	GraphicUnit(BulletImage, 95)
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
	//if (ParPlayer->tank != Other)
	{
		Delete();
		(new AnimatedGraphicUnit(HitImage, 97, false))->SetPoint(x, y, 180 + angle);
	}
}
void Bullet::Recved()
{
	(new AnimatedGraphicUnit(FireImage, 97, false))
		->SetPoint(x, y, angle);
}
Bullet::~Bullet()
{}
void Serialize(Bullet& Bullet)
{
	Serialize(Bullet.x, Bullet.y, Bullet.angle);
}

Tank::Tank():
	PointUnit(0, 0, 0),
	GraphicUnit(TankBodyImage, 98),
	//SolidPhysicUnit(TankMask, OT_TANK),
	NickName("Noname"),
	HitPoints(5),
	FireReady(100),
	TowerAngle(0),
	Tower(new GraphicUnit(TankTowerImage, 96)),
	SolidUnit(TankMask, OT_TANK)
{}
void Tank::DrawProc()
{
	Tower->x = x;
	Tower->y = y;
	Tower->angle = angle + TowerAngle;
	TextStd->Draw(NickName, x, y + 50, 95, 1);
	DrawRect(x - 50, y - 50, x + 50, y - 45, 1, 95, 1, 1, 1);
	DrawRect(x - 50, y - 50, x + 100 * HitPoints / 5 - 50, y - 45, 1, 95, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
	DrawRect(x - 50, y - 50, x + 50, y - 45, false, 95);

	/*DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 20, Controller->Mouse.x + 15, Controller->Mouse.y - 15, 1, 95, 1, 1, 1);
	DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - 20, Controller->Mouse.x + 15, Controller->Mouse.y + 15, 1, 95, 1, 1, 1);
	DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y + 20, Controller->Mouse.x - 15, Controller->Mouse.y + 15, 1, 95, 1, 1, 1);
	DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y + 20, Controller->Mouse.x - 15, Controller->Mouse.y - 15, 1, 95, 1, 1, 1);
	if (FireReady <= 75)
	{
		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - FireReady * 15 / 75, Controller->Mouse.x - 15, Controller->Mouse.y + FireReady * 15 / 75, true, 95, 0.5, 0, 1);
		DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - FireReady * 15 / 75, Controller->Mouse.x + 15, Controller->Mouse.y + FireReady * 15 / 75, true, 95, 0.5, 0, 1);
	}
	else
	{
		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 15, Controller->Mouse.x - 15, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
		DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - 15, Controller->Mouse.x + 15, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 20, Controller->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Controller->Mouse.y - 15, true, 95, 0.5, 0, 1);
		DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - 20, Controller->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Controller->Mouse.y - 15, true, 95, 0.5, 0, 1);
		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y + 20, Controller->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
		DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y + 20, Controller->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
	}
	DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 20, Controller->Mouse.x + 20, Controller->Mouse.y + 20, false, 95);
	DrawRect(Controller->Mouse.x - 15, Controller->Mouse.y - 15, Controller->Mouse.x + 15, Controller->Mouse.y + 15, false, 95);

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
	DrawRect(Laser.x - 3, Laser.y - 1, Laser.x + 3, Laser.y + 2, true, 97, 1, 0, 0, 0.5);*/

	if (ClientID == Client->ID)
	{
		DrawRect(-400, -300, -300, -290, true, 96, 1, 1, 1);
		DrawRect(-400, -300, 100 * FireReady / 150 - 400, -290, true, 96, 0.5, 0, 1);
		DrawRect(-400, -300, -300, -290, false, 96);
		DrawRect(-400, -280, -300, -290, true, 96, 1, 1, 1);
		DrawRect(-400, -280, 100 * HitPoints / 5 - 400, -290, true, 96, 1 - HitPoints / 5.0, HitPoints / 5.0, 0);
		DrawRect(-400, -280, -300, -290, false, 96, 0, 0, 0);

		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 20, Controller->Mouse.x + 15, Controller->Mouse.y - 15, 1, 95, 1, 1, 1);
		DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - 20, Controller->Mouse.x + 15, Controller->Mouse.y + 15, 1, 95, 1, 1, 1);
		DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y + 20, Controller->Mouse.x - 15, Controller->Mouse.y + 15, 1, 95, 1, 1, 1);
		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y + 20, Controller->Mouse.x - 15, Controller->Mouse.y - 15, 1, 95, 1, 1, 1);
		if (FireReady <= 75)
		{
			DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - FireReady * 15 / 75, Controller->Mouse.x - 15, Controller->Mouse.y + FireReady * 15 / 75, true, 95, 0.5, 0, 1);
			DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - FireReady * 15 / 75, Controller->Mouse.x + 15, Controller->Mouse.y + FireReady * 15 / 75, true, 95, 0.5, 0, 1);
		}
		else
		{
			DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 15, Controller->Mouse.x - 15, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
			DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - 15, Controller->Mouse.x + 15, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
			DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 20, Controller->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Controller->Mouse.y - 15, true, 95, 0.5, 0, 1);
			DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y - 20, Controller->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Controller->Mouse.y - 15, true, 95, 0.5, 0, 1);
			DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y + 20, Controller->Mouse.x - 20 + (FireReady - 75) * 20 / 75, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
			DrawRect(Controller->Mouse.x + 20, Controller->Mouse.y + 20, Controller->Mouse.x + 20 - (FireReady - 75) * 20 / 75, Controller->Mouse.y + 15, true, 95, 0.5, 0, 1);
		}
		DrawRect(Controller->Mouse.x - 20, Controller->Mouse.y - 20, Controller->Mouse.x + 20, Controller->Mouse.y + 20, false, 95);
		DrawRect(Controller->Mouse.x - 15, Controller->Mouse.y - 15, Controller->Mouse.x + 15, Controller->Mouse.y + 15, false, 95);
		/*if (player->PlaneReady == 1)
		{
			PlaneImage->Draw(-350, -250, 96, 90, 0.5, 0.5, true);
		}*/
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
}
Tank::~Tank()
{
	Tower->Delete();
}
void Serialize(Tank& Tank)
{
	Serialize(Tank.x, Tank.y, Tank.angle, Tank.TowerAngle, Tank.HitPoints, Tank.NickName, Tank.FireReady, Tank.ClientID);
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

TankController::TankController(int KeyForwardCode, int KeyBackCode, int KeyRightCode, int KeyLeftCode, int KeyFireCode, int KeyPlaneCode):
	KeyForwardCode(KeyForwardCode),
	KeyBackCode(KeyBackCode),
	KeyRightCode(KeyRightCode),
	KeyLeftCode(KeyLeftCode),
	KeyFireCode(KeyFireCode),
	KeyPlaneCode(KeyPlaneCode)
{}
void TankController::SteepProc()
{
	if (Contr1.Keys[KeyForwardCode] == 1) KeyForwardPressed = true; else KeyForwardPressed = false;
	if (Contr1.Keys[KeyBackCode] == 1) KeyBackPressed = true; else KeyBackPressed = false;
	if (Contr1.Keys[KeyRightCode] == 1) KeyRightPressed = true; else KeyRightPressed = false;
	if (Contr1.Keys[KeyLeftCode] == 1) KeyLeftPressed = true; else KeyLeftPressed = false;
	if (Contr1.Keys[KeyFireCode] == 1) KeyFirePressed = true; else KeyFirePressed = false;
	if (Contr1.Keys[KeyPlaneCode] == 1) KeyPlanePressed = true; else KeyPlanePressed = false;
	Mouse = Contr1.Mouse;
}
void Serialize(TankController& TankController)
{
	Serialize(TankController.Mouse, TankController.KeyForwardPressed, TankController.KeyBackPressed, TankController.KeyRightPressed, TankController.KeyLeftPressed, TankController.KeyFirePressed, TankController.KeyPlanePressed);
}

void Draw()
{

}

void PreSteepProc()
{
	//Controller->Send();
}
void PostSteepProc()
{}

void GameInit()
{
	//RegisterTypes<Tank, Bullet>(ReservedCount);
	//RecvUnit<Tank>::MessageType = ReservedCount;
	//RecvUnit<Bullet>::MessageType = ReservedCount+1;
	//Client->RegisterRecvProc(RecvUnit<Tank>::MessageType, TankRecvProc);

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

	Controller = new TankController(87, 83, 68, 65, 1, 2);

	Client = new GameClient(INADDR_ANY);

	while (true)
	{
		char IP[16];
		WORD Port;
		char* NickName=new char[256];
		cout << "IP: ";
		cin >> IP;
		cout << "Port: ";
		cin >> Port;
		cout << "NickName: ";
		cin >> NickName;
		for (int i = 0; i < 3; i++)
		{
			cout << "Connecting... ";
			Client->Connect(IP, Port, 1s, NickName);
			if (Client->Connected) break;
			else cout << "Fail" << endl;
		}
		if (Client->Connected) break;
		cout << "Repeat? (Y/no Y) " << endl;
		char Answer;
		cin >> Answer;
		if (Answer == 'Y') continue;
		else exit(1);
	}
	cout << "Done" << endl;

	RegisterRecvTypes<Tank, Bullet>(ReservedCount);
	SendUnit<TankController>::Register(ReservedCount);
}