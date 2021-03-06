#include "Game.h"
#include <iostream>

HINSTANCE hInst;
RECT RectInScreen;
Console* GameConsole;

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
Image* SightImage;
AnimatedImage* PointerImage;

Layer* MapLayer;
Layer* TanksBodyLayer;
Layer* EffectsLayer;
Layer* TanksTowerLayer;
Layer* LabelsLayer;
Layer* SkyLayer;

Mask* TankMask;
Mask* Map1Mask;
Mask* BulletMask;
Mask* HealMask;
Mask* ExplosiveMask;

Player* WathedPlayer = NULL;

int GameMode = 0;
int CamMode = 0;
int DbgPointsMode = 0;
int DbgClockMode = 0;

int ClientTime = 0;
int ServerTime = 0;
int TimeMessageType;

TankController* tankControler;

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

Bullet::Bullet() :
	PointUnit(50, 0, 0),
	SolidUnit(BulletMask, OT_BULLET),
	GraphicUnit(BulletImage, TanksBodyLayer)
{}
void Bullet::SteepProc()
{
	x += 15 * dcos(angle);
	y += 15 * dsin(angle);
	//angle += rand() % 7 - 3;
	//if (InScreen() == false) ManagedUnit<Bullet>::Delete();
}
void Bullet::CollProc(SolidUnit* Other)
{
	//if (ParPlayer->tank != Other)
	{
		Delete();
		(new AnimatedGraphicUnit(HitImage, EffectsLayer, false))->SetPoint(x, y, 180 + angle);
	}
}
void Bullet::Recved()
{
	(new AnimatedGraphicUnit(FireImage, EffectsLayer, false))
		->SetPoint(x, y, angle);
}
void Bullet::Serialize()
{
	Serial(x, y, angle);
}
Bullet::~Bullet()
{}

