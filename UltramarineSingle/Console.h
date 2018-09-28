#pragma once
#include <map>
#include <string>
#include <sstream>
#include "Management.h"
#include "Graphic.h"
#include "Font.h"
#include "TupleRec.h"
#include "Geometry.h"

class BaseCommand;
template<class... Args>
struct Command;
class Expression;
template<class... Args>
struct ConcreteExpression;
class Console;


template<class T>
void Parse(Console* console, istream& String, T& t)
{
	String >> t;
	if (String.fail())
	{
		if (String.eof())
			throw "Too few arguments"s;
		else
			throw "Parse error"s;
	}
}
template<class... Types>
void ParseTuple(Console* console, istream& String, TupleRec<Types...>& Tuple)
{
	Parse(console, String, Tuple.Head);
	ParseTuple(console, String, Tuple.Tail);
}
void ParseTuple(Console* console, istream& String, TupleRec<>& Tuple);

class BaseCommand
{
public:
	string Name;
	BaseCommand(string Name);
	//virtual string Parse(istream& String) = 0;
	string Execute(Console* console, istream& Args);
	virtual Expression* GetExpression(Console* console, istream& Args) = 0;
};
template<class... Args> class Command:
	public BaseCommand
{
public:
	function<string(Args...)> Action;
	Command(string name, function<string(Args...)> Action);
	Expression* GetExpression(Console* console, istream& Args) override;
};
template<class... Args>
Command<Args...>::Command(string Name, function<string(Args...)> Action):
	BaseCommand(Name), Action(Action)
{}
template<class... Args>
Expression* Command<Args...>::GetExpression(Console* console, istream& args)
{
	ConcreteExpression<Args...>* expr = new ConcreteExpression<Args...>(this);
	ParseTuple(console, args, expr->Arguments);
	return (Expression*)expr;
}

struct Expression
{
	BaseCommand* basecommand;
	virtual string Execute() = 0;
	Expression(BaseCommand* command);
};
template<class... Args>
struct ConcreteExpression:
	Expression
{
	Command<Args...>* command;
	TupleRec<Args...> Arguments;
	string Execute() override;
	ConcreteExpression(Command<Args...>* command);
};
template<class... Args>
ConcreteExpression<Args...>::ConcreteExpression(Command<Args...>* command):
	command(command),
	Expression((BaseCommand*)command)
{}
template<class... Args>
string ConcreteExpression<Args...>::Execute()
{
	CalculateTuple(Arguments);
	return Arguments.Call(command->Action);
}
void Parse(Console* console, istream& String, Expression*& e);

class Console:
	public SteepProcedUnit,
	public GraphicUnit
{
	map<string, BaseCommand*> Commands;
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
	void RegisterCommand(BaseCommand* command);
	template<class... Args> void RegisterCommand(string Name, function<string(Args...)> Action);
	template<class... Args> void RegisterCommand(string Name, function<void(Args...)> Action);
	template<class... Args> void RegisterCommand(string Name, function<bool(Args...)> Action);
	void DrawProc() override;
	void Activate();
	void Deactivate();
	bool IsActive();
	void AddChar(char Char);
	string Execute();
	string Execute(string Input);
	string Execute(istream& Input);
	Expression* GetExpression(istream& Input);
	void SteepProc() override;
	void SetMessage(string Message);
	void ForMagic(function<void()>);
};

template<class... Args>
void Console::RegisterCommand(string Name, function<string(Args...)> Action)
{
	if (Commands.find(Name) != Commands.end())
	{
		Message = "Command " + Name + " already exist";
		return;
	}
	RegisterCommand((BaseCommand*)new Command<Args...>(Name, Action));
}
template<class... Args>
void Console::RegisterCommand(string Name, function<void(Args...)> Action)
{
	function<string(Args...)> action = [&, Action](Args... args)
	{
		Action(args...);
		return ""s;
	};
	RegisterCommand(Name, action);
}
template<class... Args>
void Console::RegisterCommand(string Name, function<bool(Args...)> Action)
{
	function<string(Args...)> action = [&, Action](Args... args)->string
	{
		bool res = Action(args...);
		if (res == true) return "true"s;
		else return "false"s;
	};
	RegisterCommand(Name, action);
}

template<class T>
void Calculate(T& t)
{}
template<class... Args>
void CalculateTuple(TupleRec<Args...>& Tuple)
{
	Calculate(Tuple.Head);
	Calculate(Tuple.Tail);
}
void CalculateTuple(TupleRec<>& Tuple);


template<class T>
struct Arg
{
	T Value;
	bool IsValid = true;
	bool IsError = false;
	bool IsSkipped = false;
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
template<class T>
void Parse(Console* console, istream& String, Arg<T>& t)
{
	int State = String.rdstate();
	Parse(console, String, t.Value);
	if (String.fail())
	{
		if (String.eof())
		{
			t.IsValid = false;
			t.IsSkipped = true;
		}
		else
		{
			t.IsValid = false;
			t.IsError = true;
		}
	}
	String.setstate(State);
}
template<>
struct Arg<string>
{
	string Value;
	operator string();
	string& operator* ();
};
template<class T>
void Calculate(Arg<T>& t)
{
	if (t.IsValid)
		Calculate(t.Value);
}