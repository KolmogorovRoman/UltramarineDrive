#include "Single.h"
                                 /*Tank*/ /*Bullet*/ /*Map*/
bool CollTable[3][3]={/*Tank*/       0,       1,      1,
					  /*Bullet*/     1,       0,      1,
					  /*Map*/        0,       1,      0    };

int tiv;

Controller Contr1;
Controller Contr2;
Tank* Tank1;
Tank* Tank2;
Bot* Bot1;

Map* Map1;
int TankBodyID, TankTowerID, BulletID, MapID;
int TankMaskID, Map1MaskID, BulletMaskID;
Tank* TankManager[MAXUSERSCOUNT];
int TankCount;

GLuint textures[IMAGECOUNT];
BOOL CALLBACK Dialog1Proc(HWND, UINT, WPARAM, LPARAM);

HANDLE ConsoleIn, ConsoleOut;
void Angle::calculate()
{
	while (value<0) value+=360;
	while (value>360) value-=360;
	vsin=sin(value*PI/180);
	vcos=cos(value*PI/180);
	vtan=tan(value*PI/180);
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

Mask::Mask(LPCTSTR Maskname, int ACenterX, int ACenterY, int* IDVar)
{
	Center.x=ACenterX;
	Center.y=ACenterY;
	AUX_RGBImageRec *mask;
	mask=auxDIBImageLoad(Maskname);
	Width=mask->sizeX;
	Height=mask->sizeY;
	Size=Width*Height;
	Array=new bool[Size];

	for (int i=0; i<mask->sizeX*mask->sizeY; i+=1)
	{
		if (mask->data[i*3]==255) Array[i]=1;
		if (mask->data[i*3]==0) Array[i]=0;
	}

	*IDVar=MaskCount;
	MaskManager[MaskCount]=this;
	MaskCount+=1;
}
Mask::Mask(Mask* OtherMask, Angle angle)
{
	if (angle.value!=0)
	{
		angle.calculate();
		int SrcWidth, SrcHeight;
		SrcWidth=OtherMask->Width;
		SrcHeight=OtherMask->Height;
		Width=TurnedRectSize(SrcWidth, SrcHeight, angle).x;
		Height=TurnedRectSize(SrcWidth, SrcHeight, angle).y;
		Size=Width*Height;
		POINT SrcCenter;
		SrcCenter.x=OtherMask->Center.x;
		SrcCenter.y=OtherMask->Center.y;
		Center=TurnedRectCenter(SrcWidth, SrcHeight, SrcCenter.x, SrcCenter.y, angle);
		Array=new bool[Size];
		bool *OtherArray=OtherMask->Array;
		int cx, cy;
		int xsrc, ysrc;
		double asin, acos, cyasin, cyacos;
		asin=angle.vsin;
		acos=angle.vcos;
	
		//glBegin(GL_POINTS);
		
		for (int i=0; i<Size; i+=1)
			{
				cx=i%Width;
				cy=i/Width;
				xsrc=SrcCenter.x+((cx-Center.x)*acos+(cy-Center.y)*asin);
				ysrc=SrcCenter.y+(-(cx-Center.x)*asin+(cy-Center.y)*acos);
				if (xsrc>=0 && xsrc<SrcWidth && ysrc>=0 && ysrc<SrcHeight)
					Array[i]=OtherArray[ysrc*SrcWidth+xsrc];
				else
					Array[i]=0;
				/*if (Array[i]==1)
				{
					
				}*/
			}
		//glEnd();
	}
	else
	{
		Size=OtherMask->Size;
		Width=OtherMask->Width;
		Height=OtherMask->Height;
		Center.x=OtherMask->Center.x;
		Center.y=OtherMask->Center.y;
		bool *OtherArray=OtherMask->Array;
		Array=new bool[Size];
		for (int i=0; i<Size; i+=1)
		{
			Array[i]=OtherArray[i];
			/*if (Array[i]==1)
				{
					int x, y;
					x=i%Width;
					y=i/Width;
					glLoadIdentity();
					glOrtho(-400, 400, -300, 300, -1, 1);
					glTranslatef(0, 0, 0);
					glRotatef(0, 0, 0, 1);
					glBindTexture(GL_TEXTURE_2D, 0);
					glColor3f(1, 1, 0);
					glBegin(GL_POINTS);
					glVertex3f(x, y, 0);
					glEnd();
				}*/
		}
	}
}
Mask::~Mask()
{
	delete[] Array;
}

ContMask::ContMask(Mask* MaskSrc)
{
	Center=MaskSrc->Center;
	Width=MaskSrc->Width;
	Height=MaskSrc->Height;
	Size=MaskSrc->Size;
	Array=new bool[Size];
	bool* ArraySrc=MaskSrc->Array;
	int x, y;
	PointCount=0;
	//glBegin(GL_POINTS);
	for (int i=0; i<Size; i+=1)
	{
		x=i%Width;
		y=i/Width;
		Array[i]=0;
		if (ArraySrc[i]==1)
		{
			if (x==0 || y==0 || x==Width-1|| y==Height-1)
			{
				PointCount++;
				Array[i]=1;
				continue;
			}
			if (ArraySrc[i-1]==0 || ArraySrc[i+1]==0 || ArraySrc[i-Width]==0 || ArraySrc[i+Width]==0)
			{
				PointCount++;
				Array[i]=1;
			}
		}
		/*if(Array[i]==1)
		{
			glLoadIdentity();
			glOrtho(-400, 400, -300, 300, -1, 1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glColor3f(0, 1, 0);
			glVertex3f(x, y, 0);
		}*/
	}
	//glEnd();
}
ContMask::~ContMask()
{
	delete[] Array;
}

bool MaskCollCheck(Mask* Mask1, Mask* Mask2, int x1, int y1, int x2, int y2)
{
	int xstart, cx1, cx2, dx, lx1, lx2, ystart, cy1, cy2, dy, ly1, ly2, w1, w2, wscan, h1, h2, hscan;
	w1=Mask1->Width;
	w2=Mask2->Width;
	h1=Mask1->Height;
	h2=Mask2->Height;
	cx1=Mask1->Center.x;
	cx2=Mask2->Center.x;
	cy1=Mask1->Center.y;
	cy2=Mask2->Center.y;

	dx=abs(x1-x2);
	dy=abs(y1-y2);
	if (x1-cx1<=x2-cx2)
	{
		xstart=x2-cx2;
		wscan=(w1-cx1+cx2)-dx;
		lx1=xstart-x1+cx1;
		lx2=0;
	}
	else
	{
		xstart=x1-cx1;
		wscan=(cx1+w2-cx2)-dx;
		lx1=0;
		lx2=xstart-x2+cx2;
	}
	if (y1-cy1<=y2-cy2)
	{
		ystart=y2-cy2;
		hscan=(h1-cy1+cy2)-dy;
		ly1=ystart-y1+cy1;
		ly2=0;
	}
	else
	{
		ystart=y1-cy1;
		hscan=(cy1+h2-cy2)-dy;
		ly1=0;
		ly2=ystart-y2+cy2;
	}
	if (wscan<=0 || hscan<=0) return 0;
	if (x1-cx1>x2-cx2 && x1+w1-cx1<x2+w2-cx2) wscan=w1;
	if (x2-cx2>x1-cx1 && x2+w2-cx2<x1+w1-cx1) wscan=w2;
	if (y1-cy1>y2-cy2 && y1+h1-cy1<y2+h2-cy2) hscan=h1;
	if (y2-cy2>y1-cy1 && y2+h2-cy2<y1+h1-cy1) hscan=h2;
	for (int y=0; y<hscan; y+=1)
		for(int x=0; x<wscan; x+=1)
		{
			if (Mask1->Array[lx1+x+(ly1+y)*w1]==1 && Mask2->Array[lx2+x+(ly2+y)*w2]==1)
			{
				return 1;
				/*glLoadIdentity();
				glOrtho(-400, 400, -300, 300, -1, 1);
				glTranslatef(x1-cx1, y1-cy1, 0);
				glRotatef(0, 0, 0, 1);
				glBindTexture(GL_TEXTURE_2D, 0);
				glColor3f(1, 1, 0);
				glBegin(GL_POINTS);
				glVertex3f(x, y, 0);
				glEnd();*/
			}
		}
		return 0;
}
Impact ImpactCalc(Mask* Mask1, Mask* Mask2, ContMask* ContMask1, int x1, int y1, int x2, int y2)
{
	int xstart, cx1, cx2, dx, lx1, lx2, ystart, cy1, cy2, dy, ly1, ly2, w1, w2, wscan, h1, h2, hscan;
	w1=Mask1->Width;
	w2=Mask2->Width;
	h1=Mask1->Height;
	h2=Mask2->Height;
	cx1=Mask1->Center.x;
	cx2=Mask2->Center.x;
	cy1=Mask1->Center.y;
	cy2=Mask2->Center.y;

	dx=abs(x1-x2);
	dy=abs(y1-y2);
	if (x1-cx1<=x2-cx2)
	{
		xstart=x2-cx2;
		wscan=(w1-cx1+cx2)-dx;
		lx1=xstart-x1+cx1;
		lx2=0;
	}
	else
	{
		xstart=x1-cx1;
		wscan=(cx1+w2-cx2)-dx;
		lx1=0;
		lx2=xstart-x2+cx2;
	}
	if (y1-cy1<=y2-cy2)
	{
		ystart=y2-cy2;
		hscan=(h1-cy1+cy2)-dy;
		ly1=ystart-y1+cy1;
		ly2=0;
	}
	else
	{
		ystart=y1-cy1;
		hscan=(cy1+h2-cy2)-dy;
		ly1=0;
		ly2=ystart-y2+cy2;
	}
	if (wscan<=0 || hscan<=0)
	if (x1-cx1>x2-cx2 && x1+w1-cx1<x2+w2-cx2) wscan=w1;
	if (x2-cx2>x1-cx1 && x2+w2-cx2<x1+w1-cx1) wscan=w2;
	if (y1-cy1>y2-cy2 && y1+h1-cy1<y2+h2-cy2) hscan=h1;
	if (y2-cy2>y1-cy1 && y2+h2-cy2<y1+h1-cy1) hscan=h2;

	Impact Ret;
	Ret.Force=0;
	Ret.angle.value=0;
	Ret.x=0;
	Ret.y=0;
	POINT vector;
	vector.x=0; vector.y=0;
	bool* ContMask1Array=ContMask1->Array;
	bool* Mask1Array=Mask1->Array;
	bool* Mask2Array=Mask2->Array;
	for (int y=0; y<hscan; y+=1)
	{
		for(int x=0; x<wscan; x+=1)
		{
			if (Mask1Array[lx1+x+(ly1+y)*w1]==1 && Mask2Array[lx2+x+(ly2+y)*w2]==1)
			{
				Ret.Force+=1;
				Ret.x+=x;
				Ret.y+=y;
				if (ContMask1Array[lx1+x+(ly1+y)*w1]==1)
				{
					if (lx1+x+1<w1 && Mask1Array[lx1+x+1+(ly1+y)*w1]==1) {vector.x+=1;}
					if (lx1+x+1<w1 && ly1+y+1<h1 && Mask1Array[lx1+x+1+(ly1+y+1)*w1]==1) {vector.x+=SQRT2; vector.y+=SQRT2;}
					if (ly1+y+1<h1 && Mask1Array[lx1+x+(ly1+y+1)*w1]==1) {vector.y+=1;}
					if (lx1+x-1>=0 && ly1+y+1<h1 && Mask1Array[lx1+x-1+(ly1+y+1)*w1]==1) {vector.x-=SQRT2; vector.y+=SQRT2;}
					if (lx1+x-1>=0 && Mask1Array[lx1+x-1+(ly1+y)*w1]==1) {vector.x-=1;}
					if (lx1+x-1>=0 && ly1+y-1>=0 && Mask1Array[lx1+x-1+(ly1+y-1)*w1]==1) {vector.x-=SQRT2; vector.y-=SQRT2;}
					if (ly1+y-1>=0 && Mask1Array[lx1+x+(ly1+y-1)*w1]==1) {vector.y-=1;}
					if (lx1+x+1<w1 && ly1+y-1>=0 && Mask1Array[lx1+x+1+(ly1+y-1)*w1]==1) {vector.x+=SQRT2; vector.y-=SQRT2;}
				}
				/*glLoadIdentity();
				glOrtho(-400, 400, -300, 300, -1, 1);
				glTranslatef(0, 0, 0);
				glRotatef(0, 0, 0, 1);
				glBindTexture(GL_TEXTURE_2D, 0);
				glColor3f(1, 1, 0);
				glBegin(GL_POINTS);
				glVertex3f(x2-cx2+x, y2-cy2+y, 0);
				glEnd();*/
			}
		}
	}
	Ret.x=Ret.x/Ret.Force;
	Ret.y=Ret.y/Ret.Force;
	if (Ret.Force==0) {Ret.x=0; Ret.y=0;}
	Ret.angle.value=AngleToPoint(0, 0, -vector.x, -vector.y);
	Ret.angle.calculate();
	return Ret;
}

Image::Image(LPCTSTR Name, LPCTSTR Maskname, int ACenterx, int ACentery, int* IDVar)
{
	ID=ImgIDCount;
	ImgIDCount+=1;
	ImageManager[ID]=this;
	*IDVar=ID;

	GLubyte* Data;
	texture=auxDIBImageLoad(Name);
	mask=auxDIBImageLoad(Maskname);
	RealWidth=texture->sizeX;
	RealHeight=texture->sizeY;
	Width=pow(2, ceil(log(RealWidth+2.0)/log(2.0)));
	Height=pow(2, ceil(log(RealHeight+2.0)/log(2.0)));
	Center.x=ACenterx+1;
	Center.y=ACentery+1;
	Data=new GLubyte[Height*Width*4];
	for (int i=0; i<Height*Width*4; i+=1)
	{
		Data[i]=0;
	}
	int i1=0;
	for (int iy=0; iy<RealHeight; iy+=1)
		for (int ix=0; ix<RealWidth; ix+=1)
		{
			Data[((iy+1)*Width+ix+1)*4]=texture->data[i1];
			Data[((iy+1)*Width+ix+1)*4+1]=texture->data[i1+1];
			Data[((iy+1)*Width+ix+1)*4+2]=texture->data[i1+2];
			Data[((iy+1)*Width+ix+1)*4+3]=mask->data[i1];
			i1+=3;
		}
	/*for (int i=0; i<SrcHeight*SrcWidth; i+=1)
	{
		Data[i*4]=texture->data[i1];
		Data[i*4+1]=texture->data[i1+1];
		Data[i*4+2]=texture->data[i1+2];
		Data[i*4+3]=mask->data[i1];
		i1+=3;
	}*/
	glGenTextures(1, &textures[ID]);
	glBindTexture(GL_TEXTURE_2D, textures[ID]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA , GL_UNSIGNED_BYTE, Data);
}
void Image::Draw(int x, int y, int z, Angle angle, double wScale, double hScale, bool OverScreen)
{
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, 0, -100);
	glTranslatef(x, y, z);
	glRotatef(angle.value, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, textures[ID]);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);

	/*glTexCoord2f(0, 0); glVertex3f(-Center.x, -Center.y, 0);
	glTexCoord2f(0, (RealHeight+0.0)/Height); glVertex3f(-Center.x, RealHeight-Center.y, 0);
	glTexCoord2f((RealWidth+0.0)/Width, (RealHeight+0.0)/Height); glVertex3f(RealWidth-Center.x, RealHeight-Center.y, 0);
	glTexCoord2f((RealWidth+0.0)/Width, 0); glVertex3f(RealWidth-Center.x, -Center.y, 0);*/
	glTexCoord2f(0, 0); glVertex3f(-Center.x, -Center.y, 0);
	glTexCoord2f(0, 1); glVertex3f(-Center.x, Height-Center.y, 0);
	glTexCoord2f(1, 1); glVertex3f(Width-Center.x, Height-Center.y, 0);
	glTexCoord2f(1, 0); glVertex3f(Width-Center.x, -Center.y, 0);
	glEnd();
}
void Image::Draw(int x, int y, int z, double AngleValue, double wScale, double hScale, bool OverScreen)
{
	Angle angle;
	angle.value=AngleValue;
	angle.calculate();
	Draw(x, y, z, angle, wScale, hScale, OverScreen);
}
Image::~Image()
{
	delete[] Mask;
}

