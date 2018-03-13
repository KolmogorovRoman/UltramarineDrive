#include "Solid.h"

Mask::Mask(LPCTSTR Maskname, int ACenterX, int ACenterY)
{
	Center.x = ACenterX;
	Center.y = ACenterY;
	AUX_RGBImageRec *mask;
	mask = auxDIBImageLoad(Maskname);
	MaskSrc = NULL;
	Angle = 0;

	Width = mask->sizeX;
	Height = mask->sizeY;
	Size = Width*Height;
	Array = new bool[Size];
	for (int i = 0; i < mask->sizeX*mask->sizeY; i += 1)
	{
		if (mask->data[i * 3] == 255)
			Array[i] = 1;
		if (mask->data[i * 3] == 0) Array[i] = 0;
	}

	//ToDo Chunks
	/*ChunkWidth = ((Width + 15) / 16);
	ChunkHeight = ((Height + 15) / 16);
	ChunkSize = ChunkWidth*ChunkHeight;
	ChunkArray = new BYTE[ChunkSize];
	for (int i = 0; i < ChunkSize; i++)
	{
		ChunkArray[i] = 0;
	}
	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			ChunkArray[y / 16 * ChunkWidth + x / 16]++;
		}
	}
	for (int i = 0; i < ChunkSize; i++)
	{
		if (ChunkArray[i] == 0) ChunkArray[i] = CH_EMPTY;
		else if (ChunkArray[i] == 255) ChunkArray[i] = CH_FULLED;
		else ChunkArray[i] = CH_MIXED;
	}*/
}
Mask::Mask(Mask* OtherMask, double angle)
{
	MaskSrc = OtherMask;
	Angle = angle;
	if (angle != 0)
	{
		int SrcWidth, SrcHeight;
		SrcWidth = OtherMask->Width;
		SrcHeight = OtherMask->Height;
		Width = TurnedRectSize(SrcWidth, SrcHeight, angle).x;
		Height = TurnedRectSize(SrcWidth, SrcHeight, angle).y;
		Size = Width*Height;
		POINT SrcCenter;
		SrcCenter = OtherMask->Center;
		Center = TurnedRectCenter(SrcWidth, SrcHeight, SrcCenter.x, SrcCenter.y, angle);
		Array = new bool[Size];
		bool *OtherArray = OtherMask->Array;
		int xsrc, ysrc;
		int* xcos = new int[Width];
		int* xsin = new int[Width];
		int* ycos = new int[Height];
		int* ysin = new int[Height];
		double asin, acos;
		asin = dsin(angle);
		acos = dcos(angle);

		//dbg
		/*glLoadIdentity();
		glOrtho(-400, 400, -300, 300, 0, -100);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(1, 1, 0);
		glBegin(GL_POINTS);*/
		for (int cx = 0; cx < Width; cx += 1)
		{
			xcos[cx] = (cx - Center.x)*acos;
			xsin[cx] = (cx - Center.x)*asin;
		}
		for (int cy = 0; cy < Height; cy += 1)
		{
			ycos[cy] = (cy - Center.y)*acos;
			ysin[cy] = (cy - Center.y)*asin;
		}
		for (int cy = 0; cy < Height; cy += 1)
			for (int cx = 0; cx < Width; cx += 1)
			{
				xsrc = SrcCenter.x + xcos[cx] + ysin[cy];
				ysrc = SrcCenter.y - xsin[cx] + ycos[cy];
				if (xsrc >= 0 && xsrc < SrcWidth && ysrc >= 0 && ysrc < SrcHeight)
					Array[cy*Width + cx] = OtherArray[ysrc*SrcWidth + xsrc];
				else
					Array[cy*Width + cx] = 0;
				/*if (Array[cy*Width+cx]==1)
				{
					glVertex3f(cx, cy, 0);
				}*/
			}
		//glEnd();
		delete[] xsin;
		delete[] xcos;
		delete[] ysin;
		delete[] ycos;

		/*ChunkWidth = ((Width + 15) / 16);
		ChunkHeight = ((Height + 15) / 16);
		ChunkSize = ChunkWidth*ChunkHeight;
		ChunkArray = new BYTE[ChunkSize];
		for (int i = 0; i < ChunkSize; i++)
		{
			ChunkArray[i] = CH_NOCALC;
		}*/
	}
	else
	{
		Size = OtherMask->Size;
		Width = OtherMask->Width;
		Height = OtherMask->Height;
		Center.x = OtherMask->Center.x;
		Center.y = OtherMask->Center.y;
		bool *OtherArray = OtherMask->Array;
		Array = new bool[Size];
		for (int i = 0; i < Size; i += 1)
		{
			Array[i] = OtherArray[i];
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
		/*ChunkWidth = ((Width + 15) / 16);
		ChunkHeight = ((Height + 15) / 16);
		ChunkSize = ChunkWidth*ChunkHeight;*/
		/*ChunkArray = new BYTE[ChunkSize];
		for (int i = 0; i < ChunkSize; i++)
		{
			ChunkArray[i] = CH_NOCALC;
		}*/
	}
}
//BYTE Mask::CalcChunk(int x, int y)
//{
//	int cx = x / CHUNK_SIZE, cy = y / CHUNK_SIZE;
//	int i = cy*ChunkWidth + cx;
//	if (ChunkArray[i] != CH_NOCALC) return ChunkArray[i];
//	//if (cx < ChunkWidth && cx < ChunkWidth)
//	bool *OtherArray = MaskSrc->Array;
//	int* xcos = new int[CHUNK_SIZE];
//	int* xsin = new int[CHUNK_SIZE];
//	int* ycos = new int[CHUNK_SIZE];
//	int* ysin = new int[CHUNK_SIZE];
//	double asin, acos;
//	asin = dsin(Angle);
//	acos = dcos(Angle);
//	for (int ix = cx*CHUNK_SIZE; ix < CHUNK_SIZE; ix++)
//	{
//		xcos[ix] = (ix - Center.x)*acos;
//		xsin[ix] = (ix - Center.x)*asin;
//	}
//	for (int iy = cy*CHUNK_SIZE; iy < CHUNK_SIZE; iy++)
//	{
//		ycos[iy] = (cy - Center.y)*acos;
//		ysin[iy] = (cy - Center.y)*asin;
//	}
//	int xsrc, ysrc;
//	POINT SrcCenter;
//	SrcCenter = MaskSrc->Center;
//	int SrcWidth, SrcHeight;
//	SrcWidth = MaskSrc->Width;
//	SrcHeight = MaskSrc->Height;
//	for (int iy = cy*CHUNK_SIZE; iy < CHUNK_SIZE; iy += 1)
//		for (int ix = cx*CHUNK_SIZE; ix < CHUNK_SIZE; ix += 1)
//		{
//			xsrc = SrcCenter.x + xcos[ix%CHUNK_SIZE] + ysin[iy%CHUNK_SIZE];
//			ysrc = SrcCenter.y - xsin[ix%CHUNK_SIZE] + ycos[iy%CHUNK_SIZE];
//			if (xsrc >= 0 && xsrc < SrcWidth && ysrc >= 0 && ysrc < SrcHeight)
//				Array[iy*Width + ix] = OtherArray[ysrc*SrcWidth + xsrc];
//			else
//				Array[iy*Width + ix] = 0;
//			ChunkArray[i] += Array[iy*Width + ix];
//		}
//	if (ChunkArray[i] == 0) ChunkArray[i] = CH_EMPTY;
//	else if (ChunkArray[i] == 255) ChunkArray[i] = CH_FULLED;
//	else ChunkArray[i] = CH_MIXED;
//	delete[] xsin;
//	delete[] xcos;
//	delete[] ysin;
//	delete[] ycos;
//}
Mask::~Mask()
{
	delete[] Array;
	//delete[] ChunkArray;
}

SolidUnit::SolidUnit(Mask* Amask, int AType) :
	PointUnit()
{
	Type = AType;
	maskSrc = new Mask(Amask, 0);
	mask = new Mask(maskSrc, 0);
}
SolidUnit::SolidUnit()
{
}
bool SolidUnit::PixelCheck(int x, int y)
{
	if (x < this->x - mask->Center.x || y < this->y - mask->Center.y || x >= this->x - mask->Center.x + mask->Width || y >= this->y - mask->Center.y + mask->Height)
		return false;
	int cx = x - (this->x - mask->Center.x);
	int cy = y - (this->y - mask->Center.y);
	return mask->Array[cy*mask->Width + cx];
}
bool SolidUnit::CollisionCheck(SolidUnit* Other)
{
	if (CollTable[Type][Other->Type] == true && Other != this)
	{
		if (MaskCollCheck(mask, x, y, Other->mask, Other->x, Other->y) == 1)
			return true;
	}
	return false;
}
SolidUnit::~SolidUnit()
{
	delete mask;
	delete maskSrc;
}

ContMask::ContMask(Mask* MaskSrc)
{
	Center = MaskSrc->Center;
	Width = MaskSrc->Width;
	Height = MaskSrc->Height;
	Size = MaskSrc->Size;
	Array = new bool[Size];
	bool* ArraySrc = MaskSrc->Array;
	int x, y;
	PointCount = 0;
	//glBegin(GL_POINTS);
	for (int i = 0; i < Size; i += 1)
	{
		x = i%Width;
		y = i / Width;
		Array[i] = false;
		if (ArraySrc[i] == 1)
		{
			if (x == 0 || y == 0 || x == Width - 1 || y == Height - 1)
			{
				PointCount++;
				Array[i] = 1;
				continue;
			}
			if (ArraySrc[i - 1] == 0 || ArraySrc[i + 1] == 0 || ArraySrc[i - Width] == 0 || ArraySrc[i + Width] == 0)
			{
				PointCount++;
				Array[i] = 1;
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
		//glEnd();
	}
}
ContMask::~ContMask()
{
	delete[] Array;
}

bool MaskCollCheck(Mask* Mask1, int x1, int y1, Mask* Mask2, int x2, int y2)
{
	int xstart, cx1, cx2, dx, lx1, lx2, ystart, cy1, cy2, dy, ly1, ly2, w1, w2, wscan, h1, h2, hscan;
	w1 = Mask1->Width;
	w2 = Mask2->Width;
	h1 = Mask1->Height;
	h2 = Mask2->Height;
	cx1 = Mask1->Center.x;
	cx2 = Mask2->Center.x;
	cy1 = Mask1->Center.y;
	cy2 = Mask2->Center.y;

	dx = abs(x1 - x2);
	dy = abs(y1 - y2);
	if (x1 - cx1 <= x2 - cx2)
	{
		xstart = x2 - cx2;
		wscan = (w1 - cx1 + cx2) - dx;
		lx1 = xstart - x1 + cx1;
		lx2 = 0;
	}
	else
	{
		xstart = x1 - cx1;
		wscan = (cx1 + w2 - cx2) - dx;
		lx1 = 0;
		lx2 = xstart - x2 + cx2;
	}
	if (y1 - cy1 <= y2 - cy2)
	{
		ystart = y2 - cy2;
		hscan = (h1 - cy1 + cy2) - dy;
		ly1 = ystart - y1 + cy1;
		ly2 = 0;
	}
	else
	{
		ystart = y1 - cy1;
		hscan = (cy1 + h2 - cy2) - dy;
		ly1 = 0;
		ly2 = ystart - y2 + cy2;
	}
	if (wscan <= 0 || hscan <= 0) return false;
	if (x1 - cx1 > x2 - cx2 && x1 + w1 - cx1 < x2 + w2 - cx2) wscan = w1;
	if (x2 - cx2 > x1 - cx1 && x2 + w2 - cx2 < x1 + w1 - cx1) wscan = w2;
	if (y1 - cy1 > y2 - cy2 && y1 + h1 - cy1 < y2 + h2 - cy2) hscan = h1;
	if (y2 - cy2 > y1 - cy1 && y2 + h2 - cy2 < y1 + h1 - cy1) hscan = h2;
	for (int y = 0; y < hscan; y += 1)
		for (int x = 0; x < wscan; x += 1)
		{
			if (Mask1->Array[lx1 + x + (ly1 + y)*w1] == true && Mask2->Array[lx2 + x + (ly2 + y)*w2] == true)
			{
				return true;
			}
		}
	return false;
}
bool MaskCollCheck(Mask* Mask1, int x1, int y1, double Angle1, Mask* Mask2, int x2, int y2, double Angle2)
{
	return false;
}

bool Visible(int X1, int Y1, int X2, int Y2, Mask* Barrier, int BarX, int BarY)
{
	bool ret;
	double angle;
	int dir, len;
	double x, y;
	len = sqrt((X2 - X1)*(X2 - X1) + (Y2 - Y1)*(Y2 - Y1));
	angle = AngleToPoint(X1, Y1, X2, Y2);
	x = X1;
	y = Y1;
	/*glLoadIdentity();
	glOrtho(-400, 400, -300, 300, -1, 1);
	glBegin(GL_POINTS);*/
	for (int i = 0; i < len; i += 1)
	{
		if (Barrier->Array[(int) (x + Barrier->Center.x - BarX) + Barrier->Width*((int) (y + Barrier->Center.y - BarY))] == true)
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
		x += dcos(angle);
		y += dsin(angle);
	}
	//glEnd();
	return true;
}
bool Visible(int X, int Y, ContMask* Target, int TarX, int TarY, Mask* Barrier, int BarX, int BarY)
{
	int x, y;
	bool ret;
	for (int i = 0; i < Target->Size; i += 1)
	{
		if (Target->Array[i] == 0) continue;
		x = i%Target->Width;
		y = i / Target->Width;
		if (Visible(X, Y, x - Target->Center.x + TarX, y - Target->Center.y + TarY, Barrier, BarX, BarY) == true) return true;
	}
	return false;
}

void AllCollCheck()
{
	ForEach(SolidUnit, Unit)
	{
		delete Unit->mask;
		Unit->mask = new Mask(Unit->maskSrc, Unit->angle);
	}
	ForEach(SolidUnit, Unit)
	{
		ForEach(SolidUnit, Unit1)
			if (Unit->CollisionCheck(Unit1)) Unit->CollProc(Unit1);
	}
}