Tank::Tank() :
	PointUnit(0, 0, 0),
	GraphicUnit(TankBodyImage, TanksBodyLayer),
	Contr(tankControler),
	//SolidPhysicUnit(TankMask, OT_TANK),
	NickName("Noname"),
	HitPoints(5),
	FireReady(100),
	TowerAngle(0),
	Tower(new GraphicUnit(TankTowerImage, TanksTowerLayer)),
	SolidUnit(TankMask, OT_TANK)
{}
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
		Vector Laser{ x, y };
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
	}
}
Tank::~Tank()
{
	Tower->Delete();
}
void Tank::Serialize()
{
	UINT PlayerID;
	Serial(x, y, angle, TowerAngle, HitPoints, NickName, FireReady, PlayerID, PlaneReady);
	if (Deserialazing)
	{
		player = (Player*)RecvUnit<BaseUnit>::Manager[PlayerID];
		/*ForEach(Tank, tank)
			if (tank != this && tank->NickName == NickName)
				cout << "!!!";*/
	}
}
stringstream& operator >> (stringstream& stream, Tank*& tank)
{
	string NickName;
	stream >> NickName;
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
	return stream;
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

Player::Player()
{}
void Player::Serialize()
{
	UINT TankID;
	Serial(NickName, KillsCount, DeathsCount, WinsCount, TankID);
	if (tank == NULL || tank->ID != TankID)
	{
		if (tank != NULL) tank->Delete();
		tank = (Tank*)RecvUnit<Tank>::Manager[TankID];
	}
}
stringstream& operator >> (stringstream& stream, Player*& player)
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


Box::Box() :
	PointUnit(0, 0, 0),
	GraphicUnit(HealImage, TanksBodyLayer),
	SolidPhysicUnit(HealMask, OT_HEAL)
{}
Box::Box(int x, int y) :
	PointUnit(x, y, 0),
	GraphicUnit(HealImage, TanksBodyLayer),
	SolidPhysicUnit(HealMask, OT_HEAL)
{}
void Box::Serialize()
{
	Serial(x, y);
}
void Box::Recved()
{}

Plane::Plane() :
	GraphicUnit(PlaneImage, SkyLayer),
	Time(0)
{}
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
void Plane::Serialize()
{
	Serial(x, y, angle, TurnSpeed);
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
void TankController::Serialize()
{
	BYTE Keys = 0;
	if (Serialazing)
	{
		if (KeyForwardPressed) Keys |= 0b00100000;
		if (KeyBackPressed)    Keys |= 0b00010000;
		if (KeyRightPressed)   Keys |= 0b00001000;
		if (KeyLeftPressed)    Keys |= 0b00000100;
		if (KeyFirePressed)    Keys |= 0b00000010;
		if (KeyPlanePressed)   Keys |= 0b00000001;
	}
	Serial(Mouse, Keys);
	if (Deserialazing)
	{
		KeyForwardPressed = Keys & 0b00100000;
		KeyBackPressed = Keys & 0b00010000;
		KeyRightPressed = Keys & 0b00001000;
		KeyLeftPressed = Keys & 0b00000100;
		KeyFirePressed = Keys & 0b00000010;
		KeyPlanePressed = Keys & 0b00000001;
	}
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
	}
	if (DbgClockMode == 2 || (DbgClockMode == 1 && Contr1.Keys[VK_TAB] >= 1))
	{
		int RealTime = clock() * 60 / 1000;
		DrawLine(-250, -50, -250 + 50 * dcos(RealTime), -50 + 50 * dsin(RealTime), InterfaceLayer, 0, 0, 1);
		DrawLine(-250, -50, -250 + 50 * dcos(ClientTime), -50 + 50 * dsin(ClientTime), InterfaceLayer, 0, 1, 0);
		DrawLine(-250, -50, -250 + 50 * dcos(ServerTime), -50 + 50 * dsin(ServerTime), InterfaceLayer, 1, 0, 0);

		DrawLine(-250, 50, -250 + 50 * dcos(ClientTime - ServerTime), 50 + 50 * dsin(ClientTime - ServerTime), InterfaceLayer, 1, 1, 0);
		DrawLine(-250, 50, -250 + 50 * dcos(ClientTime - RealTime), 50 + 50 * dsin(ClientTime - RealTime), InterfaceLayer, 0, 1, 1);
		DrawLine(-250, 50, -250 + 50 * dcos(ServerTime - RealTime), 50 + 50 * dsin(ServerTime - RealTime), InterfaceLayer, 1, 0, 1);
	}
	if (DbgPointsMode == 2 || (DbgPointsMode == 1 && Contr1.Keys[VK_TAB] >= 1))
		for (int i = 0; i < Map1->PointsCount; i++)
		{
			PointerImage->Draw(0, Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, LabelsLayer, 0, 0.5, 0.5);
			for (int j = 0; j < Map1->NearsCounts[i]; j++)
				if (Map1->Net[i][j] > i)
					DrawLine(Map1->TargetPoints[i].x, Map1->TargetPoints[i].y, Map1->TargetPoints[Map1->Net[i][j]].x, Map1->TargetPoints[Map1->Net[i][j]].y, LabelsLayer, 1, 0, 1);
		}
}

void SteepProc()
{
	ClientTime++;
	tankControler->Check();
}
void PostSteepProc()
{}

void InitConsole()
{
	GameConsole->RegisterCommand<Player*>("wath", [](Player* player)
	{
		WathedPlayer = player;
	});
	GameConsole->RegisterCommand<int>("cammode", [](int mode)
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
	});
	GameConsole->RegisterCommand<double>("scale", [](double scale)
	{
		MainCamera.Scale = scale;
	});
	GameConsole->RegisterCommand<bool>("fullscreen", [](bool fulscreen)
	{
		if (fulscreen)
			NeedFullscreen = true;
		else
			NeedWindowed = true;
	});
	GameConsole->RegisterCommand<string>("map", [](string map)
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
	});
	GameConsole->RegisterCommand<int>("exit", [](int)
	{
		Closed = true;
	});
	GameConsole->RegisterCommand<int>("dbgpoints", [&](int mode)
	{
		if (mode >= 0 && mode <= 2)
			DbgPointsMode = mode;
		else throw "DbgPointsMode " + to_string(mode) + " is not exist";
	});
	GameConsole->RegisterCommand<int>("dbgclock", [&](int mode)
	{
		if (mode >= 0 && mode <= 2)
			DbgClockMode = mode;
		else throw "DbgClockMode " + to_string(mode) + " is not exist";
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
	GameConsole = new Console(MainFont);
	InitConsole();

	tankControler = new TankController(&Contr1, 'W', 'S', 'D', 'A', 1, 2);

	Map1 = new Map("Map4");

	Client::Init();
	Client::RegisterRecvProc(RecvUnit<Tank>::MessageType, (function<bool(Client::MessageInfo*)>)[](Client::MessageInfo* Message)
	{
		return true;
	});
	Client::StartRecv();

	while (true)
	{
		char IP[16];
		WORD Port;
		string NickName;
		cout << "IP: ";
		cin >> IP;
		cout << "Port: ";
		cin >> Port;
		cout << "NickName: ";
		cin >> NickName;
		for (int i = 0; i < 3; i++)
		{
			cout << "Connecting... ";
			Client::Connect(IP, Port, 1s, NickName);
			//Client::Connect("127.0.0.1", 60004, 1s, (char*)"Player");
			if (Client::Connected) break;
			else cout << "Fail" << endl;
		}
		if (Client::Connected) break;
		cout << "Repeat? (Y/no Y) " << endl;
		char Answer;
		cin >> Answer;
		if (Answer == 'Y' || Answer == 'y') continue;
		else exit(1);
	}
	cout << "Done" << endl;
	HWND ConsoleHWND = GetConsoleWindow();
	ShowWindow(ConsoleHWND, SW_HIDE);

	RegisterRecvTypes<Player, Tank, Bullet, Box, Plane>(NextRecvType);
	SendUnit<TankController>::Register(NextSendType);
	TimeMessageType = NextRecvType++;
	Client::RegisterRecvProc(TimeMessageType, [](Client::MessageInfo* Message)
	{
		Message->Array->Deserialize(ServerTime);
		return false;
	});
}

void SetCamera()
{
	if (WathedPlayer == NULL || WathedPlayer->tank == NULL)
		return;
	if (CamMode == 1)
		MainCamera.SetPoint((WathedPlayer->tank->x * 2 + Contr1.Mouse.x) / 3, (WathedPlayer->tank->y * 2 + Contr1.Mouse.y) / 3, 0);
	//MainCamera.SetPoint(WathedPlayer->tank->x, WathedPlayer->tank->y, 0);
	if (CamMode == 2)
		MainCamera.SetPoint((WathedPlayer->tank->x * 2 + Contr1.Mouse.x) / 3, (WathedPlayer->tank->y * 2 + Contr1.Mouse.y) / 3, -WathedPlayer->tank->angle + 90);
}
void CameraChanged()
{
	if (GameConsole != NULL) GameConsole->UpdateInputRect();
}