Text::Text(LPCWSTR FontFileBMP, int ASymbolsCount, int AChWidth, int AChHeight)
{
	SymbolsCount=ASymbolsCount;
	ChWidth=AChWidth;
	ChHeight=AChHeight;
	Font=auxDIBImageLoad(FontFileBMP);
	Width=Font->sizeX;
	Height=Font->sizeY;
	int i1=0;
	Data=new GLubyte[Width*Height*4];
	for (int i=0; i<Width*Height; i+=1)
	{
		Data[i*4+3]=255;
		if(Font->data[i1*3+0]==255 && Font->data[i1*3+1]==255 && Font->data[i1*3+2]==255)
			{Data[i*4+0]=255; Data[i*4+1]=255; Data[i*4+2]=255;}

			if(Font->data[i1*3+0]==0 && Font->data[i1*3+1]==255 && Font->data[i1*3+2]==0)
			{Data[i*4+0]=0; Data[i*4+1]=255; Data[i*4+2]=0;}

		if(Font->data[i1*3+0]==255 && Font->data[i1*3+1]==0 && Font->data[i1*3+2]==0)
			{Data[i*4+0]=255; Data[i*4+1]=0; Data[i*4+2]=0;}
		if(Font->data[i1*3+0]==0 && Font->data[i1*3+1]==0 && Font->data[i1*3+2]==255) Data[i*4+3]=0;
		i1+=1;
	}
	glGenTextures(1, &FontTexture);
	glBindTexture(GL_TEXTURE_2D, FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA , GL_UNSIGNED_BYTE, Data);
	Symbols=new Symbol[SymbolsCount];
	for (WCHAR i=0; i<SymbolsCount; i+=1)
	{
		Symbols[i].x=(i*ChWidth)%Width;
		Symbols[i].y=Height-(i*ChWidth)/(Width)*ChWidth-ChHeight;
		Symbols[i].Height=ChHeight;
		Symbols[i].Width=0;
		while (Data[((Symbols[i].y+1)*Width+Symbols[i].x+Symbols[i].Width)*4]!=255)
			Symbols[i].Width+=1;
		//memcpy(&Symbols[i].Symb, &i, 2);
		Symbols[i].Symb=i;
	}
}
void Text::DrawSymbol(UCHAR Symb, int x, int y, bool Center)
	{
		glLoadIdentity();
		glOrtho(-400, 400, -300, 300, 0, -100);
		glTranslatef(x, y, 0);
		glBindTexture(GL_TEXTURE_2D, FontTexture);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);

		glTexCoord2f((Symbols[Symb].x*1.0+1)/Width, (Symbols[Symb].y+1.0)/Height);
		glVertex3f(-Symbols[Symb].Width/2.0, -Symbols[Symb].Height/2.0+1, 0);

		glTexCoord2f((Symbols[Symb].x*1.0+1)/Width, (Symbols[Symb].y*1.0+Symbols[Symb].Height)/Height);
		glVertex3f(-Symbols[Symb].Width/2.0, Symbols[Symb].Height/2.0, 0);

		glTexCoord2f((Symbols[Symb].x*1.0+Symbols[Symb].Width)/Width, (Symbols[Symb].y*1.0+Symbols[Symb].Height)/Height);
		glVertex3f(Symbols[Symb].Width/2.0, Symbols[Symb].Height/2.0, 0);

		glTexCoord2f((Symbols[Symb].x*1.0+Symbols[Symb].Width)/Width, (Symbols[Symb].y+1.0)/Height);
		glVertex3f(Symbols[Symb].Width/2.0, -Symbols[Symb].Height/2.0+1, 0);

		glEnd();
	}
