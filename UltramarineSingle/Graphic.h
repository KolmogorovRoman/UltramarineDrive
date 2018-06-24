#pragma once
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <avrt.h>

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
class Layer;

extern HDC hdc;
extern HWND hWnd;
extern HGLRC  hRC;
extern GLuint textures[IMAGECOUNT];
extern Text* TextStd;
extern CRITICAL_SECTION cs;
extern bool Closed;
extern bool NeedFullscreen;
extern bool NeedWindowed;

void InitGraphic(HINSTANCE hInst, int nCmdShow, LPCWSTR WindowCaption);
void DrawLine(int x1, int y1, int x2, int y2, Layer* layer, double R = 0, double G = 0, double B = 0, double A = 1);
void DrawRect(int x1, int y1, int x2, int y2, bool Filled, Layer* layer, double R = 0, double G = 0, double B = 0, double A = 1);
void DrawBezier(Vector P1, Vector P2, Vector P3, Vector P4, double R = 0, double G = 0, double B = 0, double A = 1);

void WindowThreadProc(LPCWSTR WindowCaption);
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void BeginDraw();
extern void EndDraw();
extern void Draw();
extern void SetCamera();
extern void InitLayers();
extern void LoadImages();

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
	double angle, wScale, hScale;
	bool OverScreen;
	ImageExemplar(Image* image, double x, double y, double angle, double wScale = 1, double hScale = 1, bool OverScreen = false);
	void Draw() override;
};
class RectExemplar:public Exemplar
{
public:
	RectExemplar(double x1, double y1, double x2, double y2, bool Filled, double R = 0, double G = 0, double B = 0, double A = 1);
	double x1, y1, x2, y2;
	double R, G, B, A;
	bool Filled;
	void Draw() override;
};
class LineExemplar:public Exemplar
{
public:
	LineExemplar(double x1, double y1, double x2, double y2, double R = 0, double G = 0, double B = 0, double A = 1);
	double x1, y1, x2, y2;
	double R, G, B, A;
	void Draw() override;
};
class TextExemplar: public Exemplar
{
public:
	Text* text;
	string String;
	double x, y;
	bool Center;
	TextExemplar(Text* text, string String, double x, double y, bool Center);
	void Draw() override;
	~TextExemplar();
};
class BezierExemplar:public Exemplar
{
public:
	BezierExemplar(Vector P1, Vector P2, Vector P3, Vector P4, double R = 0, double G = 0, double B = 0, double A = 1);
	double R, G, B, A;
	Vector P1, P2, P3, P4;
	void Draw() override;
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
	Image(string Name, string Maskname, int Centerx, int Centery);
	Image(string Name);
	void Draw(double x, double y, Layer* layer, double angle, double wScale = 1, double hScale = 1, bool OverScreen = 0);
	~Image();
};
class AnimatedImage
{
public:
	Image** Images;
	int* ImagesTime;
	int ImagesCount;
	AnimatedImage(int Count, Image* Image1, int Time1...);
	void Draw(int ImageNumber, int x, int y, Layer* layer, double angle, double wScale = 1, double hScale = 1, bool OverScreen = false);
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
	void DrawSymbol(UCHAR Symb, int x, int y, bool Center);
	void Draw(string String, int x, int y, Layer* layer, bool Center);
};

class Layer
{
public:
	bool IsInterface;
	std::list<Exemplar*> Content;
	static std::list<Layer*> LayersList;
	std::list<Layer*>::iterator Iterator;
	static Layer* New(bool Interface);
	static Layer* Higher(Layer* Other, bool IsInterface);
	static Layer* Lower(Layer* Other, bool IsInterface);
	void Add(Exemplar* exemplar);
};

class GraphicUnit:
	public virtual PointUnit,
	public ManagedUnit<GraphicUnit>
{
public:
	Image* image;
	Layer* layer;
	GraphicUnit(Image* image, Layer* layer);
	void Draw();
	virtual void DrawProc() {};
	bool InScreen();
	virtual ~GraphicUnit();
};
class AnimatedGraphicUnit:
	public virtual PointUnit,
	public ManagedUnit<AnimatedGraphicUnit>
{
public:
	AnimatedImage* Image;
	Layer* layer;
	int CurrentImage;
	int CurrentTime;
	bool Cicled;
	AnimatedGraphicUnit(AnimatedImage* Image, Layer* layer, bool Cicled);
	void Draw();
	virtual void DrawProc() {};
	bool InScreen();
	virtual ~AnimatedGraphicUnit();
};

class Camera:
	public PointUnit
{
public:
	int Width = 800, Height = 600;
	double Scale = 1;
};
extern Camera MainCamera;