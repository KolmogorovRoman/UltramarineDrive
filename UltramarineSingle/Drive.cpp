#include "Drive.h"

Controller Contr1;
Syncronizer* Sync1;

void Control()
{
	GetCursorPos(&(Contr1.Mouse));
	ScreenToClient(hWnd, (&Contr1.Mouse));
	Contr1.Mouse.y = WindowHeigth - Contr1.Mouse.y;
	Contr1.Mouse.x += (xGlobal - WindowWidth / 2);
	Contr1.Mouse.y += (yGlobal - WindowHeigth / 2);
	Contr1.Mouse.x = Contr1.Mouse.x / GlobalScale;
	Contr1.Mouse.y = Contr1.Mouse.y / GlobalScale;
	for (int i = 0; i < 256; i += 1)
	{
		if (GetKeyState(i) & 0b10000000)
			Contr1.Keys[i] = 1;
		else Contr1.Keys[i] = 0;
	}
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	InitGraphic(hinst, nCmdShow, L"Single");
	Sync1 = new Syncronizer();
	GameInit();
	while (true)
	{
		auto Time = std::chrono::high_resolution_clock::now();
		BaseUnit::ActivateAll();
		Control();
		BeginDraw();
		AllCollCheck();
		CalcAllSolidPhysicUnits();
		CallEach(PhysicUnit) Calc();
		CallEach(SteepProcedUnit) SteepProc();
		CallEach(GraphicUnit) DrawProc();
		CallEach(GraphicUnit) Draw();
		CallEach(AnimatedGraphicUnit) DrawProc();
		CallEach(AnimatedGraphicUnit) Draw();
		Draw();
		EndDraw();
		BaseUnit::DeleteAllThatNeedDelete();
		if (Closed) return 0;
		bool OutTime = Contr1.Keys[VK_TAB];
		if (OutTime) cout << (std::chrono::high_resolution_clock::now() - Time).count() / 1'000'000.0 << ' ';
		Sync1->Sync(17ms);
		if (OutTime) cout << (std::chrono::high_resolution_clock::now() - Time).count() / 1'000'000.0 << std::endl;
	}
	return 0;
}