void Text::Draw(LPCWSTR String, int x, int y, bool Center)
	{
		int TotalWidth, Len, cx;
		TotalWidth=0;
		UCHAR* AString=new UCHAR;
		Len=WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, String, -1, (LPSTR) AString, 0, 0, 0);
		WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, String, -1, (LPSTR) AString, Len, 0, 0);
		for (int i=0; i<Len; i+=1)
		{
			TotalWidth+=Symbols[AString[i]].Width;
		}
		cx=-TotalWidth/2;
		for (int i=0; i<Len; i+=1)
		{
			DrawSymbol(AString[i], x+cx+Symbols[AString[i]].Width/2, y, 0);
			cx+=Symbols[AString[i]].Width;
		}
	}

void Object::Create()
{
	ID=IDCount;
	ObjectManager[ID]=this;
	IDCount+=1;
}
Object::Object(double x0, double y0, double ACelterX, double ACelterY, double ASpeed, double AngleValue0, int AImageID, int AMaskID, WORD AFlag)
{
	Created=0;
	x=x0;
	y=y0;
	angle.value=AngleValue0;
	angle.calculate();
	Speed=ASpeed;
	Flag=AFlag;
	xSpeed=Speed*angle.vcos;
	ySpeed=Speed*angle.vsin;
	wScale=1;
	hScale=1;
	ImageID=AImageID;
	if (ImageID!=-1) image=ImageManager[ImageID];
	NeedDelete=0;
	Deleted=0;
	Type=0;

	CenterSrc.x=ACelterX;
	CenterSrc.y=ACelterY;
	MaskID=AMaskID;
	mask=new Mask(MaskManager[MaskID], angle);

	ID=IDCount;
	AID=AIDCount;
	ObjectIDs[ID]=AID;
	ObjectManager[ID]=this;
	IDCount+=1;
	AIDCount+=1;
	Created=1;
}
bool Object::CollisionCheck(int IDSecond)
{
	if (Flag & OF_NCOLL || ObjectManager[IDSecond]->Flag & OF_NCOLL) return 0;
	if (CollTable[Type][ObjectManager[IDSecond]->Type]==0) return 0;
	if (Flag & OF_NSCOLL) return ChildCollisionCheck(IDSecond);
	if (ObjectManager[IDSecond]->Flag & OF_NSCOLL) return ObjectManager[IDSecond]->ChildCollisionCheck(ID);

	return MaskCollCheck(mask, ObjectManager[IDSecond]->mask, x, y, ObjectManager[IDSecond]->x, ObjectManager[IDSecond]->y);
}
void Object::SteepProc()
{
	if (Created==0) return;
	ChildSteepProc();
	if (Deleted==true)
	{
		delete this;
		return;
	}
	angle.calculate();
	if ((ImageID!=-1) && ((Flag & OF_NSDRAW)==0))
		image->Draw(x, y, 0, angle, wScale, hScale, OVERGLOBAL);

	if (angle.value!=PrevAngleValue)
	{
		delete mask;
		mask=new Mask(MaskManager[MaskID], angle);
	}
	//x+=Impulse.Length*Impulse.angle.vcos;
	//y+=Impulse.Length*Impulse.angle.vsin;
	PrevAngleValue=angle.value;
}
bool Object::InScreen()
{
	if (x+WindowWidth/2<0 || y+WindowHeigth/2<0 || x-WindowWidth/2>=WindowWidth || y-WindowHeigth/2>=WindowHeigth) return false;
	else return true;
}
Object::~Object()
{
	Deleted=1;
	IDCount-=1;
	if (ID!=IDCount)
	{
		ObjectManager[ID]=ObjectManager[IDCount];
		ObjectManager[ID]->ID=ID;
	}
	ObjectManager[IDCount]=NULL;
	if (ImageID!=-1) 
	{
		image=0;
		delete mask;
	}
}

