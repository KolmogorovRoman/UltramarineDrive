#pragma once

struct Point
{
	double x, y;
	Point(double x, double y);
	Point(Point& Other);
};
struct Line
{
	Point v1, v2;
	Line(Point& v1, Point& v2);
	Line(Line& Other);
};
struct Rectangle
{
	double Left, Bottom, Right, Top;
	Rectangle(double Left, double Bottom, double Right, double Top);
	Rectangle(Point& v1, Point& v2);
	Rectangle(Rectangle& Other);
};
