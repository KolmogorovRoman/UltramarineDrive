#include "Graphic.h"

HDC hdc;
HGLRC  hRC;
HWND hWnd;
GLuint textures[IMAGECOUNT];
Text* TextStd;
mutex Mutex;
CRITICAL_SECTION cs;
condition_variable Cond;
List<Exemplar> PrimitiveList[100];

HINSTANCE ghInst;
int gnCmdShow;
thread* WindowThread;
bool Closed = false;

void DrawLine(int x1, int y1, int x2, int y2, int z, double R, double G, double B, double A)
{
	PrimitiveList[z].Add(new LineExemplar(x1, y1, x2, y2, z, R, G, B, A));
}
void DrawRect(int x1, int y1, int x2, int y2, bool Filled, int z, double R, double G, double B, double A)
{
	PrimitiveList[z].Add(new RectExemplar(x1, y1, x2, y2, Filled, z, R, G, B, A));
}

LineExemplar::LineExemplar(double x1, double y1, double x2, double y2, int z, double R, double G, double B, double A)
{
	this->x1 = x1;
	this->y1 = y1;
	this->x2 = x2;
	this->y2 = y2;
	this->z = z;
	this->R = R;
	this->G = G;
	this->B = B;
	this->A = A;
}
void LineExemplar::Draw()
{
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, 0, -100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(R, G, B, A);
	glBegin(GL_LINES);
	glVertex3i(x1, y1, z);
	glVertex3i(x2, y2, z);
	glEnd();
}

RectExemplar::RectExemplar(double x1, double y1, double x2, double y2, bool Filled, int z, double R, double G, double B, double A)
{
	this->x1 = x1;
	this->y1 = y1;
	this->x2 = x2;
	this->y2 = y2;
	this->Filled = Filled;
	this->z = z;
	this->R = R;
	this->G = G;
	this->B = B;
	this->A = A;
}
void RectExemplar::Draw()
{
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, 0, -100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(R, G, B, A);
	if (Filled == true) glBegin(GL_QUADS);
	if (Filled == false) glBegin(GL_LINE_LOOP);
	glVertex3f(x1 + 0.5, y1 + 0.5, z);
	glVertex3f(x2 + 0.5, y1 + 0.5, z);
	glVertex3f(x2 + 0.5, y2 + 0.5, z);
	glVertex3f(x1 + 0.5, y2 + 0.5, z);
	glEnd();
}

ImageExemplar::ImageExemplar(Image* image, double x, double y, int z, double angle, double wScale, double hScale, bool OverScreen):
	image(image),
	x(x),
	y(y),
	z(z),
	angle(angle),
	wScale(wScale),
	hScale(hScale),
	OverScreen(OverScreen)
{}
void ImageExemplar::Draw()
{
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, 0, -100);
	glTranslatef(x, y, z);
	glRotatef(angle, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, image->TextureIndex);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3i(-image->Center.x * wScale, -image->Center.y * hScale, 0);
	glTexCoord2i(0, 1); glVertex3i(-image->Center.x * wScale, (image->Height - image->Center.y) * hScale, 0);
	glTexCoord2i(1, 1); glVertex3i((image->Width - image->Center.x) * wScale, (image->Height - image->Center.y) * hScale, 0);
	glTexCoord2i(1, 0); glVertex3i((image->Width - image->Center.x) * wScale, -image->Center.y * hScale, 0);
	glEnd();
}

TextExemplar::TextExemplar(Text* text, LPSTR String, int x, int y, int z, bool Center):
	text(text),
	x(x),
	y(y),
	z(z),
	Center(Center)
{
	this->String = new char[strlen(String) + 1];
	strcpy(this->String, String);
}
void TextExemplar::Draw()
{
	int TotalWidth, Len, cx;
	TotalWidth = 0;
	Len = strlen(String);
	for (int i = 0; i < Len; i += 1)
	{
		TotalWidth += text->Symbols[String[i]].Width;
	}
	if (Center == 1) cx = -TotalWidth / 2;
	else cx = 0;
	for (int i = 0; i < Len; i += 1)
	{
		if (Center == true) text->DrawSymbol(String[i], x + cx + text->Symbols[String[i]].Width / 2, y, z, 1);
		else text->DrawSymbol(String[i], x + cx, y, z, false);
		cx += text->Symbols[String[i]].Width;
	}
}
TextExemplar::~TextExemplar()
{
	delete[] String;
}