Bullet::Bullet(double Ax, double Ay, double AAngleValue, int AParID)
	:Object(Ax, Ay, 12, 4, 5, AAngleValue, BulletID, BulletMaskID, 0/*|OF_NCOLL*/)
{
	ParID=AParID;
	Type=1;
	ManagerID=BulletsCount;
	BulletManager[ManagerID]=this;
	BulletsCount+=1;
}
void Bullet::ChildSteepProc()
{
	if (InScreen()==0) Deleted=1;
	angle.calculate();
	x+=15*angle.vcos;
	y+=15*angle.vsin;
}
void Bullet::ChildCollisionProc(int OtherID)
{
	if (ObjectManager[OtherID]->Type==0 && OtherID!=ParID)
		Deleted=1;
	if (ObjectManager[OtherID]->Type==2)
		Deleted=1;
}
Bullet::~Bullet()
{
	if(ManagerID!=BulletsCount)
		BulletManager[ManagerID]=BulletManager[BulletsCount];
	BulletManager[ManagerID]=0;
}

Tank::Tank(int Ax, int Ay, int Aangle, Controller* AContr, LPWSTR ANickName, int AKeyForward, int AKeyBack, int AKeyRight, int AKeyLeft, int AKeyFire)
	:Object(Ax, Ay, 36, 24, 0, Aangle, -1, TankMaskID, OF_NSDRAW)
{
	Type=0;
	Contr=AContr;
	NickName=ANickName;
	KeyForward=AKeyForward;
	KeyBack=AKeyBack;
	KeyRight=AKeyRight;
	KeyLeft=AKeyLeft;
	KeyFire=AKeyFire;
	TowerAngle.value=Aangle;
	HitPoints=5;
	FireReady=150;
	contmask=new ContMask(mask);
	TankManager[TankCount]=this;
	ManagerID=TankCount;
	TankCount+=1;
}
void Tank::ChildSteepProc()
{
	FireReady+=1;
	if (FireReady>150) FireReady=150;
	if (Contr->Keys[KeyFire]==1 && FireReady>=150) {new Bullet(x, y, TowerAngle.value, ID); FireReady=0;}
	Speed=0;
	if (Contr->Keys[KeyForward]==1) Speed=1.5;
	if (Contr->Keys[KeyBack]==1) Speed=-1;
	if (Contr->Keys[KeyRight]==1)
	{
		angle.value-=2;
		TowerAngle.value-=2;
	}
	if (Contr->Keys[KeyLeft]==1)
	{
		angle.value+=2;
		TowerAngle.value+=2;
	}
	angle.calculate();
	x+=Speed*angle.vcos;
	y+=Speed*angle.vsin;

	Impact impact=ImpactCalc(Map1->mask, mask, MapManager[0]->MapContMask, Map1->x, Map1->y, x, y);
	impact.angle.calculate();
	impact.x-=mask->Center.x;
	impact.y-=mask->Center.y;

	if (impact.Force!=0)
	{
		if (impact.Force>10) impact.Force=10;
		double DAngle, NormAngle;
		NormAngle=AngleToPoint(0, 0, impact.x, impact.y);
		DAngle=impact.Force*cos((impact.angle.value-90-NormAngle)*PI/180)/2;
		angle.value+=DAngle;
		TowerAngle.value+=DAngle;
		angle.calculate();
		x+=impact.Force*impact.angle.vcos/6;
		y+=impact.Force*impact.angle.vsin/6;
	}
	/*glLoadIdentity();
	glOrtho(-400, 400, -300, 300, -1, 1);
	glTranslatef(x, y, 0);
	glRotatef(0, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(1, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(impact.x, impact.y, 0);
	glVertex3f(impact.x+impact.Force*impact.angle.vcos*10, impact.y+impact.Force*impact.angle.vsin*10, 0);
	glEnd();*/

	double TargetAngle;
	TargetAngle=AngleToPoint(x, y, Contr->Mouse.x, Contr->Mouse.y);
	if(AnglesDiff(TargetAngle, TowerAngle.value)<1) TowerAngle.value-=1.5;
	if(AnglesDiff(TargetAngle, TowerAngle.value)>1) TowerAngle.value+=1.5;
	TowerAngle.calculate();

	angle.calculate();
	xSpeed=Speed*angle.vcos;
	ySpeed=Speed*angle.vsin;

	delete contmask;
	contmask=new ContMask(mask);
	ImageManager[TankBodyID]->Draw(x, y, 98, angle, 1, 1, 0);
	ImageManager[TankTowerID]->Draw(x, y, 97, TowerAngle, 1, 1, 0);
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, 0, -100);
	glTranslatef(x, y, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(1-HitPoints/5.0, HitPoints/5.0, 0);
	glBegin(GL_QUADS);
	glVertex3f(-50, -50, 1);
	glVertex3f(100*HitPoints/5-50, -50, 1);
	glVertex3f(100*HitPoints/5-50, -45, 1);
	glVertex3f(-50, -45, 1);
	glEnd();
	glColor3f(0, 0, 0);
	glLineStipple(5, 1);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-50, -50, 0);
	glVertex3f(50, -50, 0);
	glVertex3f(50, -45, 0);
	glVertex3f(-50, -45, 0);
	glEnd();
	if (this==Tank1)
	{
		glLoadIdentity();
		glOrtho(-400, 400, -300, 300, 0, -100);
		glTranslatef(0, 0, 1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.5, 0, 1);
		glBegin(GL_QUADS);
		glVertex3f(-400, -300, 1);
		glVertex3f(100*FireReady/150-400, -300, 1);
		glVertex3f(100*FireReady/150-400, -290, 1);
		glVertex3f(-400, -290, 1);
		glEnd();
		glColor3f(0, 0, 0);
		glLineStipple(5, 1);
		glBegin(GL_LINE_LOOP);
		glVertex3f(-400, -300, 0);
		glVertex3f(-300, -300, 0);
		glVertex3f(-300, -290, 0);
		glVertex3f(-400, -290, 0);
		glEnd();

		glColor3f(1-HitPoints/5.0, HitPoints/5.0, 0);
		glBegin(GL_QUADS);
		glVertex3f(-400, -280, 1);
		glVertex3f(100*HitPoints/5-400, -280, 1);
		glVertex3f(100*HitPoints/5-400, -290, 1);
		glVertex3f(-400, -290, 1);
		glEnd();
		glColor3f(0, 0, 0);
		glLineStipple(5, 1);
		glBegin(GL_LINE_LOOP);
		glVertex3f(-400, -280, 0);
		glVertex3f(-300, -280, 0);
		glVertex3f(-300, -290, 0);
		glVertex3f(-400, -290, 0);
		glEnd();

		glColor3f(1, 1, 0);
		glLineStipple(5, 1);
		glBegin(GL_LINE_LOOP);
		glVertex3f(x, y, 0);
		glVertex3f(Contr->Mouse.x, Contr->Mouse.y, 0);
		glVertex3f(x, y, 0);
		glVertex3f(Contr->Mouse.x, Contr->Mouse.y, 0);
		glEnd();
	}
	TextStd->Draw(NickName, x, y+50, 1);
	if (HitPoints<=0)
	{
		Deleted=1;
		//new Tank(0, 0, 0, Contr, UserID, 87, 83, 68, 65, 1);
		if (this==Tank1) Tank1=new Tank(0, 0, 0, &Contr1, L"Player", 87, 83, 68, 65, 1);
		if (this==Tank2)
		{
			Tank2=new Tank(-320, 0, 0, &Contr2, L"Alpha", 87, 83, 68, 65, 1);
			Bot1=new Bot(Tank2);
		}
	}
}
void Tank::ChildCollisionProc(int OtherID)
{
	if (ObjectManager[OtherID]->Type==1 && BulletManager[ObjectManager[OtherID]->ManagerID]->ParID!=ID)
		HitPoints-=1;
	if (ObjectManager[OtherID]->Type==0)
	{
		TankImp=ImpactCalc(TankManager[ObjectManager[OtherID]->ManagerID]->mask, mask, TankManager[ObjectManager[OtherID]->ManagerID]->contmask, ObjectManager[OtherID]->x, ObjectManager[OtherID]->y, x, y);
		TankImp.angle.calculate();
		TankImp.x-=mask->Center.x;
		TankImp.y-=mask->Center.y;
		if (TankImp.Force!=0)
		{
			if (TankImp.Force>10) TankImp.Force=10;
			double DAngle, NormAngle;
			NormAngle=AngleToPoint(0, 0, TankImp.x, TankImp.y);
			DAngle=TankImp.Force*cos((TankImp.angle.value-90-NormAngle)*PI/180)/2;
			angle.value+=DAngle;
			TowerAngle.value+=DAngle;
			angle.calculate();
			x+=TankImp.Force*TankImp.angle.vcos/6;
			y+=TankImp.Force*TankImp.angle.vsin/6;
		}
	}
}

