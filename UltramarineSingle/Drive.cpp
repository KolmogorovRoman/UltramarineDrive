#include "Drive.h"

Controller Contr1;
Syncronizer* Sync1;

Layer* LowestLayer;
Layer* InterfaceLayer;
Layer* InterfaceBackLayer;

void Control()
{
	GetCursorPos(&(Contr1.Mouse));
	ScreenToClient(hWnd, (&Contr1.Mouse));
	Contr1.Mouse.y = MainCamera.Height - Contr1.Mouse.y;
	Contr1.Mouse.x -= MainCamera.Width / 2;
	Contr1.Mouse.y -= MainCamera.Height / 2;
	Vector tv = TurnV(v(Contr1.Mouse.x, Contr1.Mouse.y)*MainCamera.Scale, -MainCamera.angle);
	Contr1.Mouse.x = tv.x;
	Contr1.Mouse.y = tv.y;
	Contr1.Mouse.x += MainCamera.x;
	Contr1.Mouse.y += MainCamera.y;
	for (int i = 0; i < 256; i += 1)
	{
		if (GetKeyState(i) & 0b10000000)
		{
			if (Contr1.Keys[i] < 255) Contr1.Keys[i] ++;
			/*if ((Contr1.Keys[i] & 0b00000001) == 0)
				Contr1.Keys[i] = 0b00000011;
			else
				Contr1.Keys[i] = 0b00000001;*/
		}
		else Contr1.Keys[i] = 0;
	}
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	//AllocConsole();
	//freopen("CONOUT$", "w", stdout);
	//freopen("CONIN$", "r", stdin);
	InitGraphic(hinst, nCmdShow, L"Single"); // Инициализация графики
	Sync1 = new Syncronizer();
	LowestLayer = Layer::New(false);
	InterfaceLayer = Layer::Higher(LowestLayer, true);       // Создание основных слоёв
	InterfaceBackLayer = Layer::Lower(InterfaceLayer, true);
	BeginDraw();
	InitLayers(); // Создание остальных игровых слоев
	LoadImages(); // Загрузка изображений
	LoadMasks(); //Загрузка масок для столкновений
	GameInit(); // Инициализация игры
	EndDraw();
	while (true)
	{
		BaseUnit::ActivateAll(); // Активация всех объектов
		Control(); // Получение состояния клавиатуры и мыши
		BeginDraw(); 
		AllCollCheck(); // Проверка столкновений и их обработка
		CalcAllSolidPhysicUnits(); // Расчет всех физических столкновений
		CallEach(PhysicUnit) Calc(); // Расчет физики
		CallEach(SteepProcedUnit) SteepProc(); // Вызов всех функций SteepProc
		CallEach(GraphicUnit) DrawProc(); // Вызов функций, сообщающих об отрисовке
		CallEach(GraphicUnit) Draw(); // Рисование GraphicUnit'ов
		CallEach(AnimatedGraphicUnit) DrawProc(); // Вызов функций, сообщающих об отрисовке
		CallEach(AnimatedGraphicUnit) Draw(); // Рисование AnimatedGraphicUnit'ов
		SteepProc(); // Вызов функции для каждого шага
		Draw(); // Общее рисование
		EndDraw();
		BaseUnit::DeleteAllThatNeedDelete(); // Удаление всех объектов, помеченных, как требующе удаления
		if (Closed) return 0;
		//bool OutTime = Contr1.Keys[VK_TAB];
		//if (OutTime) cout << (std::chrono::high_resolution_clock::now() - Time).count() / 1'000'000.0 << ' ';
		Sync1->Sync(17ms); // Осуществление задержки для поддержания 60 шагов в секунду
		//if (OutTime) cout << (std::chrono::high_resolution_clock::now() - Time).count() / 1'000'000.0 << std::endl;
	}
	return 0;
}