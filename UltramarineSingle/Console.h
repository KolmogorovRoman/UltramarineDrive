#pragma once
#include <map>
#include <string>
#include <sstream>
#include "Management.h"
#include "Graphic.h"
#include "Font.h"
#include "TupleRec.h"
#include "Geometry.h"

class Console:
	public SteepProcedUnit,
	public GraphicUnit
{
	class BaseCommand
	{
	public:
		string Name;
		BaseCommand(string Name);
		virtual void Execute(stringstream& Args) = 0;
	};
	template<class... Args> class Command:
		public BaseCommand
	{
	public:
		function<void(Args...)> Action;
		Command(string name, function<void(Args...)> Action);
		void Execute(stringstream& Args) override;
	};
	std::map<string, BaseCommand*> Commands;
	bool Active = false;
	Font* font;
	Layer* backlayer;
	Layer* layer;
	Controller& contr;
	string CurrentInput = "";
	string PrevInput = "";
	string Message = "";
	struct Rectangle InputRect;
	BYTE EnterKey = VK_RETURN;
	BYTE CloseKey = VK_ESCAPE;
	BYTE PrevKey = VK_UP;
	BYTE NextKey = VK_DOWN;
public:
	Console(Font* font, Layer* backlayer, Layer* layer, Controller& contr);
	void UpdateInputRect();
	template<class... Args> void RegisterCommand(string Name, function<void(Args...)> Action);
	void DrawProc() override;
	void Activate();
	void Deactivate();
	bool IsActive();
	void AddChar(char Char);
	void Execute();
	void Execute(string Input);
	void Execute(stringstream& Input);
	void SteepProc() override;
	void SetMessage(string Message);
	void ForMagic(function<void()>);
};
template<class T>
void Parse(stringstream& String, T& t)
{
	string s;
	String >> s;
	if (String.fail() && String.eof())
		throw "Too few arguments"s;
	stringstream ss(s);
	ss >> t;
	if (ss.fail())
		throw "Parse error: "s + s;
}
template<class... Types>
void ParseTuple(stringstream& String, TupleRec<Types...>& Tuple)
{
	Parse(String, Tuple.Head);
	ParseTuple(String, Tuple.Tail);
}
void ParseTuple(stringstream& String, TupleRec<>& Tuple);

template<class T>
struct Arg
{
	T Value;
	bool IsValid = true;
	bool IsError = false;
	bool IsSkipped = false;
	string Input;
	void SetDefault(T& Value);
	void SetDefault(T&& Value);
	operator T();
	T& operator* ();
	T operator-> ();
};
template<class T>
void Arg<T>::SetDefault(T& Value)
{
	if (IsSkipped) this->Value = Value;
}
template<class T>
void Arg<T>::SetDefault(T&& Value)
{
	if (IsSkipped) this->Value = Value;
}
template<class T>
stringstream& operator >> (stringstream& String, Arg<T>& arg)
{
	String >> arg.Input;
	if (String.fail() && String.eof())
	{
		arg.IsValid = false;
		arg.IsSkipped = true;
		return String;
	}
	stringstream ss(arg.Input);
	ss >> arg.Value;
	if (ss.fail())
	{
		arg.IsValid = false;
		arg.IsError = true;
		return String;
	}
	return String;
}
template<class T>
Arg<T>::operator T()
{
	return Value;
}
template<class T>
T& Arg<T>::operator* ()
{
	return Value;
}
template<class T>
T Arg<T>::operator-> ()
{
	return Value;
}
template<>
struct Arg<string>
{
	string Value;
	operator string();
	string& operator* ();
};
template<>
void Parse<Arg<string>>(stringstream& String, Arg<string>& s);

template<class... Args>
Console::Command<Args...>::Command(string Name, function<void(Args...)> Action):
	BaseCommand(Name), Action(Action)
{}
template<class... Args>
void Console::Command<Args...>::Execute(stringstream& String)
{
	TupleRec<Args...> Tuple;
	ParseTuple(String, Tuple);
	Tuple.Call(Action);
}

template<class... Args>
void Console::RegisterCommand(string Name, function<void(Args...)> Action)
{
	if (Commands.find(Name) != Commands.end())
	{
		Message = "Command " + Name + " already exist";
		return;
	}
	Commands[Name] = new Command<Args...>(Name, Action);
}