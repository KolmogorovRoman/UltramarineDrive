#pragma once
#include <stdlib.h>
#include <windows.h>
#include <io.h>

#include "Management.h"
#include "Misc.h"
#include "Graphic.h"
#include "Font.h"
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

extern Font* MainFont;
extern Console* GameConsole;

#define BeginCommands() GameConsole = new Console(MainFont, InterfaceBackLayer, InterfaceLayer, Contr1); GameConsole->ForMagic([]()
#define RegisterCommand(Name, ...) ); GameConsole->RegisterCommand(Name, (function<void(__VA_ARGS__)>)[&](__VA_ARGS__)
#define EndCommands() );

void Control();

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);