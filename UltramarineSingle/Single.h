#include <stdlib.h>
#include <math.h>
#include "resource.h"
#include <windows.h>
#include <fstream>
using namespace std;

#define PI 3.141592654
#define SQRT2 1.414213562

#define MAXUSERSCOUNT 16
#define MAXOBJECTCOUNT 65536
#define MAXBULLETCOUNT 65536
#define IMAGECOUNT 16
#define MASKCOUNT 16
#define OVERGLOBAL 0
#define OVERSCREEN 1

/*#define OF_NSCOLL       0x00000001
#define OF_NCOLL        0x00000010
#define OF_NSDRAW       0x00000100
#define OF_NETTRANSLATE 0x00001000*/
#define OF_NSCOLL       0b0000000000000001
#define OF_NCOLL        0b0000000000000010
#define OF_NSDRAW       0b0000000000000100

HGLRC  hRC;
HDC hdc, hdcMem;
HWND hWnd;
WNDCLASSEX wc;
HINSTANCE hInst;
RECT RectInScreen;
int timer;
int WindowWidth, WindowHeigth, xGlobal, yGlobal;
double GlobalScale;
int IDCount, AIDCount, ImgIDCount, MaskCount, BulletsCount;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class Angle;
class Vector;
class Image;
class Text;
class Mask;
class ContMask;
class Impact;
class Object;
class Controller;
class Tank;
class Map;
class Bullet;
class Net;
class Bot;

int ObjectIDs[MAXOBJECTCOUNT];
Object* ObjectManager[MAXOBJECTCOUNT];
Image* ImageManager[IMAGECOUNT];
Mask* MaskManager[MASKCOUNT];
Map* MapManager[1];
Bullet* BulletManager[MAXBULLETCOUNT];
Text* TextStd;

