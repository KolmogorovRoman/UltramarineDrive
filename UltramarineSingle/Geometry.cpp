#include "Geometry.h"

#define MIN(a, b) (((a)<=(b))?(a):(b))
#define MAX(a, b) (((a)>=(b))?(a):(b))

Point::Point(double x, double y):
	x(x), y(y)
{}
Point::Point(Point& Other) :
	x(Other.x), y(Other.y)
{}

Line::Line(Point& v1, Point& v2) :
	v1(v1), v2(v2)
{}
Line::Line(Line& Other) :
	v1(Other.v1), v2(Other.v2)
{}

Rectangle::Rectangle(double Left, double Bottom, double Right, double Top) :
	Left(MIN(Left, Right)), Bottom(MIN(Bottom, Top)), Right(MAX(Left, Right)), Top(MAX(Bottom, Top))
{}
Rectangle::Rectangle(Point& v1, Point& v2) :
	Left(MIN(v1.x, v2.x)), Bottom(MIN(v1.y, v2.y)), Right(MAX(v1.x, v2.x)), Top(MAX(v1.y, v2.y))
{}
Rectangle::Rectangle(Rectangle& Other) :
	Left(Other.Left), Bottom(Other.Bottom), Right(Other.Right), Top(Other.Top)
{}