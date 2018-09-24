#pragma once
#include <stdlib.h>
#include <windows.h>

#include "Management.h"
#include "Misc.h"
#include "Graphic.h"
#include "Font.h"
#include "Physic.h"
#include "SolidPhysic.h"
#include "Solid.h"
#include "Client.h"
#include "Console.h"

extern Controller Contr1;
extern Syncronizer* Sync1;
extern void GameInit();
extern void SteepProc();
extern void PostSteepProc();

void Control();

extern Layer* LowestLayer;
extern Layer* InterfaceLayer;
extern Layer* InterfaceBackLayer;

extern Font* MainFont;

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);