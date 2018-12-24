template<class T>
T& Arg<T>::GetValue(Type& arg)
{
	return arg.Value;
}


template<class... Args>
void Console::RegisterCommand(string Name, function<string(Args...)> Action, CommandType Type)
{
	if (Type == WrapArgs || Type == Both)
	{
		function<string(typename Arg<Args>::Type...)> NewAction = [&, Action](typename Arg<Args>::Type... args)->string
		{
			return Action(Arg<Args>::GetValue(args)...);
		};
		RegisterCommand((BaseCommand*)new Command<typename Arg<Args>::Type...>(Name, NewAction));
	}
	if (Type == NoWrapArgs)
	{
		RegisterCommand((BaseCommand*)new Command<Args...>(Name, Action));
	}
	if (Type == Both)
	{
		RegisterCommand((BaseCommand*)new Command<Args...>("~"s + Name, Action));
	}
}
//template<class... Args>
//void Console::RegisterCommand(string Name, function<void(Args...)> Action)
//{
//	function<string(Args...)> action = [&, Action](Args... args)
//	{
//		Action(args...);
//		return ""s;
//	};
//	RegisterCommand(Name, action);
//}
//template<class... Args>
//void Console::RegisterCommand(string Name, function<bool(Args...)> Action)
//{
//	function<string(Args...)> action = [&, Action](Args... args)->string
//	{
//		bool res = Action(args...);
//		if (res == true) return "true"s;
//		else return "false"s;
//	};
//	RegisterCommand(Name, action);
//}
template<class Res, class... Args>
void Console::RegisterCommand(string Name, function<Res(Args...)> Action, CommandType Type)
{
	function<string(Args...)> action = [&, Action](Args... args)
	{
		Res r = Action(args...);
		stringstream s;
		s << r;
		return s.str();
	};
	RegisterCommand(Name, action, Type);
}
template<class... Args>
void Console::RegisterCommand(string Name, function<void(Args...)> Action, CommandType Type)
{
	function<string(Args...)> action = [&, Action](Args... args)
	{
		Action(args...);
		return ""s;
	};
	RegisterCommand(Name, action, Type);
}

template<class T>
void Parse(Interpreter* Interpreter, istream& String, Arg<T>& t)
{
	/*string NextLexemme;
	streampos pos = String.tellg();
	String >> NextLexemme;
	String.seekg(pos);
	if (NextLexemme == "{"s)
	{
		string Result = Interpreter->GetExpression(String)->Execute();
		Parse(Interpreter, istringstream(Result), t.Value);
	}
	else
		Parse(Interpreter, String, t.Value);*/
	string Result = Interpreter->GetExpression(String)->Execute();
	Parse(Interpreter, istringstream(Result), t.Value);
}