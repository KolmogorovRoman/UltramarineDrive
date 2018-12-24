#pragma once
#include <Windows.h>
#define GLEW_STATIC
#include "OpenGL/glew.h"
#include <GL/glu.h>
#include <GL/glaux.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "Misc.h"
#include "Management.h"
using namespace std;

#define IMAGECOUNT 16
#define OVERGLOBAL 0
#define OVERSCREEN 1

class ImageExemplar;
class Image;
class GraphicUnit;
class Layer;

extern HDC hdc;
extern HWND hWnd;
extern HGLRC  hRC;
extern GLuint textures[IMAGECOUNT];
extern CRITICAL_SECTION cs;
extern bool Closed;
extern bool NeedFullscreen;
extern bool NeedWindowed;

void InitGraphic(HINSTANCE hInst, int nCmdShow, LPCWSTR WindowCaption);
void DrawLine(double x1, double y1, double x2, double y2, Layer* layer, double R = 0, double G = 0, double B = 0, double A = 1);
void DrawRect(double x1, double y1, double x2, double y2, bool Filled, Layer* layer, double R = 0, double G = 0, double B = 0, double A = 1);
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
	virtual void operator()() = 0;
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
	void operator()() override;
};
class RectExemplar:public Exemplar
{
public:
	RectExemplar(double x1, double y1, double x2, double y2, bool Filled, double R = 0, double G = 0, double B = 0, double A = 1);
	double x1, y1, x2, y2;
	double R, G, B, A;
	bool Filled;
	void operator()() override;
};
class LineExemplar:public Exemplar
{
public:
	LineExemplar(double x1, double y1, double x2, double y2, double R = 0, double G = 0, double B = 0, double A = 1);
	double x1, y1, x2, y2;
	double R, G, B, A;
	void operator()() override;
};
class BezierExemplar:public Exemplar
{
public:
	BezierExemplar(Vector P1, Vector P2, Vector P3, Vector P4, double R = 0, double G = 0, double B = 0, double A = 1);
	double R, G, B, A;
	Vector P1, P2, P3, P4;
	void operator()() override;
};

class Image
{
private:
	AUX_RGBImageRec *texture, *mask;
public:
	GLuint TextureIndex;
	POINT Center;
	int Height, Width, RealWidth, RealHeight;
	Image(string Name, string Maskname, int Centerx, int Centery);
	Image(string Name);
	void Draw(double x, double y, Layer* layer, double angle, double wScale = 1, double hScale = 1, bool OverScreen = 0);
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

class Layer
{
public:
	bool IsInterface;
	std::list<function<void(void)>*> Content;
	static std::list<Layer*> LayersList;
	std::list<Layer*>::iterator Iterator;
	GLuint FrameBuffer;
	GLuint Texture;
	Layer(bool IsInterface);
	static Layer* New(bool Interface);
	static Layer* Higher(Layer* Other, bool IsInterface);
	static Layer* Lower(Layer* Other, bool IsInterface);
	template<class T> void Add(T& exemplar);
};
template<class T> void Layer::Add(T& exemplar)
{
	function<void(void)>* f = new function<void(void)>(exemplar);
	Content.push_back(f);
}

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

class Model
{
public:
	GLuint VboIndex;
	GLuint ColorsIndex;
	GLuint NormalsIndex;
	struct Vertex
	{
		double x, y, z;
	};
	struct TextureVertex
	{
		double u, v;
	};
	struct Normale
	{
		double x, y, z;
	};
	struct Material
	{
		struct Color
		{
			double r, g, b;
		};
		Color Ambient;
		Color Diffuse;
	};
	struct Face
	{
		struct Vertex
		{
			int VertexIndex = 0;
			int TextureIndex = 0;
			int NormalIndex = 0;
		};
		Vertex Vertexes[3];
		Material* Material;
	};
	vector<Vertex> Vertexes;
	vector<TextureVertex> TextureVertexes;
	vector<Normale> Normales;
	vector<Face> Faces;
	Model(string Path, string FileName);
	void Draw(double x, double y, double z, double angle);
};
class ModelExemplar:
	public Exemplar
{
public:
	Model* model;
	double x, y, z;
	double angle;
	ModelExemplar(Model* model, double x, double y, double z, double angle);
	void operator()() override;
};

class Camera:
	public PointUnit
{
public:
	int Width = 800, Height = 600;
	double Scale = 1;
	double HeightAndge = 0;
};
extern Camera MainCamera;