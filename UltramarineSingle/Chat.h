#pragma once
#include "Graphic.h"
#include "Font.h"
#include "Geometry.h"
#include "Management.h"
#include "Drive.h"
#include <list>

class Chat:
	public SteepProcedUnit,
	public GraphicUnit
{
	Font* font;
	struct Rectangle Rect;
	struct Rectangle InputRect;
	list<string> Messages;
	string Input;
	bool Active = false;
	BYTE EnterKey = VK_RETURN;
	BYTE CloseKey = VK_ESCAPE;
	BYTE PrevKey = VK_UP;
	BYTE NextKey = VK_DOWN;
	void AddChar(char Char);
public:
	Chat(Font* font);
	void SteepProc() override;
	void DrawProc() override;
	void Send(string Message);
	void Activate();
	void Deactivate();
};