#define BORDER 1
Image::Image(LPCTSTR Name, LPCTSTR Maskname, int ACenterx, int ACentery)
{
	GLubyte* Data;
	texture = auxDIBImageLoad(Name);
	mask = auxDIBImageLoad(Maskname);
	RealWidth = texture->sizeX;
	RealHeight = texture->sizeY;
	Width = pow(2, ceil(log(RealWidth + BORDER*2.0) / log(2.0)));
	Height = pow(2, ceil(log(RealHeight + BORDER*2.0) / log(2.0)));
	Center.x = ACenterx + BORDER;
	Center.y = ACentery + BORDER;
	Data = new GLubyte[Height*Width * 4];
	for (int i = 0; i < Height*Width * 4; i += 1)
	{
		Data[i] = 0;
	}
	for (int iy = 0; iy < RealHeight; iy += 1)
		for (int ix = 0; ix < RealWidth; ix += 1)
		{
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 0] = texture->data[((iy) *RealWidth + ix) * 3 + 0];
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 1] = texture->data[((iy) *RealWidth + ix) * 3 + 1];
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 2] = texture->data[((iy) *RealWidth + ix) * 3 + 2];
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 3] = mask->data[((iy) *RealWidth + ix) * 3 + 0];
		}
	glGenTextures(1, &TextureIndex);
	glBindTexture(GL_TEXTURE_2D, TextureIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
}
void Image::Draw(double x, double y, int z, double angle, double wScale, double hScale, bool OverScreen)
{
	PrimitiveList[z].Add(new ImageExemplar(this, x, y, z, angle, wScale, hScale, OverScreen));
}
Image::~Image()
{
	delete[] Mask;
}

AnimatedImage::AnimatedImage(int Count, Image* Image1, int Time1...)
{
	ImagesCount = Count;
	Images = new Image*[Count];
	ImagesTime = new int[Count];
	Image** ReadedImage = &Image1;
	int* ReadedTime = &Time1;
	for (int i = 0; i < Count; i++)
	{
		Images[i] = *ReadedImage;
		ImagesTime[i] = *ReadedTime;
		ReadedImage = (Image**) ((char*) (ReadedImage) +sizeof(Image*) + sizeof(int));
		ReadedTime = (int*) ((char*) (ReadedTime) +sizeof(Image*) + sizeof(int));
	}
}
void AnimatedImage::Draw(int ImageNumber, int x, int y, int z, double angle, double wScale, double hScale, bool OverScreen)
{
	Images[ImageNumber]->Draw(x, y, z, angle, wScale, hScale, OverScreen);
}
AnimatedImage::~AnimatedImage()
{
	delete[] Images;
	delete[] ImagesTime;
}

