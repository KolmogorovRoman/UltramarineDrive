#pragma once
#include <functional>
using namespace std;

template<class... T>
struct TupleRec;

template<>
struct TupleRec<>
{
	template<class fRes, class... fArgs>
	fRes Call(function<fRes(fArgs...)> func)
	{
		return call(func);
	}
	template<class fRes, class... fArgs, class... Args>
	fRes call(function<fRes(fArgs...)> func, Args... args)
	{
		return func(args...);
	}
};

template<class H, class... T>
struct TupleRec<H, T...>
{
	H Head;
	TupleRec<T...> Tail;
	TupleRec()
	{}
	template<class Opt, class... Args>
	TupleRec(Opt Opt, Args... args):
		Head(Opt),
		Tail(args...)
	{}
	template<class fRes, class... fArgs>
	fRes Call(function<fRes(fArgs...)> func)
	{
		return call(func);
	}
	template<class Func>
	auto Call(Func* func)
	{
		return call((function<Func>) func);
	}
	template<class fRes, class... fArgs, class... Args>
	fRes call(function<fRes(fArgs...)> func, Args... args)
	{
		return Tail.call(func, args..., Head);
	}
};