template<class T>
void Parse(Interpreter* Interpreter, istream& String, T& t)
{
	String >> t;
	/*if (String.fail())
	{
	if (String.eof())
	throw "Too few arguments"s;
	else
	throw "Parse error"s;
	}*/
}
template<class... Types>
void ParseTuple(Interpreter* Interpreter, istream& String, TupleRec<Types...>& Tuple)
{
	Parse(Interpreter, String, Tuple.Head);
	ParseTuple(Interpreter, String, Tuple.Tail);
}

template<class... Args>
Command<Args...>::Command(string Name, function<string(Args...)> Action):
	BaseCommand(Name), Action(Action)
{}
template<class... Args>
Expression* Command<Args...>::GetExpression(Interpreter* Interpreter, istream& args)
{
	ConcreteExpression<Args...>* expr = new ConcreteExpression<Args...>(this);
	ParseTuple(Interpreter, args, expr->Arguments);
	return (Expression*)expr;
}

template<class... Args>
ConcreteExpression<Args...>::ConcreteExpression(Command<Args...>* command) :
	command(command),
	Expression((BaseCommand*)command)
{}
template<class... Args>
string ConcreteExpression<Args...>::Execute()
{
	CalculateTuple(Arguments);
	return Arguments.Call(command->Action);
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