#pragma once
class Vector;
class TwinVector;
class PointUnit;
class Controller;

#include <fstream>
#include <Windows.h>
#include <Math.h>
#include <cmath>

#define PI 3.141592654
#define SQRT2 1.414213562

extern int WindowWidth, WindowHeigth, xGlobal, yGlobal;
extern double PixelInMeter, GlobalScale;

class Vector
{
public:
	double x, y;
	Vector();
	Vector(int x, int y);
	double Len();
	void Norm();
};
Vector operator + (Vector vct1, Vector vct2);
Vector operator - (Vector vct1, Vector vct2);
Vector operator + (Vector vct);
Vector operator - (Vector vct);
void operator += (Vector& vct1, Vector vct2);
void operator -= (Vector& vct1, Vector vct2);
Vector operator * (Vector vct, double scl);
Vector operator / (Vector vct, double scl);
Vector operator * (double scl, Vector vct);
Vector operator / (double scl, Vector vct);
void operator *= (Vector& vct1, double);
void operator /= (Vector& vct1, double);
double operator * (Vector vct1, Vector vct2);
Vector v(double x, double y);
Vector TurnV(Vector vct, double angle);

class TwinVector
{
public:
	Vector vct1, vct2;
};
TwinVector tv(double x1, double y1, double x2, double y2);
TwinVector tv(Vector vct1, Vector vct2);
TwinVector tv(Vector vct1, double x2, double y2);
TwinVector tv(double x1, double y1, Vector vct2);
TwinVector TurnTV(TwinVector tv, double angle);

POINT TurnedRectSize(int sizex, int sizey, double angle);
POINT TurnedRectCenter(int sizex, int sizey, int CenterX, int CenterY, double angle);
double dcos(double Angle);
double dsin(double Angle);
double dtan(double Angle);
double DistToPoint(int x1, int y1, int x2, int y2);
double AngleToPoint(int x1, int y1, int x2, int y2);
double AnglesDiff(double angle1, double angle2);
int NeightNeumann(bool* Array, int Width, int Height, int x, int y, bool Border = true); //von Neumann neighborhood

class PointUnit
{
public:
	double x, y, angle;
	PointUnit(double x, double y, double angle);
	PointUnit();
	PointUnit(POINT Point);
	void SetPoint(double x, double y, double angle);
	bool InScreen();
};
class Controller
{
public:
	BYTE Keys[256];
	POINT Mouse;
	Controller();
	void operator =(Controller* OtherController);
};