Map::Map(int MapImageID, LPCTSTR MapMask, LPCTSTR AIPoints)
	:Object(0, 0, 400, 300, 0, 0, /*MapImageID*/-1, Map1MaskID, 0)
{
	Type=2;
	MapContMask=new ContMask(mask);
	MapManager[0]=this;

	ifstream fin;
	fin.open(AIPoints);
	fin>>PointsCount;
	Net=new int*[PointsCount];
	TargetPoints=new POINT[PointsCount];
	for(int i=0; i<PointsCount; i+=1)
	{
		fin>>TargetPoints[i].x;
		fin>>TargetPoints[i].y;
	}
	NearsCounts=new int[PointsCount];
	for (int i=0; i<PointsCount; i+=1)
	{
		fin>>NearsCounts[i];
		Net[i]=new int[NearsCounts[i]];
		for (int i1=0; i1<NearsCounts[i]; i1+=1)
		{
			fin>>Net[i][i1];
		}
	}
	Pathes=new int**[PointsCount];
	PathesLen=new int*[PointsCount];
	PathesDist=new int*[PointsCount];
	for (int i=0; i<PointsCount; i+=1)
	{
		Pathes[i]=new int*[PointsCount];
		PathesLen[i]=new int[PointsCount];
		PathesDist[i]=new int[PointsCount];
		for (int i1=0; i1<PointsCount; i1+=1)
		{
			fin>>PathesLen[i][i1];
			Pathes[i][i1]=new int[PathesLen[i][i1]];
			for (int i2=0; i2<PathesLen[i][i1]; i2+=1)
			{
				fin>>Pathes[i][i1][i2];
			}
			fin>>PathesDist[i][i1];
		}
	}
}
void Map::ChildCollisionProc(int OtherID)
{
}
void Map::ChildSteepProc()
{
	ImageManager[MapID]->Draw(x, y, 99, 0, 1, 1, 0);
}