Text::Text(LPCWSTR FontFileBMP, int ASymbolsCount, int AChWidth, int AChHeight)
{
	SymbolsCount = ASymbolsCount;
	ChWidth = AChWidth;
	ChHeight = AChHeight;
	Font = auxDIBImageLoad(FontFileBMP);
	Width = Font->sizeX;
	Height = Font->sizeY;
	int i1 = 0;
	Data = new GLubyte[Width*Height * 4];
	for (int i = 0; i < Width*Height; i += 1)
	{
		Data[i * 4 + 3] = 255;

		if (Font->data[i1 * 3 + 0] == 0 && Font->data[i1 * 3 + 1] == 255 && Font->data[i1 * 3 + 2] == 0)
		{
			Data[i * 4 + 0] = 0; Data[i * 4 + 1] = 255; Data[i * 4 + 2] = 0; Data[i * 4 + 3] = 0;
		}

		else if (Font->data[i1 * 3 + 0] == 255 && Font->data[i1 * 3 + 1] == 0 && Font->data[i1 * 3 + 2] == 0)
		{
			Data[i * 4 + 0] = 255; Data[i * 4 + 1] = 0; Data[i * 4 + 2] = 0;
		}

		else if (Font->data[i1 * 3 + 0] == 0 && Font->data[i1 * 3 + 1] == 0 && Font->data[i1 * 3 + 2] == 255)
			Data[i * 4 + 3] = 0;

		else
		{
			Data[i * 4 + 0] = Font->data[i1 * 3 + 0]; Data[i * 4 + 1] = Font->data[i1 * 3 + 1]; Data[i * 4 + 2] = Font->data[i1 * 3 + 2];
		}
		i1 += 1;
	}
	glGenTextures(1, &FontTexture);
	glBindTexture(GL_TEXTURE_2D, FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
	Symbols = new Symbol[SymbolsCount];
	for (WCHAR i = 0; i < SymbolsCount; i += 1)
	{
		Symbols[i].x = (i*ChWidth) % Width;
		Symbols[i].y = Height - (i*ChWidth) / (Width) *ChWidth - ChHeight;
		Symbols[i].Height = ChHeight;
		Symbols[i].Width = 0;
		while (Data[((Symbols[i].y + 1)*Width + Symbols[i].x + Symbols[i].Width) * 4] != 255)
			Symbols[i].Width += 1;
		Symbols[i].Symb = i;
	}
}
void Text::DrawSymbol(UCHAR Symb, int x, int y, int z, bool Center)
{
	glLoadIdentity();
	glOrtho(-400, 400, -300, 300, 0, -100);
	glTranslatef(x, y, z);
	glBindTexture(GL_TEXTURE_2D, FontTexture);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);

	if (Center == 1)
	{
		glTexCoord2d((Symbols[Symb].x + 0.0) / Width, (Symbols[Symb].y + 0.0) / Height);
		glVertex3f(-Symbols[Symb].Width / 2.0, -Symbols[Symb].Height / 2.0, 0);

		glTexCoord2d((Symbols[Symb].x + 0.0) / Width, (Symbols[Symb].y + Symbols[Symb].Height + 0.0) / Height);
		glVertex3f(-Symbols[Symb].Width / 2.0, Symbols[Symb].Height / 2.0, 0);

		glTexCoord2d((Symbols[Symb].x + Symbols[Symb].Width - 0.0) / Width, (Symbols[Symb].y + Symbols[Symb].Height + 0.0) / Height);
		glVertex3f(Symbols[Symb].Width / 2.0, Symbols[Symb].Height / 2.0, 0);

		glTexCoord2d((Symbols[Symb].x + Symbols[Symb].Width - 0.0) / Width, (Symbols[Symb].y + 0.0) / Height);
		glVertex3f(Symbols[Symb].Width / 2.0, -Symbols[Symb].Height / 2.0, 0);
	}
	else
	{
		glTexCoord2d((Symbols[Symb].x + 0.0) / Width, (Symbols[Symb].y + 0.0) / Height);
		glVertex3f(0, 0, 0);

		glTexCoord2d((Symbols[Symb].x + 0.0) / Width, (Symbols[Symb].y + Symbols[Symb].Height + 0.0) / Height);
		glVertex3i(0, Symbols[Symb].Height, 0);

		glTexCoord2d((Symbols[Symb].x + Symbols[Symb].Width - 0.0) / Width, (Symbols[Symb].y + Symbols[Symb].Height + 0.0) / Height);
		glVertex3i(Symbols[Symb].Width, Symbols[Symb].Height, 0);

		glTexCoord2d((Symbols[Symb].x + Symbols[Symb].Width - 0.0) / Width, (Symbols[Symb].y + 0.0) / Height);
		glVertex3i(Symbols[Symb].Width, 0, 0);
	}
	glEnd();
}
void Text::Draw(LPSTR String, int x, int y, int z, bool Center)
{
	PrimitiveList[z].Add(new TextExemplar(this, String, x, y, z, Center));
}

GraphicUnit::GraphicUnit(Image* Aimage, int DefaultZ)
{
	image = Aimage;
	z = DefaultZ;
}
void GraphicUnit::Draw()
{
	if (image == 0) return;
	image->Draw(x, y, z, angle);
}
GraphicUnit::~GraphicUnit()
{}

AnimatedGraphicUnit::AnimatedGraphicUnit(AnimatedImage* Image, int DefaultZ, bool Cicled)
{
	this->Image = Image;
	CurrentImage = 0;
	CurrentTime = 0;
	z = DefaultZ;
	this->Cicled = Cicled;
}
void AnimatedGraphicUnit::Draw()
{
	Image->Draw(CurrentImage, x, y, z, angle);
	CurrentTime++;
	if (CurrentTime >= Image->ImagesTime[CurrentImage])
	{
		CurrentImage++;
		CurrentTime = 0;
	}
	if (!Cicled && CurrentImage >= Image->ImagesCount)
	{
		Delete();
	}
	if (Cicled && CurrentImage >= Image->ImagesCount)
	{
		CurrentImage = 0;
	}
}
AnimatedGraphicUnit::~AnimatedGraphicUnit()
{}

//DefineFuncForEach(GraphicUnit, Draw)
//DefineFuncForEach(GraphicUnit, DrawProc)

//DefineFuncForEach(AnimatedGraphicUnit, Draw)
//DefineFuncForEach(AnimatedGraphicUnit, DrawProc)

