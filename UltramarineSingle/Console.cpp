#include "Console.h"

Console::Command::Command()
{}
Console::Command::Command(string Name, function<void(string Arg)> Action):
	Name(Name), Action(Action)
{}
void Console::RegisterCommand(string Name, function<void(string Arg)> Action)
{
	if (Commands.find(Name) != Commands.end())
	{
		Message = "Command " + Name + " already exist";
		return;
	}
	Commands[Name] = new Command(Name, Action);
}
void Console::Draw()
{
	if (Active)
	{
		DrawRect(-MainCamera.Width / 2, MainCamera.Height / 2, MainCamera.Width / 2, MainCamera.Height / 2 - 32, true, InterfaceBackLayer, 0.2, 0.2, 0.2, 0.75);
		DrawRect(-MainCamera.Width / 2, MainCamera.Height / 2, MainCamera.Width / 2, MainCamera.Height / 2 - 32, false, InterfaceLayer);
		if (CurrentInput != "")
			TextStd->Draw(CurrentInput, -MainCamera.Width / 2, MainCamera.Height / 2 - 32, InterfaceLayer, false);
		else
			TextStd->Draw(Message, -MainCamera.Width / 2, MainCamera.Height / 2 - 32, InterfaceLayer, false);
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
void Console::Execute()
{
	string CommandText;
	string Args = "";
	getline(stringstream(CurrentInput), CommandText, ' ');
	if (CurrentInput.length() > CommandText.length())
		Args = CurrentInput.substr(CommandText.length() + 1, 256);
	Command* command = Commands[CommandText];
	if (command != NULL) command->Action(Args);
	else Message = "Command not exist";
	PrevInput = CurrentInput;
	CurrentInput = "";
	if (Message == "") Deactivate();
}
void Console::Execute(string Input)
{
	string CommandText;
	string Args = "";
	getline(stringstream(Input), CommandText, ' ');
	if (Input.length() > CommandText.length())
		Args = Input.substr(CommandText.length() + 1, 256);
	Command* command = Commands[CommandText];
	if (command != NULL) command->Action(Args);
	else Message = "Command not exist";
	Deactivate();
}
void Console::SteepProc()
{
	if (!Active && Contr1.Keys[VK_RETURN] == 1)
	{
		Activate();
		return;
	}
	if (!Active) return;
	if (Contr1.Keys[VK_RETURN] == 1 && CurrentInput == "")
	{
		Deactivate();
		return;
	}
	if ((Contr1.Keys[VK_UP] == 1 || Contr1.Keys[VK_DOWN] == 1))
	{
		swap(CurrentInput, PrevInput);
		Message = "";
	}
	BYTE* KeyboardState = new BYTE[256];
	GetKeyboardState(KeyboardState);
	for (int i = 0; i < 256; i++)
	{
		WORD C;
		if (Contr1.Keys[i] == 1 || (Contr1.Keys[i] >= 20 && Contr1.Keys[i] & 1 != 0))
		{
			if (ToAscii(i, 0b10000000, KeyboardState, &C, 0) == 1)
				if ((char) C >= ' ' && (char) C <= '~')
					AddChar(C);
		}
	}
	if (Contr1.Keys[VK_RETURN] == 1) Execute();
	if (Contr1.Keys[VK_BACK] == 1 || (Contr1.Keys[VK_BACK] >= 20 && Contr1.Keys[VK_BACK] & 1 != 0)) CurrentInput = CurrentInput.substr(0, CurrentInput.length() - 1);
	if (Contr1.Keys[VK_ESCAPE] == 1)
	{
		Deactivate();
		return;
	}
	Draw();
}
void Console::SetMessage(string Message)
{
	this->Message = Message;
}