Bot::Bot(Tank* Atank)
{
	tank=Atank;
	Net=Map1->Net;
	PointsCount=Map1->PointsCount;
	NearsCounts=Map1->NearsCounts;
	TargetPoints=Map1->TargetPoints;
	CurrentTargetPoint=2;
	TargetPoint=TargetPoints[CurrentTargetPoint];
	PrevPoint=-1;
	PathLen=0;
	PathPoint=0;
	PathEnd=2;
}
void Bot::SteepProc()
{
	tank->Contr->Mouse.x=Tank1->x;
	tank->Contr->Mouse.y=Tank1->y;
	tank->Contr->Keys[tank->KeyFire]=false;
	if (Visible(tank->x, tank->y, Tank1->x, Tank1->y, Map1->mask, Map1->x, Map1->y))
	{
		Moving=0;
		if (fabs(AnglesDiff(tank->TowerAngle.value, AngleToPoint(tank->x, tank->y, Tank1->x, Tank1->y)))<10)
			tank->Contr->Keys[tank->KeyFire]=true;
	}
	else
	{
		//tank->Contr->Mouse.x=tank->x+100*tank->angle.vcos;
		//tank->Contr->Mouse.y=tank->y+100*tank->angle.vsin;
		int MinDist;
		MinDist=10000;
		int MinPoint;
		MinPoint=-1;
		bool Back;
		for (int i=0; i<PointsCount; i+=1)
		{
			if (Visible(TargetPoints[i].x, TargetPoints[i].y, Tank1->x, Tank1->y, Map1->mask, Map1->x, Map1->y))
			{
				if(Map1->PathesDist[CurrentTargetPoint][i]+DistToPoint(tank->x, tank->y, TargetPoints[CurrentTargetPoint].x, TargetPoints[CurrentTargetPoint].y)<MinDist)
				{
					MinDist=Map1->PathesDist[CurrentTargetPoint][i];
					MinPoint=i;
					Back=false;
				}
				if(PrevPoint!=-1 && Map1->PathesDist[PrevPoint][i]+DistToPoint(tank->x, tank->y, TargetPoints[PrevPoint].x, TargetPoints[PrevPoint].y)<MinDist)
				{
					MinDist=Map1->PathesDist[PrevPoint][i];
					MinPoint=i;
					Back=true;
				}
			}
		}
		if (MinPoint==-1) MinPoint=5;
		PathPoint=0;
		if (Back==false)
		{
			Path=Map1->Pathes[CurrentTargetPoint][MinPoint];
			PathLen=Map1->PathesLen[CurrentTargetPoint][MinPoint];
		}
		if (Back==true)
		{
			CurrentTargetPoint=PrevPoint;
			Path=Map1->Pathes[PrevPoint][MinPoint];
			PathLen=Map1->PathesLen[PrevPoint][MinPoint];
		}
	}
	tank->Contr->Keys[tank->KeyRight]=false;
	tank->Contr->Keys[tank->KeyLeft]=false;
	tank->Contr->Keys[tank->KeyForward]=false;
	tank->Contr->Keys[tank->KeyBack]=false;
	if (DistToPoint(tank->x, tank->y, TargetPoint.x, TargetPoint.y)<=10)
	{
		/*int Near;
		do Near=rand()%NearsCounts[CurrentTargetPoint];
		while (Net[CurrentTargetPoint][Near]==PrevPoint && NearsCounts[CurrentTargetPoint]!=1);
		PrevPoint=CurrentTargetPoint;
		CurrentTargetPoint=Net[CurrentTargetPoint][Near];
		TargetPoint=TargetPoints[CurrentTargetPoint];*/
		if (PathPoint<PathLen)
		{
			PrevPoint=CurrentTargetPoint;
			CurrentTargetPoint=Path[PathPoint];
			PathPoint+=1;
		}
		Moving=0;
	}
	else Moving=1;
	TargetPoint=TargetPoints[CurrentTargetPoint];
	double TargetAngle;
	TargetAngle=AngleToPoint(tank->x, tank->y, TargetPoint.x, TargetPoint.y);
	if (AnglesDiff(TargetAngle, tank->angle.value)>3) tank->Contr->Keys[tank->KeyLeft]=true;
	if (AnglesDiff(TargetAngle, tank->angle.value)<-3) tank->Contr->Keys[tank->KeyRight]=true;
	if (fabs(AnglesDiff(TargetAngle, tank->angle.value))>90)
	{
		tank->Contr->Keys[tank->KeyLeft]=!tank->Contr->Keys[tank->KeyLeft];
		tank->Contr->Keys[tank->KeyRight]=!tank->Contr->Keys[tank->KeyRight];
	}
	if (fabs(AnglesDiff(TargetAngle, tank->angle.value))<30 && Moving==1) tank->Contr->Keys[tank->KeyForward]=true;
	if (fabs(AnglesDiff(TargetAngle, tank->angle.value))>150 && Moving==1) tank->Contr->Keys[tank->KeyBack]=true;
	/*if (PathLen>0)
	{
		glLoadIdentity();
		glOrtho(-400, 400, -300, 300, -1, 1);
		glTranslatef(0, 0, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0, 0, 1);
		glBegin(GL_QUADS);
		glVertex3f(TargetPoint.x-5, TargetPoint.y-5, -0.5);
		glVertex3f(TargetPoint.x+5, TargetPoint.y-5, -0.5);
		glVertex3f(TargetPoint.x+5, TargetPoint.y+5, -0.5);
		glVertex3f(TargetPoint.x-5, TargetPoint.y+5, -0.5);
		glEnd();
	}*/
}

