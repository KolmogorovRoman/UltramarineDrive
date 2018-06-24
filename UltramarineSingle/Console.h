#pragma once
#include <map>
#include <string>
#include <sstream>
#include "Management.h"
#include "Graphic.h"
#include "Drive.h"

class Console :
	public SteepProcedUnit
{
	class Command
	{
	public:
		string Name;
		std::function<void(string Arg)> Action;
		Command();
		Command(string Name, function<void(string Arg)> Action);
	};
	std::map<string, Command*> Commands;
	bool Active;
	string CurrentInput = "";
	string PrevInput = "";
	string Message = "";
public:
	void RegisterCommand(string Name, function<void(string Arg)> Action);
	void Regisrer();
	void Draw();
	void Activate();
	void Deactivate();
	bool IsActive();
	void AddChar(char Char);
	void Execute();
	void Execute(string Input);
	void SteepProc() override;
	void SetMessage(string Message);
};