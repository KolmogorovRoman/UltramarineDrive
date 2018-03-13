#pragma once
#include <stdlib.h>
#include <windows.h>

#include "Management.h"
#include "Misc.h"
#include "Graphic.h"
#include "Physic.h"
#include "SolidPhysic.h"
#include "Solid.h"
#include "GameClient.h"

extern Controller Contr1;
extern Syncronizer* Sync1;
extern GameClient* Client;
extern void GameInit();
extern void PreSteepProc();
extern void PostSteepProc();
void Control();

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);