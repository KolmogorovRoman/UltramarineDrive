#include "Misc.h"

double PixelInMeter, GlobalScale;

Vector::Vector(){}
Vector::Vector(double x, double y):
	x(x), y(y)
{}
double Vector::Len()
{
	return sqrt(x*x+y*y);
}
void Vector::Norm()
{
	double l=Len();
	x/=l;
	y/=l;
} 
Vector operator + (Vector vct1, Vector vct2)
{
	Vector Ret;
	Ret.x=vct1.x+vct2.x;
	Ret.y=vct1.y+vct2.y;
	return Ret;
}
Vector operator - (Vector vct1, Vector vct2)
{
	Vector Ret;
	Ret.x=vct1.x-vct2.x;
	Ret.y=vct1.y-vct2.y;
	return Ret;
}
Vector operator + (Vector vct)
{
	Vector Ret;
	Ret.x=+vct.x;
	Ret.y=+vct.y;
	return Ret;
}
Vector operator - (Vector vct)
{
	Vector Ret;
	Ret.x=-vct.x;
	Ret.y=-vct.y;
	return Ret;
}
void operator += (Vector& vct1, Vector vct2)
{
	vct1=vct1+vct2;
}
void operator -= (Vector& vct1, Vector vct2)
{
	vct1=vct1-vct2;
}
Vector operator * (Vector vct, double scl)
{
	Vector Ret;
	Ret.x=vct.x*scl;
	Ret.y=vct.y*scl; 
	return Ret;
}
Vector operator / (Vector vct, double scl)
{
	Vector Ret;
	Ret.x=vct.x/scl;
	Ret.y=vct.y/scl; 
	return Ret;
}
Vector operator * (double scl, Vector vct)
{
	Vector Ret;
	Ret.x = vct.x*scl;
	Ret.y = vct.y*scl;
	return Ret;
}
Vector operator / (double scl, Vector vct)
{
	Vector Ret;
	Ret.x = vct.x / scl;
	Ret.y = vct.y / scl;
	return Ret;
}
void operator *= (Vector& vct, double scl)
{
	vct.x *= scl;
	vct.y *= scl;
}
void operator /= (Vector& vct, double scl)
{
	vct.x /= scl;
	vct.y /= scl;
}
double operator * (Vector vct1, Vector vct2)
{
	double Ret;
	Ret=vct1.x*vct2.x+vct1.y*vct2.y;
	return Ret;
}
Vector v (double x, double y)
{
	Vector Ret;
	Ret.x=x;
	Ret.y=y;
	return Ret;
}
Vector TurnV(Vector vct, double angle)
{
	return v(vct.x*dcos(angle) - vct.y*dsin(angle), vct.y*dcos(angle) + vct.x*dsin(angle));
}

TwinVector tv(double x1, double y1, double x2, double y2)
{
	TwinVector ret;
	ret.vct1=v(x1, y1);
	ret.vct2=v(x2, y2);
	return ret;
}
TwinVector tv(Vector vct1, Vector vct2)
{
	TwinVector ret;
	ret.vct1=vct1;
	ret.vct2=vct2;
	return ret;
}
TwinVector tv(Vector vct1, double x2, double y2)
{
	TwinVector ret;
	ret.vct1=vct1;
	ret.vct2=v(x2, y2);
	return ret;
}
TwinVector tv(double x1, double y1, Vector vct2)
{
	TwinVector ret;
	ret.vct1=v(x1, y1);
	ret.vct2=vct2;
	return ret;
}
TwinVector TurnTV(TwinVector Atv, double angle)
{
	TwinVector ret;
	ret.vct1=v(Atv.vct1.x*dcos(angle)-Atv.vct1.y*dsin(angle), Atv.vct1.y*dcos(angle)+Atv.vct1.x*dsin(angle));
	ret.vct2=v(Atv.vct2.x*dcos(angle)-Atv.vct2.y*dsin(angle), Atv.vct2.y*dcos(angle)+Atv.vct2.x*dsin(angle));
	return ret;
}

