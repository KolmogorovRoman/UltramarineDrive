#include "Interpreter.h"

template<>
void ParseTuple<>(Interpreter* Interpreter, istream& String, TupleRec<>& Tuple)
{}

BaseCommand::BaseCommand(string Name):
	Name(Name)
{}

void Interpreter::RegisterCommand(BaseCommand* command)
{
	Commands[command->Name] = command;
}
Expression* Interpreter::GetExpression(istream& Input)
{
	string CommandName;
	Input >> CommandName;
	BaseCommand* basecommand = Commands[CommandName];
	Expression* expr = basecommand->GetExpression(this, Input);
	return expr;
}

BaseCommand* Interpreter::GetCommand(string Name)
{
	return Commands[Name];
}

Expression::Expression(BaseCommand* basecommand):
	basecommand(basecommand)
{}

template<>
void Parse<Expression*>(Interpreter* Interpreter, istream& String, Expression*& e)
{
	e = Interpreter->GetExpression(String);
}

template<>
void CalculateTuple<>(TupleRec<>& Tuple)
{}