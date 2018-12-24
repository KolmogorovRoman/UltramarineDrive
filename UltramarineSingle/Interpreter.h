#pragma once
#include <map>
#include <string>
#include <sstream>
#include "TupleRec.h"
using namespace std;

#define Proc(ResultType, ...) (function<ResultType(__VA_ARGS__)>)[&](__VA_ARGS__)

class BaseCommand;
template<class... Args>
struct Command;
class Expression;
template<class... Args>
struct ConcreteExpression;
class Interpreter;

class BaseCommand
{
public:
	string Name;
	BaseCommand(string Name);
	virtual Expression* GetExpression(Interpreter* Interpreter, istream& Args) = 0;
};
template<class... Args> class Command:
	public BaseCommand
{
public:
	function<string(Args...)> Action;
	Command(string name, function<string(Args...)> Action);
	virtual Expression* GetExpression(Interpreter* Interpreter, istream& Args) override;
};

template<class T>
void Parse(Interpreter* Interpreter, istream& String, T& t);
template<class... Types>
void ParseTuple(Interpreter* Interpreter, istream& String, TupleRec<Types...>& Tuple);
template<>
void ParseTuple<>(Interpreter* Interpreter, istream& String, TupleRec<>& Tuple);

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

template<class T>
void Calculate(T& t);
template<class... Args>
void CalculateTuple(TupleRec<Args...>& Tuple);
template<>
void CalculateTuple<>(TupleRec<>& Tuple);

template<>
void Parse<Expression*>(Interpreter* Interpreter, istream& String, Expression*& e);

class Interpreter
{
protected:
	map<string, BaseCommand*> Commands;
public:
	void RegisterCommand(BaseCommand* command);
	BaseCommand* GetCommand(string Name);
	virtual Expression* GetExpression(istream& Input);	
};

#include "Interpreter.hpp"