void InitGraphic(HINSTANCE hInst, int nCmdShow, LPCWSTR WindowCaption)
{
	ghInst = hInst;
	gnCmdShow = nCmdShow;
	WindowThread = new thread(&WindowThreadProc, WindowCaption);
	WindowThread->detach();
	InitializeCriticalSection(&cs);
	unique_lock<mutex> Lock(Mutex, try_to_lock);
	Cond.wait(Lock);
}

void BeginDraw()
{
	EnterCriticalSection(&cs);
	for (int i = 0; i < 100; i++)
	{
		PrimitiveList[i].Clear();
	}
	wglMakeCurrent(hdc, hRC);
}
void EndDraw()
{
	InvalidateRect(hWnd, NULL, true);
	wglMakeCurrent(NULL, NULL);
	LeaveCriticalSection(&cs);
}

void WindowThreadProc(LPCWSTR WindowCaption)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"Class1";
	wc.cbWndExtra = NULL;
	wc.cbClsExtra = NULL;
	wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_LOGO));
	wc.hIconSm = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_LOGO));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.hInstance = ghInst;
	RegisterClassEx(&wc);

	RECT rect = {0, 0, 800, 600};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	hWnd = CreateWindow(
		L"Class1",
		WindowCaption,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		(HWND) NULL,
		NULL,
		HINSTANCE(ghInst),
		NULL);

	hdc = GetDC(hWnd);

	WindowWidth = 800;
	WindowHeigth = 600;
	xGlobal = 0;
	yGlobal = 0;
	GlobalScale = 1;

	int m_GLPixelIndex;
	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cRedShift = 16;
	pfd.cGreenBits = 8;
	pfd.cGreenShift = 8;
	pfd.cBlueBits = 8;
	pfd.cBlueShift = 0;
	pfd.cAlphaBits = 0;
	pfd.cAlphaShift = 0;
	pfd.cAccumBits = 64;
	pfd.cAccumRedBits = 16;
	pfd.cAccumGreenBits = 16;
	pfd.cAccumBlueBits = 16;
	pfd.cAccumAlphaBits = 0;
	pfd.cDepthBits = 32;
	pfd.cStencilBits = 8;
	pfd.cAuxBuffers = 0;
	pfd.dwLayerMask = 0;
	pfd.dwVisibleMask = 0;
	pfd.dwDamageMask = 0;

	m_GLPixelIndex = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, m_GLPixelIndex, &pfd);
	hRC = wglCreateContext(hdc);
	hdc = GetDC(hWnd);
	wglMakeCurrent(hdc, hRC);

	glClearColor(0, 0, 0, 0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);

	glDepthFunc(GL_LEQUAL);
	glAlphaFunc(GL_GREATER, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, WindowWidth, WindowHeigth);
	glMatrixMode(GL_MODELVIEW);

	ShowWindow(hWnd, gnCmdShow);
	UpdateWindow(hWnd);

	Cond.notify_one();
	MSG msg;
	while (GetMessage(&msg, hWnd, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HCURSOR Cursor = LoadCursor(NULL, IDC_ARROW);
	if (uMsg == WM_CREATE)
	{
	}
	else if (uMsg == WM_SETCURSOR)
	{
		if (LOWORD(lParam) == HTCLIENT)
		{
			SetCursor(NULL);
		}
		else
		{
			SetCursor(Cursor);
		}
	}
	else if (uMsg == WM_PAINT)
	{
		EnterCriticalSection(&cs);
		wglMakeCurrent(hdc, hRC);
		bool b = false;
		for (int i = 99; i >= 0; i--)
		{
			if (PrimitiveList[i].First->Next != PrimitiveList[i].Last)
				b = true;
			ForList(PrimitiveList[i], Primitive)
			{
				Primitive->Draw();
			}
		}
		SwapBuffers(hdc);
		/*static COLORREF PrevColor=0, Color;
		Color = GetPixel(hdc, 10, 10);
		if (Color != PrevColor)
		{
			b = b;
		}
		PrevColor = Color;
		if (!b)
		{
			b=b;
		}*/
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		wglMakeCurrent(NULL, NULL);
		ValidateRect(hwnd, NULL);
		//Mutex.unlock();
		LeaveCriticalSection(&cs);
	}
	else if (uMsg == WM_DESTROY)
	{
		ReleaseDC(hwnd, hdc);
		PostQuitMessage(NULL);
		Closed = true;
	}
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	return NULL;
}