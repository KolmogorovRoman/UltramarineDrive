#pragma once
#include "Interpreter.h"
#include "Management.h"
#include "Graphic.h"
#include "Font.h"
#include "Geometry.h"

template<class T>
struct Arg;
template<class T>
struct Opt;
class Console;
class BlockCommand;
class BlockExpression;
struct PrevExpression;

class Console:
	public Interpreter,
	public SteepProcedUnit,
	public GraphicUnit
{
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
	enum CommandType
	{
		WrapArgs,
		NoWrapArgs,
		Both
	};
	Expression* LastExpression;
	string LastResult;
	//map<string, string> Variables;
	Console(Font* font, Layer* backlayer, Layer* layer, Controller& contr);
	using Interpreter::RegisterCommand;
	template<class... Args> void RegisterCommand(string Name, function<string(Args...)> Action, CommandType Type = Both);
	template<class Res, class... Args> void RegisterCommand(string Name, function<Res(Args...)> Action, CommandType Type = Both);
	template<class... Args> void RegisterCommand(string Name, function<void(Args...)> Action, CommandType Type = Both);
	/*template<class... Args> void RegisterCommand(string Name, function<void(Args...)> Action);
	template<class... Args> void RegisterCommand(string Name, function<bool(Args...)> Action);*/

	Expression* GetExpression(istream& Input) override;
	string Execute(istream& Input);
	string Execute();
	string Execute(string Input);

	void UpdateInputRect();
	void DrawProc() override;
	void Activate();
	void Deactivate();
	bool IsActive();
	void AddChar(char Char);
	void SteepProc() override;
	void SetMessage(string Message);
};

template<class T>
struct Arg
{
	typedef Arg<T> Type;
	T Value;
	static T& GetValue(Type& arg);
	Arg() {}
};
template<>
struct Arg<Expression*>
{
	typedef Expression* Type;
	static Expression*& GetValue(Type& arg);
};
template<>
struct Arg<PrevExpression*>
{
	typedef PrevExpression* Type;
	static PrevExpression*& GetValue(Type& arg);
};

template<class T>
void Parse(Interpreter* Interpreter, istream& String, Arg<T>& t);
template<>
void Parse<string>(Interpreter* Interpreter, istream& String, Arg<string>& t);

template<class T>
struct Opt
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
void Opt<T>::SetDefault(T& Value)
{
	if (IsSkipped) this->Value = Value;
}
template<class T>
void Opt<T>::SetDefault(T&& Value)
{
	if (IsSkipped) this->Value = Value;
}
template<class T>
Opt<T>::operator T()
{
	return Value;
}
template<class T>
T& Opt<T>::operator* ()
{
	return Value;
}
template<class T>
T Opt<T>::operator-> ()
{
	return Value;
}
template<class T>
void Parse(Interpreter* console, istream& String, Opt<T>& t)
{
	/*
	streampos pos = String.tellg();
	string Input;
	String >> Input;
	if (Input == "{"s)
	{
		String.seekg(pos);
		Expression* expr = console->GetExpression(String);

	}*/
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
template<class T>
void Calculate(Opt<T>& t)
{
	if (t.IsValid)
		Calculate(t.Value);
}

struct BlockExpression:
	public Expression
{
	list<Expression*> Expressions;
	Console* console;
	BlockExpression(BaseCommand* command, Console* console);
	string Execute() override;
};
struct BlockCommand:
	public BaseCommand
{
	BlockCommand();
	Expression* GetExpression(Interpreter* Interpreter, istream& Args) override;
};

struct StringExpression:
	public Expression
{
	string String;
	StringExpression(BaseCommand* command);
	string Execute() override;
};
struct StringCommand:
	public BaseCommand
{
	StringCommand();
	Expression* GetExpression(Interpreter* Interpreter, istream& Args) override;
};

struct VariableExpression:
	public Expression
{
	VariableExpression(BaseCommand* command);
	string Execute() override;
};
struct Variable:
	public BaseCommand
{
	string Name;
	string Value;
	Variable(string Name);
	Expression* GetExpression(Interpreter* Interpreter, istream& Args) override;
};

struct PrevExpression
{
	Expression* expr;
};
//struct PrevResultCommand:
//	public BaseCommand
//{
//	PrevResultCommand();
//	Expression* GetExpression(Interpreter* Interpreter, istream& Args) override;
//};
void Parse(Interpreter* console, istream& String, PrevExpression*& res);

bool Bool(string& s);
int Int(string& s);
template<class T>
string String(T t);

template<class T>
string String(T t)
{
	return to_string(t);
}

#include "Console.hpp"