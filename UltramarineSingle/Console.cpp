#include "Console.h"

void ParseTuple(Console* console, istream& String, TupleRec<>& Tuple)
{}

Arg<string>::operator string()
{
	return Value;
}
string& Arg<string>::operator* ()
{
	return Value;
}

Console::Console(Font* font, Layer* backlayer, Layer* layer, Controller& contr):
	font(font), backlayer(backlayer), layer(layer), contr(contr),
	GraphicUnit(NULL, NULL),
	InputRect(-MainCamera.Width / 2, MainCamera.Height / 2, MainCamera.Width / 2, MainCamera.Height / 2 - font->Common.LineHeight)
{}
void Console::RegisterCommand(BaseCommand* command)
{
	Commands[command->Name] = command;
}
void Console::UpdateInputRect()
{
	InputRect.Left = -MainCamera.Width / 2;
	InputRect.Top = MainCamera.Height / 2;
	InputRect.Right = MainCamera.Width / 2;
	InputRect.Bottom = MainCamera.Height / 2 - font->Common.LineHeight;
}
BaseCommand::BaseCommand(string Name):
	Name(Name)
{}
string BaseCommand::Execute(Console* console, istream& String)
{
	return GetExpression(console, String)->Execute();
}
void Console::DrawProc()
{
	if (Active)
	{
		DrawRect(InputRect.Left, InputRect.Bottom, InputRect.Right, InputRect.Top, true, backlayer, 0.2, 0.2, 0.2, 0.75);
		DrawRect(InputRect.Left, InputRect.Bottom, InputRect.Right, InputRect.Top, false, layer);
		if (CurrentInput != "")
			font->Draw(CurrentInput, InputRect.Left, InputRect.Top, layer);
		else
			font->Draw(Message, InputRect.Left, InputRect.Top, layer);
	}
}
void Console::Activate()
{
	Active = true;
}
void Console::Deactivate()
{
	CurrentInput = "";
	Message = "";
	Active = false;
}
bool Console::IsActive()
{
	return Active;
}
void Console::AddChar(char Char)
{
	CurrentInput += Char;
	Message = "";
}
string Console::Execute()
{
	string Result = ""s;
	try
	{
		Result = Execute(CurrentInput);
	}
	catch (string Error)
	{
		Message = Error;
	}
	PrevInput = CurrentInput;
	CurrentInput = "";
	if (Message == "") Deactivate();
	return Result;
}
string Console::Execute(string Input)
{
	istringstream Stream(Input);
	return Execute(Stream);
}
string Console::Execute(istream& Input)
{
	/*string CommandText;
	Input >> CommandText;
	BaseCommand* command = Commands[CommandText];
	string Result = ""s;
	if (command != NULL)
	{
		try
		{
			Result = command->Execute(this, Input);
		}
		catch (string Error)
		{
			Message = Error;
		}
	}
	else Message = "Command not exist";
	return Result;*/
	string Result = ""s;
	while (Input && !Input.eof())
	{
		Result = GetExpression(Input)->Execute();
	}
	return Result;
}
Expression* Console::GetExpression(istream& Input)
{
	string CommandName;
	Input >> CommandName;
	BaseCommand* basecommand = Commands[CommandName];
	Expression* expr = basecommand->GetExpression(this, Input);
	return expr;
}
void Console::SteepProc()
{
	if (!Active && contr.Keys[EnterKey] == 1)
	{
		Activate();
		return;
	}
	if (!Active) return;
	if (contr.Keys[EnterKey] == 1 && CurrentInput == "")
	{
		Deactivate();
		return;
	}
	if ((contr.Keys[PrevKey] == 1 || contr.Keys[NextKey] == 1))
	{
		swap(CurrentInput, PrevInput);
		Message = "";
	}
	BYTE* KeyboardState = new BYTE[256];
	GetKeyboardState(KeyboardState);
	for (int i = 0; i < 256; i++)
	{
		WORD C;
		if (contr.Keys[i] == 1 || (contr.Keys[i] >= 20 && contr.Keys[i] & 1 != 0))
		{
			if (ToAscii(i, 0b10000000, KeyboardState, &C, 0) == 1)
				if ((char)C >= ' ' && (char)C <= '~')
					AddChar(C);
		}
	}
	if (contr.Keys[EnterKey] == 1) Execute();
	if (contr.Keys[VK_BACK] == 1 || (contr.Keys[VK_BACK] >= 20 && contr.Keys[VK_BACK] & 1 != 0)) CurrentInput = CurrentInput.substr(0, CurrentInput.length() - 1);
	if (contr.Keys[CloseKey] == 1)
	{
		Deactivate();
		return;
	}
	//Draw();
}
void Console::SetMessage(string Message)
{
	this->Message = Message;
}
void Console::ForMagic(function<void()> func)
{
	func();
}

Expression::Expression(BaseCommand* basecommand):
	basecommand(basecommand)
{}

void Parse(Console* console, istream& String, Expression*& e)
{
	e = console->GetExpression(String);
}

void Parse(Console* console, istream& String, istream*& s)
{
	s = &String;
}

void CalculateTuple(TupleRec<>& Tuple)
{}