POINT TurnedRectSize(int sizex, int sizey, double angle)
{
	POINT ret;
	double x1, y1, x2, y2, x3, y3, xmax, ymax, xmin, ymin;
	xmax=0;
	ymax=0;
	xmin=0;
	ymin=0;
	x1=sizex*dcos(angle);
	y1=sizex*dsin(angle);
	x2=sizey*dsin(angle);
	y2=sizey*dcos(angle);
	x3=sizex*dcos(angle)+sizey*dsin(angle);
	y3=sizex*dsin(angle)+sizey*dcos(angle);
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
	return ret;
}
POINT TurnedRectCenter(int sizex, int sizey, int CenterX, int CenterY, double angle)
{
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
	tx1=x1*dcos(angle)-y1*dsin(angle);
	ty1=x1*dsin(angle)+y1*dcos(angle);
	tx2=x2*dcos(angle)-y2*dsin(angle);
	ty2=x2*dsin(angle)+y2*dcos(angle);
	tx3=x3*dcos(angle)-y3*dsin(angle);
	ty3=x3*dsin(angle)+y3*dcos(angle);
	tx4=x4*dcos(angle)-y4*dsin(angle);
	ty4=x4*dsin(angle)+y4*dcos(angle);
	ret.x=fabs(min(min(tx1, tx2), min(tx3, tx4)));
	ret.y=fabs(min(min(ty1, ty2), min(ty3, ty4)));
	return ret;
}
double dcos(double Angle)
{
	return cos(Angle*PI/180);
}
double dsin(double Angle)
{
	return sin(Angle*PI/180);
}
double dtan(double Angle)
{
	return tan(Angle*PI/180);
}
double DistToPoint(int x1, int y1, int x2, int y2)
{
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}
double AngleToPoint(int x1, int y1, int x2, int y2)
{
	if (x1==x2 && y1==y2) return 0;
	return atan2l(y2 - y1, x2 - x1)*180/PI;
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

int NeightNeumann (bool* Array, int Width, int Height, int x, int y, bool Border) //von Neumann neighborhood
{
	int Ret=0;
	int iBorder=Border?1:0;
	if (x>0) {if (Array[y*Width+x-1]==true) Ret+=1;} else Ret+=iBorder;
	if (y>0) {if (Array[(y-1)*Width+x]==true) Ret+=1;} else Ret+=iBorder;
	if (x<Width-1) {if (Array[y*Width+x+1]==true) Ret+=1;} else Ret+=iBorder;
	if (y<Height-1) {if (Array[(y+1)*Width+x]==true) Ret+=1;} else Ret+=iBorder;

	if (x>0 && y>0) {if (Array[(y-1)*Width+x-1]==true) Ret+=1;} else Ret+=iBorder;
	if (y>0 && x<Width-1) {if (Array[(y-1)*Width+x+1]==true) Ret+=1;} else Ret+=iBorder;
	if (x<Width-1 && y<Height-1) {if (Array[(y+1)*Width+x+1]==true) Ret+=1;} else Ret+=iBorder;
	if (y<Height-1 && x>0) {if (Array[(y+1)*Width+x-1]==true) Ret+=1;} else Ret+=iBorder;
	return Ret;
}

Controller::Controller()
{
	for (int i=1; i<256; i+=1) Keys[i]=0;
}
void Controller::operator=(Controller* OtherController)
{
	for (int i=1; i<=255; i+=1) Keys[i]=OtherController->Keys[i];
	Mouse.x=OtherController->Mouse.x;
	Mouse.y=OtherController->Mouse.y;
}

PointUnit::PointUnit(double Ax, double Ay, double Aangle)
{
	x=Ax;
	y=Ay;
	angle=Aangle;
}
PointUnit::PointUnit(){}
PointUnit::PointUnit(POINT Point)
{
	x = Point.x;
	y = Point.y;
}
void PointUnit::SetPoint(double x, double y, double angle)
{
	this->x = x;
	this->y = y;
	this->angle = angle;
}