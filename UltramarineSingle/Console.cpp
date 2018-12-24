#include "Console.h"

//template<>
Expression*& Arg<Expression*>::GetValue(Type& arg)
{
	return arg;
}
PrevExpression*& Arg<PrevExpression*>::GetValue(Type& arg)
{
	return arg;
}
template<>
void Parse<string>(Interpreter* Interpreter, istream& String, Arg<string>& t)
{
	t.Value = Interpreter->GetExpression(String)->Execute();
}


Console::Console(Font* font, Layer* backlayer, Layer* layer, Controller& contr):
	font(font), backlayer(backlayer), layer(layer), contr(contr),
	GraphicUnit(NULL, NULL),
	InputRect(-MainCamera.Width / 2, MainCamera.Height / 2, MainCamera.Width / 2, MainCamera.Height / 2 - font->Common.LineHeight)
{
	Interpreter::RegisterCommand((BaseCommand*) new BlockCommand);
	Interpreter::RegisterCommand((StringCommand*) new StringCommand);
	RegisterCommand("}", Proc(void){}, Console::WrapArgs);
	RegisterCommand("if", Proc(void, Expression* cond, Expression* expr)
	{
		if (cond->Execute() != "0"s) expr->Execute();
	}, Console::WrapArgs);
	RegisterCommand("and", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		if (expr1->expr->Execute() != "0"s)
			return Bool(expr2->Execute());
		else return false;
	}, Console::WrapArgs);
	RegisterCommand("or", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		if (expr1->expr->Execute() == "0"s)
			return Bool(expr2->Execute());
		else return true;
	}, Console::WrapArgs);
	RegisterCommand("==", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		return expr1->expr->Execute() == expr2->Execute();
	}, Console::WrapArgs);
	RegisterCommand(">", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		return expr1->expr->Execute() > expr2->Execute();
	}, Console::WrapArgs);
	RegisterCommand("<", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		return expr1->expr->Execute() < expr2->Execute();
	}, Console::WrapArgs);
	RegisterCommand(">=", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		return expr1->expr->Execute() >= expr2->Execute();
	}, Console::WrapArgs);
	RegisterCommand("<=", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		return expr1->expr->Execute() <= expr2->Execute();
	}, Console::WrapArgs);
	RegisterCommand("!=", Proc(bool, PrevExpression* expr1, Expression* expr2)
	{
		return expr1->expr->Execute() != expr2->Execute();
	}, Console::WrapArgs);
	RegisterCommand("+", Proc(int, PrevExpression* expr1, Expression* expr2)
	{
		return Int(expr1->expr->Execute()) + Int(expr2->Execute());
	}, Console::WrapArgs);
	RegisterCommand("-", Proc(int, PrevExpression* expr1, Expression* expr2)
	{
		return Int(expr1->expr->Execute()) - Int(expr2->Execute());
	}, Console::WrapArgs);
	RegisterCommand("*", Proc(int, PrevExpression* expr1, Expression* expr2)
	{
		return Int(expr1->expr->Execute()) * Int(expr2->Execute());
	}, Console::WrapArgs);
	RegisterCommand("/", Proc(int, PrevExpression* expr1, Expression* expr2)
	{
		return Int(expr1->expr->Execute()) / Int(expr2->Execute());
	}, Console::WrapArgs);
	RegisterCommand("%", Proc(int, PrevExpression* expr1, Expression* expr2)
	{
		return Int(expr1->expr->Execute()) % Int(expr2->Execute());
	}, Console::WrapArgs);
	RegisterCommand("const", Proc(string, string Value)
	{
		return Value;
	}, Console::NoWrapArgs);
	RegisterCommand("var", Proc(void, string VarName)
	{
		//Variables[VarName] = ""s;
		RegisterCommand(new Variable(VarName));
	}, Console::WrapArgs);
	/*RegisterCommand("get", Proc(string, string VarName)
	{
		return Variables[VarName];
	});*/
	RegisterCommand("print", Proc(void, Expression* expr)
	{
		Message = expr->Execute();
	}, Console::WrapArgs);
	/*RegisterCommand("set", Proc(void, string VarName, Expression* expr)
	{
		Variables[VarName] = expr->Execute();
	});*/
	/*RegisterCommand("del", Proc(string, string VarName)
	{
		string Value = Variables[VarName];
		Variables.erase(VarName);
		return Value;
	});*/
	RegisterCommand("for", Proc(void, Expression* init, Expression* cond, Expression* iter, Expression* expr)
	{
		for (init->Execute(); Bool(cond->Execute()); iter->Execute())
			expr->Execute();
	}, Console::WrapArgs);
	RegisterCommand("while", Proc(void, Expression* cond, Expression* expr)
	{
		while (Bool(cond->Execute()))
			expr->Execute();
	}, Console::WrapArgs);
	RegisterCommand("alias", Proc(void, string name, Expression* expr)
	{
		RegisterCommand(name, (function<string()>)[&, expr]()->string
		{
			return expr->Execute();
		});
	}, Console::NoWrapArgs);
}
Expression* Console::GetExpression(istream& Input)
{
	char NextChar;
	streampos pos = Input.tellg();
	Input >> NextChar;
	Input.seekg(pos);
	if ((NextChar >= '0' && NextChar <= '9') || NextChar == '-' || NextChar == '+')
	{
		BaseCommand* basecommand = Commands["const"s];
		Expression* expr = basecommand->GetExpression(this, Input);
		return expr;
	}
	string CommandName;
	Input >> CommandName;
	BaseCommand* basecommand = Commands[CommandName];
	Expression* expr = basecommand->GetExpression(this, Input);
	return expr;
}
string Console::Execute(istream& Input)
{
	/*string Result = ""s;
	while (Input && !Input.eof())
	{
		Result = GetExpression(Input)->Execute();
	}
	return Result;*/
	return GetExpression(Input)->Execute();
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
	Message = Result;
	return Result;
}
string Console::Execute(string Input)
{
	istringstream Stream(Input);
	return Execute(Stream);
}
void Console::UpdateInputRect()
{
	InputRect.Left = -MainCamera.Width / 2;
	InputRect.Top = MainCamera.Height / 2;
	InputRect.Right = MainCamera.Width / 2;
	InputRect.Bottom = MainCamera.Height / 2 - font->Common.LineHeight;
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

BlockExpression::BlockExpression(BaseCommand* command, Console* console):
	Expression(command),
	console(console)
{}
string BlockExpression::Execute()
{
	/*string Result;
	for (list<Expression*>::iterator expr = Expressions.begin(); expr != Expressions.end(); ++expr)
	{
		list<Expression*>::iterator nextexpr = expr;
		++nextexpr;

	}
	return Result;*/
	string Result;
	for (Expression* expr : Expressions)
	{
		Result = expr->Execute();
	}
	return Result;
}
BlockCommand::BlockCommand():
	BaseCommand("{")
{}
Expression* BlockCommand::GetExpression(Interpreter* Interpreter, istream& s)
{
	BlockExpression* expr = new BlockExpression(this, (Console*)Interpreter);
	while (s)
	{
		Expression* e = Interpreter->GetExpression(s);
		if (e->basecommand->Name == "}") break;
		expr->Expressions.push_back(e);
		((Console*)Interpreter)->LastExpression = e;
	}
	return expr;
}

StringExpression::StringExpression(BaseCommand* command):
	Expression(command)
{}
string StringExpression::Execute()
{
	/*string Result;
	for (list<Expression*>::iterator expr = Expressions.begin(); expr != Expressions.end(); ++expr)
	{
	list<Expression*>::iterator nextexpr = expr;
	++nextexpr;

	}
	return Result;*/
	return String;
}
StringCommand::StringCommand():
	BaseCommand("\""s)
{}
Expression* StringCommand::GetExpression(Interpreter* Interpreter, istream& s)
{
	StringExpression* expr = new StringExpression(this);
	getline(s, expr->String, '"');
	/*string scrap;
	s >> scrap;*/
	return expr;
}

VariableExpression::VariableExpression(BaseCommand* command):
	Expression(command)
{}
string VariableExpression::Execute()
{
	return ((Variable*)basecommand)->Value;
}
Variable::Variable(string Name):
	BaseCommand(Name)
{}
Expression* Variable::GetExpression(Interpreter* Interpreter, istream& s)
{
	VariableExpression* expr = new VariableExpression(this);
	return expr;
}
void Parse(Interpreter* console, istream& String, Variable*& v)
{
	string Name;
	String >> Name;
	BaseCommand* command = console->GetCommand(Name);
	v = (Variable*)command;
}

//PrevResultCommand::PrevResultCommand():
//	BaseCommand("begin")
//{}
//Expression* PrevResultCommand::GetExpression(Interpreter* Interpreter, istream& s)
//{
//	BlockExpression* expr = new BlockExpression(this, (Console*)Interpreter);
//	while (s)
//	{
//		Expression* e = Interpreter->GetExpression(s);
//		if (e->basecommand->Name == "end") break;
//		expr->Expressions.push_back(e);
//	}
//	return expr;
//}
void Parse(Interpreter* console, istream& String, PrevExpression*& res)
{
	res = new PrevExpression();
	res->expr = ((Console*)console)->LastExpression;
}

bool Bool(string& s)
{
	return s == "true"s;
}
int Int(string& s)
{
	return stoi(s);
}