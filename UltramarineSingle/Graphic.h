#pragma once
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "resource.h"
#include "Misc.h"
#include "Management.h"
using namespace std;

#define IMAGECOUNT 16
#define OVERGLOBAL 0
#define OVERSCREEN 1

class ImageExemplar;
class Image;
class Text;
class GraphicUnit;

extern HDC hdc;
extern HWND hWnd;
extern HGLRC  hRC;
extern GLuint textures[IMAGECOUNT];
extern Text* TextStd;
extern CRITICAL_SECTION cs;
extern bool Closed;

//extern ChainFunctor* SteepFunctor;
//extern ChainFunctor* CreatingFunctor;
//extern ChainFunctor* DrawFunctor;

void InitGraphic(HINSTANCE hInst, int nCmdShow, LPCWSTR WindowCaption);
void DrawLine(int x1, int y1, int x2, int y2, int z, double R = 0, double G = 0, double B = 0, double A = 1);
void DrawRect(int x1, int y1, int x2, int y2, bool Filled, int z, double R = 0, double G = 0, double B = 0, double A = 1);

//DeclarateFuncForEach(GraphicUnit, Draw)
//DeclarateFuncForEach(GraphicUnit, DrawProc)

//DeclarateFuncForEach(AnimatedGraphicUnit, Draw)
//DeclarateFuncForEach(AnimatedGraphicUnit, DrawProc)

void WindowThreadProc(LPCWSTR WindowCaption);
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void BeginDraw();
extern void EndDraw();
extern void Draw();

class Exemplar
{
public:
	virtual void Draw() {};
	virtual ~Exemplar() {};
};
class ImageExemplar: public Exemplar
{
public:
	Image* image;
	double x, y;
	int z;
	double angle, wScale, hScale;
	bool OverScreen;
	ImageExemplar(Image* image, double x, double y, int z, double angle, double wScale = 1, double hScale = 1, bool OverScreen = false);
	void Draw();
};
class RectExemplar:public Exemplar
{
public:
	RectExemplar(double x1, double y1, double x2, double y2, bool Filled, int z, double R = 0, double G = 0, double B = 0, double A = 0);
	int x1, y1, x2, y2;
	int z;
	double R, G, B, A;
	bool Filled;
	void Draw();
};
class LineExemplar:public Exemplar
{
public:
	LineExemplar(double x1, double y1, double x2, double y2, int z, double R = 0, double G = 0, double B = 0, double A = 1);
	double x1, y1, x2, y2;
	int z;
	double R, G, B, A;
	void Draw();
};
class TextExemplar: public Exemplar
{
public:
	Text* text;
	char* String;
	int x, y, z;
	bool Center;
	TextExemplar(Text* text, LPSTR String, int x, int y, int z, bool Center);
	void Draw();
	~TextExemplar();
};

class Image
{
private:
	AUX_RGBImageRec *texture, *mask;
public:
	GLuint TextureIndex;
	POINT Center;
	bool *Mask;
	int Height, Width, RealWidth, RealHeight;
	Image(LPCTSTR Name, LPCTSTR Maskname, int Centerx, int Centery);
	void Draw(double x, double y, int z, double angle, double wScale = 1, double hScale = 1, bool OverScreen = 0);
	~Image();
};
class AnimatedImage
{
public:
	Image** Images;
	int* ImagesTime;
	int ImagesCount;
	AnimatedImage(int Count, Image* Image1, int Time1...);
	void Draw(int ImageNumber, int x, int y, int z, double angle, double wScale = 1, double hScale = 1, bool OverScreen = false);
	~AnimatedImage();
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
	Text(LPCWSTR FontFileBMP, int SymbolsCount, int ChWidth, int ChHeight);
	void DrawSymbol(UCHAR Symb, int x, int y, int z, bool Center);
	void Draw(LPSTR String, int x, int y, int z, bool Center);
};

class GraphicUnit:
	public virtual PointUnit,
	public ManagedUnit<GraphicUnit>
{
public:
	Image* image;
	int z;
	GraphicUnit(Image* Image, int DefaultZ);
	void Draw();
	virtual void DrawProc() {};
	virtual ~GraphicUnit();
};
class AnimatedGraphicUnit:
	public virtual PointUnit,
	public ManagedUnit<AnimatedGraphicUnit>
{
public:
	AnimatedImage* Image;
	int z;
	int CurrentImage;
	int CurrentTime;
	bool Cicled;
	AnimatedGraphicUnit(AnimatedImage* Image, int DefaultZ, bool Cicled);
	void Draw();
	virtual void DrawProc() {};
	virtual ~AnimatedGraphicUnit();
};