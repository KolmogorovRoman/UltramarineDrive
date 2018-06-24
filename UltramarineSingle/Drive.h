#pragma once
#include <stdlib.h>
#include <windows.h>
#include <io.h>

#include "Management.h"
#include "Misc.h"
#include "Graphic.h"
#include "Physic.h"
#include "SolidPhysic.h"
#include "Solid.h"
#include "Console.h"

extern Controller Contr1;
extern Syncronizer* Sync1;
extern void GameInit();
extern void SteepProc();

extern Layer* LowestLayer;
extern Layer* InterfaceLayer;
extern Layer* InterfaceBackLayer;

void Control();

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);