class Angle
{
public:
	double value;
	double vsin;
	double vcos;
	double vtan;
	void calculate();
};
class Vector
{
public:
	double x, y;
	double Length;
	Angle angle;
	Vector(){}
	Vector(int Ax, int Ay, double ALength, double AAngleValue)
	{
		x=Ax;
		y=Ay;
		Length=ALength;
		angle.value=AAngleValue;
		angle.calculate();
	}
};
/*Vector operator + (Vector vct1, Vector vct2)
{
	Vector Ret;
	Ret.x=vct1.x+vct2.x;
	Ret.y=vct1.y+vct2.y;
}*/
POINT TurnedRectSize(int sizex, int sizey, Angle angle)
{
	angle.calculate();
	POINT ret;
	double x1, y1, x2, y2, x3, y3, xmax, ymax, xmin, ymin;
	xmax=0;
	ymax=0;
	xmin=0;
	ymin=0;
	x1=sizex*angle.vcos;
	y1=sizex*angle.vsin;
	x2=sizey*angle.vsin;
	y2=sizey*angle.vcos;
	x3=sizex*angle.vcos+sizey*angle.vsin;
	y3=sizex*angle.vsin+sizey*angle.vcos;
	if (x1>xmax) xmax=x1;
	if (x2>xmax) xmax=x2;
	if (x3>xmax) xmax=x3;
	if (y1>ymax) ymax=y1;
	if (y2>ymax) ymax=y2;
	if (y3>ymax) ymax=y3;
	if (x1<xmin) xmin=x1;
	if (x2<xmin) xmin=x2;
	if (x3<xmin) xmin=x3;
	if (y1<ymin) ymin=y1;
	if (y2<ymin) ymin=y2;
	if (y3<ymin) ymin=y3;
	ret.x=xmax-xmin;
	ret.y=ymax-ymin;
	/*SelectObject(hdcMem, GetStockObject(WHITE_PEN));
	Rectangle(hdcMem, 200, 200, 600, 600);
	SelectObject(hdcMem, GetStockObject(BLACK_PEN));
	Rectangle(hdcMem, 300+xmin, 300+ymin, 300+xmax, 300+ymax);
	MoveToEx(hdcMem, 300, 300, 0);
	LineTo(hdcMem, 300+x1, 300+y1);
	LineTo(hdcMem, 300+x3, 300+y3);
	LineTo(hdcMem, 300+x2, 300+y2);
	LineTo(hdcMem, 300, 300);*/
	return ret;
}
POINT TurnedRectCenter(int sizex, int sizey, int CenterX, int CenterY, Angle angle)
{
	angle.calculate();
	POINT ret;
	double x1, y1, x2, y2, x3, y3, x4, y4, tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4;
	x1=-CenterX;
	y1=-CenterY;
	x2=-CenterX;
	y2=sizey-CenterY;
	x3=sizex-CenterX;
	y3=sizey-CenterY;
	x4=sizex-CenterX;
	y4=-CenterY;
	tx1=x1*angle.vcos-y1*angle.vsin;
	ty1=x1*angle.vsin+y1*angle.vcos;
	tx2=x2*angle.vcos-y2*angle.vsin;
	ty2=x2*angle.vsin+y2*angle.vcos;
	tx3=x3*angle.vcos-y3*angle.vsin;
	ty3=x3*angle.vsin+y3*angle.vcos;
	tx4=x4*angle.vcos-y4*angle.vsin;
	ty4=x4*angle.vsin+y4*angle.vcos;
	ret.x=fabs(min(min(tx1, tx2), min(tx3, tx4)));
	ret.y=fabs(min(min(ty1, ty2), min(ty3, ty4)));
	return ret;
}
double DistToPoint(int x1, int y1, int x2, int y2)
{
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}
double AngleToPoint(int x1, int y1, int x2, int y2)
{
	if (x1==x2 && y1==y2) return 0;
	double vtan, ret;
	vtan=(y2-y1+0.0)/(x2-x1);
	ret=atan(vtan)*180/PI;
	if (x2<x1) ret+=180;
	return ret;
}
double AnglesDiff(double angle1, double angle2)
{
	double ret;
	ret=angle1-angle2;
	//if (ret>180) ret=ret-180;
	//if (ret<180 && ret>0) TowerAngle.value+=1.5;
	//if (ret>-180 && ret<0) TowerAngle.value-=1.5;
	//if (ret<-180) ret=ret+180
	while (ret<-180) ret+=360;
	while (ret>180) ret-=360;
	return ret;
}
void Line(int x1, int y1, int x2, int y2)
{
	MoveToEx(hdcMem, x1, y1, NULL);
	LineTo(hdcMem, x2, y2);
}
class Controller
{
public:
	bool Keys[256];
	POINT Mouse;
	Controller();
	void operator =(Controller*);
};
class Image
{
private:
	AUX_RGBImageRec *texture, *mask;
public:
	int ID;
	POINT Center;
	bool *Mask;
	int Height, Width, RealWidth, RealHeight;;
	Image(LPCTSTR, LPCTSTR, int, int, int*);
	void Draw(int, int, int, Angle, double, double, bool);
	void Draw(int, int, int, double, double, double, bool);
	~Image();
};
class Text
{
private:
	AUX_RGBImageRec* Font;
	GLuint FontTexture;
	GLubyte* Data;
	int Width, Height, ChWidth, ChHeight;
	int SymbolsCount;
public:
	class Symbol
	{
	public:
		WCHAR Symb;
		int x, y, Width, Height;
	};
	Symbol* Symbols;
	Text(LPCWSTR, int, int, int);
	void DrawSymbol(UCHAR, int, int, bool);
	void Draw(LPCWSTR, int, int, bool);
};
class Mask
{
public:
	int Width, Height, Size;
	bool *Array;
	POINT Center;
	Mask(LPCTSTR, int, int, int*);
	Mask(Mask*, Angle);
	~Mask();
};
class ContMask
{
public:
	int Width, Height, Size, PointCount;
	bool *Array;
	POINT Center;
	ContMask(Mask*);
	~ContMask();
};
bool Visible(int X1, int Y1, int X2, int Y2, Mask* Barrier, int BarX, int BarY)
{
	bool ret;
	Angle angle;
	int dir, len;
	double x, y;
	len=sqrt((X2-X1)*(X2-X1)+(Y2-Y1)*(Y2-Y1));
	angle.value=AngleToPoint(X1, Y1, X2, Y2);
	angle.calculate();
	x=X1;
	y=Y1;
	/*glLoadIdentity();
	glOrtho(-400, 400, -300, 300, -1, 1);
	glBegin(GL_POINTS);*/
	for (int i=0; i<len; i+=1)
	{
		if (Barrier->Array[(int) (x+Barrier->Center.x-BarX)+Barrier->Width*((int) (y+Barrier->Center.y-BarY))]==true)
		{
			/*glColor3f(1, 0, 0);
			glVertex3f(x, y, 0);*/
			return false;
		}
		/*else
		{
			glColor3f(0, 1, 0);
			glVertex3f(x, y, 0);
		}*/
		x+=angle.vcos;
		y+=angle.vsin;
	}
	//glEnd();
	return true;
}
bool Visible(int X, int Y, ContMask* Target, int TarX, int TarY, Mask* Barrier, int BarX, int BarY)
{
	int x, y;
	bool ret;
	for (int i=0; i<Target->Size; i+=1)
	{
		if (Target->Array[i]==0) continue;
		x=i%Target->Width;
		y=i/Target->Width;
		if (Visible(X, Y, x-Target->Center.x+TarX, y-Target->Center.y+TarY, Barrier, BarX, BarY)==true) return true;
	}
	return false;
}
class Impact
{
public:
	double x, y;
	Angle angle;
	double Force;
};
bool MaskCollCheck(Mask*, Mask*, int, int, int, int);
Impact ImpactCalc(Mask*, Mask*, ContMask*, int, int, int, int);
class Object
{
public:
	double x, y, Speed, xSpeed, ySpeed, wScale, hScale;
	//Vector Force, Impulse;
	Angle angle;
	double PrevAngleValue;
	Mask* mask;
	Image *image;
	POINT CenterSrc, Center;
	bool Deleted, Created;
	bool NeedDelete;
	WORD Flag;
	int ID, AID, ManagerID, ImageID, MaskID;
	int Type;
	virtual void ChildSteepProc(){}
	virtual bool ChildCollisionCheck(int IDSecond){return 0;}
	virtual void ChildCollisionProc(int OtherID){}
	void Create();
	Object(double, double, double, double, double, double, int, int, WORD);
	bool CollisionCheck(int);
	void SteepProc();
	bool InScreen();
public:
	virtual ~Object();
};
class Bullet:public Object
{
public:
	int ParID;
	Bullet(double, double, double, int);
	void ChildSteepProc();
	void ChildCollisionProc(int OtherID);
	~Bullet();
};
class Tank:public Object
{
public:
	Controller* Contr;
	int UserID;
	int KeyForward;
	int KeyBack;
	int KeyRight;
	int KeyLeft;
	int KeyFire;
	int HitPoints;
	int FireReady;
	LPWSTR NickName;
	int NickLen;
	Angle TowerAngle;
	Tank(int, int, int, Controller*, LPWSTR, int, int, int, int, int);
	void ChildSteepProc();
	void ChildCollisionProc(int);
	ContMask* contmask;
	Impact TankImp;
};
class Map:public Object
{
public:
	Map(int, LPCTSTR, LPCTSTR);
	ContMask* MapContMask;
	void ChildSteepProc();
	void ChildCollisionProc(int);

	int PointsCount;
	POINT* TargetPoints;
	int* NearsCounts;
	int** Net;
	int*** Pathes;
	int** PathesLen;
	int** PathesDist;
};
class Bot
{
public:
	Tank* tank;
	bool Moving;
	POINT TargetPoint;
	Bot(Tank*);
	virtual void SteepProc();
	int** Net;
	int PointsCount;
	POINT* TargetPoints;
	int* NearsCounts;
	int CurrentTargetPoint;
	int NearPoint;
	int PrevPoint;
	int* Path;
	int PathEnd;
	int PathLen;
	int PathPoint;
};