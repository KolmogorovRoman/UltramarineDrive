#include "SolidPhysic.h"
//#include "Graphic.h"

TwinVector ImpactCalc(Mask* Mask1, Mask* Mask2, int x1, int y1, int x2, int y2)
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
	if (wscan <= 0 || hscan <= 0) return tv(0, 0, 0, 0);
	if (x1 - cx1 > x2 - cx2 && x1 + w1 - cx1 < x2 + w2 - cx2) wscan = w1;
	if (x2 - cx2 > x1 - cx1 && x2 + w2 - cx2 < x1 + w1 - cx1) wscan = w2;
	if (y1 - cy1 > y2 - cy2 && y1 + h1 - cy1 < y2 + h2 - cy2) hscan = h1;
	if (y2 - cy2 > y1 - cy1 && y2 + h2 - cy2 < y1 + h1 - cy1) hscan = h2;

	TwinVector Ret;
	int Force;
	Force = 0;
	Ret.vct1 = v(0, 0);//Source
	Ret.vct2 = v(0, 0);//Value
	bool* Mask1Array = Mask1->Array;
	bool* Mask2Array = Mask2->Array;
	for (int y = 0; y < hscan; y += 1)
	{
		for (int x = 0; x < wscan; x += 1)
		{
			if (Mask1Array[lx1 + x + (ly1 + y)*w1] == 1 && Mask2Array[lx2 + x + (ly2 + y)*w2] == 1)
			{
				Force += 1;
				Ret.vct1 += v(x - cx1 + lx1, y - cy1 + ly1);
				if (NeightNeumann(Mask1Array, w1, h1, lx1 + x, ly1 + y) != 0)
				{
					if (lx1 + x + 1 < w1 && Mask1Array[lx1 + x + 1 + (ly1 + y)*w1] == 1) Ret.vct2 += v(1, 0);
					if (lx1 + x + 1 < w1 && ly1 + y + 1 < h1 && Mask1Array[lx1 + x + 1 + (ly1 + y + 1)*w1] == 1) Ret.vct2 += v(SQRT2, SQRT2);
					if (ly1 + y + 1 < h1 && Mask1Array[lx1 + x + (ly1 + y + 1)*w1] == 1) Ret.vct2 += v(0, 1);
					if (lx1 + x - 1 >= 0 && ly1 + y + 1 < h1 && Mask1Array[lx1 + x - 1 + (ly1 + y + 1)*w1] == 1) Ret.vct2 += v(-SQRT2, SQRT2);
					if (lx1 + x - 1 >= 0 && Mask1Array[lx1 + x - 1 + (ly1 + y)*w1] == 1) Ret.vct2 += v(-1, 0);
					if (lx1 + x - 1 >= 0 && ly1 + y - 1 >= 0 && Mask1Array[lx1 + x - 1 + (ly1 + y - 1)*w1] == 1) Ret.vct2 += v(-SQRT2, -SQRT2);
					if (ly1 + y - 1 >= 0 && Mask1Array[lx1 + x + (ly1 + y - 1)*w1] == 1) Ret.vct2 += v(0, -1);
					if (lx1 + x + 1 < w1 && ly1 + y - 1 >= 0 && Mask1Array[lx1 + x + 1 + (ly1 + y - 1)*w1] == 1) Ret.vct2 += v(SQRT2, -SQRT2);
				}
				/*glLoadIdentity();
				glOrtho(-MainCamera.Width/2, MainCamera.Width/2, -MainCamera.Height/2, MainCamera.Height/2, 0, -100);
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
	Ret.vct1.x /= Force;
	Ret.vct1.y /= Force;
	if (!(Ret.vct2.x == 0 && Ret.vct2.y == 0)) Ret.vct2.Norm();
	if (Force > 20) Force = 20;
	Ret.vct2 = Ret.vct2*(Force * 2);
	if (Force == 0)
	{
		Ret.vct1 = v(0, 0);
		Ret.vct2 = v(0, 0);
	}
	return Ret;
}

void CalcAllSolidPhysicUnits()
{
	ForEach(SolidPhysicUnit, Unit)
	{
		ForEach(SolidUnit, Unit1)
		{
			Unit->CollCalc(Unit1);
		}
	}
}

SolidPhysicUnit::SolidPhysicUnit(Mask* Amask, int AType, Vector ASpeed, Vector AImpulse, Vector AForce, double AMass) :
	PhysicUnit(ASpeed, AImpulse, AForce, AMass),
	SolidUnit(Amask, AType)
{
}
void SolidPhysicUnit::CollCalc(SolidUnit* Other)
{
	if ((SolidUnit*)this == Other) return;
	if (PhysCollTable[Type][Other->Type] == true && CollisionCheck(Other) == true)
	{
		TwinVector Impact = ImpactCalc(mask, Other->mask, x, y, Other->x, Other->y);
		ApplyImpulse(Impact);
	}
}