int SetWindowPixelFormat()
{
    int m_GLPixelIndex;
    PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion    = 1;
    pfd.dwFlags   = PFD_DRAW_TO_WINDOW |
                    PFD_SUPPORT_OPENGL |
                    PFD_DOUBLEBUFFER;
    pfd.iPixelType     = PFD_TYPE_RGBA;
    pfd.cColorBits     = 32;
    pfd.cRedBits       = 8;
    pfd.cRedShift      = 16;
    pfd.cGreenBits     = 8;
    pfd.cGreenShift    = 8;
    pfd.cBlueBits      = 8;
    pfd.cBlueShift     = 0;
    pfd.cAlphaBits     = 0;
    pfd.cAlphaShift    = 0;
    pfd.cAccumBits     = 64;
    pfd.cAccumRedBits  = 16;
    pfd.cAccumGreenBits   = 16;
    pfd.cAccumBlueBits    = 16;
    pfd.cAccumAlphaBits   = 0;
    pfd.cDepthBits        = 32;
    pfd.cStencilBits      = 8;
    pfd.cAuxBuffers       = 0;
    pfd.iLayerType        = PFD_MAIN_PLANE;
    pfd.bReserved         = 0;
    pfd.dwLayerMask       = 0;
    pfd.dwVisibleMask     = 0;
    pfd.dwDamageMask      = 0;
    m_GLPixelIndex = ChoosePixelFormat(hdc, &pfd);
    //DescribePixelFormat(hdc,m_GLPixelIndex,sizeof(PIXELFORMATDESCRIPTOR),&pfd);
    SetPixelFormat( hdc, m_GLPixelIndex, &pfd);
    return 1;
}
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hMainWnd;
	MSG msg;
	wc.cbSize        = sizeof(wc);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = L"Class1";
	wc.cbWndExtra    = NULL;
	wc.cbClsExtra    = NULL;
	wc.hIcon         = LoadIcon(NULL, L"LOGO");
	wc.hIconSm       = LoadIcon(NULL, L"LOGO");
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hInstance     = hinst;
	RegisterClassEx(&wc);

	hInst=hinst;
	RECT rect={0, 0, 800, 600};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);

	tiv=GetSystemMetrics(SM_CXSIZE);
	hMainWnd = CreateWindow(
		L"Class1",
		L"UltramarineTanks Single 0.1",
		WS_OVERLAPPEDWINDOW,
		500,
		100,
		rect.right-rect.left,
		rect.bottom-rect.top,
		//800+2*GetSystemMetrics(SM_CXSIZEFRAME),
		//600+GetSystemMetrics(SM_CXSIZE)+GetSystemMetrics(SM_CYSIZEFRAME),
		(HWND)NULL,
		NULL,
		HINSTANCE(hinst),
		NULL);

	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);
	while(GetMessage(&msg, NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	switch(uMsg)
	{
	case WM_CREATE:
		hWnd=hwnd;
		/*RECT rect;
		GetWindowRect(hwnd, &rect);
		WindowWidth=rect.right-rect.left;//-2*GetSystemMetrics(SM_CXSIZEFRAME);
		WindowHeigth=rect.bottom-rect.top;//-GetSystemMetrics(SM_CXSIZE)-GetSystemMetrics(SM_CYSIZEFRAME);*/
		rect={0, 0, 800, 600};
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);
		//WindowWidth=rect.right-rect.left;
		//WindowHeigth=rect.bottom-rect.top;
		WindowWidth=800;
		WindowHeigth=600;

        hdc=GetDC(hwnd);
		hdcMem=CreateCompatibleDC(hdc);

		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(hInst, L"LOGO"));

		xGlobal=0;
		yGlobal=0;
		GlobalScale=1;
		IDCount=0;
		AIDCount=0;
		MaskCount=0;
		ImgIDCount=0;
		BulletsCount=0;
		TankCount=0;

		SetWindowPixelFormat();
		hRC=wglCreateContext(hdc);
		wglMakeCurrent(hdc, hRC);
		//glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, WindowWidth, WindowHeigth);
		glMatrixMode(GL_MODELVIEW);

		new Image(L"Bitmaps\\TankBody.bmp", L"Bitmaps\\TankBody_Mask.bmp", 36, 24, &TankBodyID);
		new Image(L"Bitmaps\\TankTower.bmp", L"Bitmaps\\TankTower_Mask.bmp", 16, 17, &TankTowerID);
		new Image(L"Bitmaps\\Bullet.bmp", L"Bitmaps\\Bullet_Mask.bmp", 12, 4, &BulletID);
		new Image(L"Bitmaps\\Map1.bmp", L"Bitmaps\\Map1_Draw_Mask.bmp", 400, 300, &MapID);

		TextStd=new Text(L"Font\\Font.bmp", 256, 32, 32);

		new Mask(L"Bitmaps\\TankBody_Mask.bmp", 36, 24, &TankMaskID);
		new Mask(L"Bitmaps\\Map1_Coll_Mask.bmp", 400, 300, &Map1MaskID);
		new Mask(L"Bitmaps\\Bullet_Mask.bmp", 12, 4, &BulletMaskID);

		Map1=new Map(MapID, L"Bitmaps\\Map1_Coll_Mask.bmp", L"Map1_AIPoints.txt");
		Tank1=new Tank(0, 0, 0, &Contr1, L"Player", 87, 83, 68, 65, 1);
		Tank2=new Tank(-320, 0, 0, &Contr2, L"Alpha", 87, 83, 68, 65, 1);
		Bot1=new Bot(Tank2);

		timer=SetTimer(hwnd, 100, 10, 0);
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_PAINT:
		SwapBuffers(wglGetCurrentDC());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ValidateRect(hWnd, 0);
		break;
	case WM_TIMER:
		if (wParam==timer)
		{
			GetCursorPos(&(Contr1.Mouse));
			ScreenToClient(hwnd, (&Contr1.Mouse));
			Contr1.Mouse.y=WindowHeigth-Contr1.Mouse.y;
			Contr1.Mouse.x+=(xGlobal-WindowWidth/2);
			Contr1.Mouse.y+=(yGlobal-WindowHeigth/2);
			Contr1.Mouse.x=Contr1.Mouse.x/GlobalScale;
			Contr1.Mouse.y=Contr1.Mouse.y/GlobalScale;

			for (int i=0; i<IDCount; i+=1)
			{
				if (ObjectManager[i]==NULL) continue;
				ObjectManager[i]->SteepProc();
				if (ObjectManager[i]==NULL) continue;
			}
			//Bot1->SteepProc();
			for (int i=0; i<IDCount; i+=1)
				for (int i1=0; i1<IDCount; i1+=1)
					if (i!=i1 && ObjectManager[i]->CollisionCheck(i1)==1) 
						ObjectManager[i]->ChildCollisionProc(i1);

			InvalidateRect(hWnd, 0, 1);
			/*char*str=new char[5];
			itoa(int(clock()-cl), str, 10);
			SetWindowTextA(hWnd, str);
			cl=clock();*/
		}
		break;
	case WM_KEYDOWN:
		Contr1.Keys[wParam]=1;
		break;
	case WM_KEYUP:
		Contr1.Keys[wParam]=0;
		break;
	case WM_LBUTTONDOWN:
		Contr1.Keys[VK_LBUTTON]=1;
		break;
	case WM_LBUTTONUP:
		Contr1.Keys[VK_LBUTTON]=0;
		break;
    case WM_DESTROY:
		KillTimer(hWnd, timer);
		ReleaseDC(hwnd, hdc);
		DeleteDC(hdcMem);
        PostQuitMessage(NULL);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return NULL;
}