#include "Chat.h"

Chat::Chat(Font* font):
	GraphicUnit(NULL, NULL),
	font(font),
	Rect(100, -300, 400, -100),
	InputRect(Rect.Left, Rect.Bottom, Rect.Right, Rect.Bottom + font->Common.LineHeight)
{}
void Chat::AddChar(char Char)
{
	Input += Char;
}
void Chat::Send(string Message)
{
	Messages.push_back(Message);
	Deactivate();
}
void Chat::SteepProc()
{
	if (!Active && Contr1.Keys[EnterKey] == 1)
	{
		Activate();
		return;
	}
	if (!Active) return;
	if (Contr1.Keys[EnterKey] == 1 && Input == "")
	{
		Deactivate();
		return;
	}
	BYTE* KeyboardState = new BYTE[256];
	GetKeyboardState(KeyboardState);
	for (int i = 0; i < 256; i++)
	{
		WORD C;
		if (Contr1.Keys[i] == 1 || (Contr1.Keys[i] >= 20 && Contr1.Keys[i] & 1 != 0))
		{
			if (ToAscii(i, 0b10000000, KeyboardState, &C, 0) == 1)
				if ((char)C >= ' ' && (char)C <= '~')
					AddChar(C);
		}
	}
	if (Contr1.Keys[EnterKey] == 1)
	{
		Send(Input);
		Deactivate();
	}
	if (Contr1.Keys[VK_BACK] == 1 || (Contr1.Keys[VK_BACK] >= 20 && Contr1.Keys[VK_BACK] & 1 != 0)) Input = Input.substr(0, Input.length() - 1);
	if (Contr1.Keys[CloseKey] == 1)
	{
		Deactivate();
		return;
	}
}
void Chat::DrawProc()
{
	if (!Active)
		DrawRect(Rect.Left, Rect.Bottom, Rect.Right, Rect.Top, true, InterfaceBackLayer, 0.2, 0.2, 0.2, 0.2);
	else
		DrawRect(Rect.Left, Rect.Bottom, Rect.Right, Rect.Top, true, InterfaceBackLayer, 0.2, 0.2, 0.2, 0.8);
	DrawRect(Rect.Left, Rect.Bottom, Rect.Right, Rect.Top, false, InterfaceLayer, 0, 0, 0, 1);
	int y = Rect.Top;
	for (string Message : Messages)
	{
		font->Draw(Message, Rect.Left, y, InterfaceLayer);
		y -= font->Common.LineHeight;
	}
	if (Active)
	{
		DrawRect(InputRect.Left, InputRect.Bottom, InputRect.Right, InputRect.Top,
			true, InterfaceBackLayer, 0.2, 0.2, 0.2, 0.8);
		DrawRect(InputRect.Left, InputRect.Bottom, InputRect.Right, InputRect.Top,
			false, InterfaceLayer, 0, 0, 0, 1);
		font->Draw(Input, InputRect.Left, InputRect.Top, InterfaceLayer);
	}
}
void Chat::Activate()
{
	Active = true;
}
void Chat::Deactivate()
{
	Input = "";
	Active = false;
}