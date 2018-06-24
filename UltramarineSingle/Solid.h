#pragma once
class Mask;
class SolidUnit;
class ContMask;

#include <gl/glaux.h>
#include "Misc.h"
#include "Management.h"
#define CHUNK_SIZE 16
#define CH_NOCALC 0
#define CH_EMPTY 1
#define CH_MIXED 2
#define CH_FULLED 3
extern bool CollTable[5][5];
extern void LoadMasks();

class Mask
{
public:
	Mask* MaskSrc;
	double Angle;
	int Width, Height, Size;
	int ChunksWidth, ChunksHeight;
	bool *Array;
	BYTE* ChunksArray = NULL;
	POINT Center;
	Mask(std::string, int, int);
	Mask(Mask*, double);
	void CalcChunks();
	//BYTE CalcChunk(int x, int y);
	~Mask();
};
class SolidUnit:
	public ManagedUnit<SolidUnit>,
	public virtual PointUnit
{
public:
	double PrevAngle;
	Mask* mask;
	Mask* maskSrc;
	bool MaskCalced;
	int Type;
	SolidUnit(Mask* mask, int AType);
	SolidUnit();
	bool PixelCheck(int x, int y);
	bool CollisionCheck(SolidUnit* Other);
	virtual void CollProc(SolidUnit* Other) {};
	virtual ~SolidUnit();
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

bool MaskCollCheck(Mask* Mask1, int x1, int y1, Mask* Mask2, int x2, int y2);

bool Visible(int X1, int Y1, int X2, int Y2, Mask* Barrier, int BarX, int BarY);
bool Visible(int X, int Y, ContMask* Target, int TarX, int TarY, Mask* Barrier, int BarX, int BarY);

void AllCollCheck();