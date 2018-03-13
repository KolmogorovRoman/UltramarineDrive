#pragma once
#include <stdlib.h>
#include <windows.h>

#include "Management.h"
#include "Misc.h"
#include "Graphic.h"
#include "Physic.h"
#include "SolidPhysic.h"
#include "Solid.h"
#include "GameServer.h"

extern Controller Contr1;
extern Syncronizer* Sync1;
extern GameServer* Server;
extern void GameInit();
void Control();

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);