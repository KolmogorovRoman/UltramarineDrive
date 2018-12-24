#include "Graphic.h"

HDC hdc;
HGLRC  hRC;
HWND hWnd;
GLuint textures[IMAGECOUNT];
mutex Mutex;
CRITICAL_SECTION cs;
condition_variable Cond;

HINSTANCE ghInst;
int gnCmdShow;
thread* WindowThread;
bool Closed = false;
bool NeedFullscreen = false;
bool NeedWindowed = false;

void DrawLine(double x1, double y1, double x2, double y2, Layer* layer, double R, double G, double B, double A)
{
	layer->Add(*(new LineExemplar(x1, y1, x2, y2, R, G, B, A)));
}
void DrawRect(double x1, double y1, double x2, double y2, bool Filled, Layer* layer, double R, double G, double B, double A)
{
	layer->Add(*(new RectExemplar(x1, y1, x2, y2, Filled, R, G, B, A)));
}
void DrawBezier(Vector P1, Vector P2, Vector P3, Vector P4, Layer* layer, double R = 0, double G = 0, double B = 0, double A = 0)
{
	layer->Add(*(new BezierExemplar(P1, P2, P3, P4, R, G, B, A)));
}

LineExemplar::LineExemplar(double x1, double y1, double x2, double y2, double R, double G, double B, double A):
	x1(x1), y1(y1), x2(x2), y2(y2), R(R), G(G), B(B), A(A)
{}
void LineExemplar::operator()()
{
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(R, G, B, A);
	glBegin(GL_LINES);
	glVertex3i(x1, y1, 0);
	glVertex3i(x2, y2, 0);
	glEnd();
	glPopMatrix();
}

RectExemplar::RectExemplar(double x1, double y1, double x2, double y2, bool Filled, double R, double G, double B, double A):
	x1(ceil(min(x1, x2))), y1(ceil(min(y1, y2))),
	x2(floor(max(x1, x2))), y2(floor(max(y1, y2))),
	Filled(Filled), R(R), G(G), B(B), A(A)
{}
void RectExemplar::operator()()
{
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(R, G, B, A);
	if (Filled) glBegin(GL_POLYGON);
	else glBegin(GL_LINE_LOOP);
	glVertex2d(x1 + 0.5, y1 + 0.5);
	glVertex2d(x2 + 0.5, y1 + 0.5);
	glVertex2d(x2 + 0.5, y2 + 0.5);
	glVertex2d(x1 + 0.5, y2 + 0.5);
	glEnd();
	glPopMatrix();
}

ImageExemplar::ImageExemplar(Image* image, double x, double y, double angle, double wScale, double hScale, bool OverScreen):
	image(image),
	x(x),
	y(y),
	angle(angle),
	wScale(wScale),
	hScale(hScale),
	OverScreen(OverScreen)
{}
void ImageExemplar::operator()()
{
	glPushMatrix();
	glTranslatef(x, y, 0);
	glRotatef(angle, 0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, image->TextureIndex);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex3i(-image->Center.x * wScale, -image->Center.y * hScale, 0);
	glTexCoord2i(0, 1); glVertex3i(-image->Center.x * wScale, (image->Height - image->Center.y) * hScale, 0);
	glTexCoord2i(1, 1); glVertex3i((image->Width - image->Center.x) * wScale, (image->Height - image->Center.y) * hScale, 0);
	glTexCoord2i(1, 0); glVertex3i((image->Width - image->Center.x) * wScale, -image->Center.y * hScale, 0);
	glEnd();
	glPopMatrix();
}

BezierExemplar::BezierExemplar(Vector P1, Vector P2, Vector P3, Vector P4, double R, double G, double B, double A):
	P1(P1), P2(P2), P3(P3), P4(P4), R(R), G(G), B(B), A(A)
{}
void BezierExemplar::operator()()
{
	glColor4d(R, G, B, A);
	glBegin(GL_LINE_STRIP);
	for (double t = 0; t <= 1; t += 0.01)
	{
		double t1 = 1 - t;
		Vector P = P1*t*t*t + 3 * P2*t*t*t1 + 3 * P3*t*t1*t1 + P4*t1*t1*t1;
		glVertex2d(P.x, P.y);
	}
	glEnd();
}

#define BORDER 1
Image::Image(string Name, string Maskname, int Centerx, int Centery)
{
	GLubyte* Data;
	texture = auxDIBImageLoadA(Name.c_str());
	mask = auxDIBImageLoadA(Maskname.c_str());
	RealWidth = texture->sizeX;
	RealHeight = texture->sizeY;
	Width = pow(2, ceil(log(RealWidth + BORDER*2.0) / log(2.0)));
	Height = pow(2, ceil(log(RealHeight + BORDER*2.0) / log(2.0)));
	Center.x = Centerx + BORDER;
	Center.y = Centery + BORDER;
	Data = new GLubyte[Height*Width * 4];
	for (int i = 0; i < Height*Width * 4; i += 1)
	{
		Data[i] = 0;
	}
	for (int iy = 0; iy < RealHeight; iy += 1)
		for (int ix = 0; ix < RealWidth; ix += 1)
		{
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 0] = texture->data[((iy)*RealWidth + ix) * 3 + 0];
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 1] = texture->data[((iy)*RealWidth + ix) * 3 + 1];
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 2] = texture->data[((iy)*RealWidth + ix) * 3 + 2];
			Data[((iy + BORDER)*Width + ix + BORDER) * 4 + 3] = mask->data[((iy)*RealWidth + ix) * 3 + 0];
		}
	glGenTextures(1, &TextureIndex);
	glBindTexture(GL_TEXTURE_2D, TextureIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
}
Image::Image(string Name)
{
	BYTE* Data;
	texture = auxDIBImageLoadA(Name.c_str());
	RealWidth = texture->sizeX;
	RealHeight = texture->sizeY;
	Width = pow(2, ceil(log(RealWidth) / log(2.0)));
	Height = pow(2, ceil(log(RealHeight) / log(2.0)));
	Data = new BYTE[Height * Width * 4];
	BYTE* VoidColor = texture->data;
	BYTE* EndColor = &texture->data[(RealWidth*RealHeight - 1) * 3];
	bool TransparencyVoid = true;
	if (VoidColor[0] != EndColor[0] || VoidColor[1] != EndColor[1] || VoidColor[2] != EndColor[2])
		TransparencyVoid = false;
	Center.x = 0;
	Center.y = 0;
	for (int i = 0; i < RealWidth; i++)
	{
		if (texture->data[i * 3 + 0] != VoidColor[0] ||
			texture->data[i * 3 + 1] != VoidColor[1] ||
			texture->data[i * 3 + 2] != VoidColor[2])
			Center.x = i + 1;
	}
	for (int j = 0; j < RealHeight; j++)
	{
		if (texture->data[j * RealWidth * 3 + 0] != VoidColor[0] ||
			texture->data[j * RealWidth * 3 + 1] != VoidColor[1] ||
			texture->data[j * RealWidth * 3 + 2] != VoidColor[2])
			Center.y = j + 1;
	}
	for (int i = 0; i < Width * Height * 4; i++)
		Data[i] = 0;
	for (int iy = 1; iy < RealHeight - 1; iy += 1)
		for (int ix = 1; ix < RealWidth - 1; ix += 1)
		{
			Data[(iy*Width + ix) * 4 + 0] = texture->data[((iy)* RealWidth + ix) * 3 + 0];
			Data[(iy*Width + ix) * 4 + 1] = texture->data[((iy)* RealWidth + ix) * 3 + 1];
			Data[(iy*Width + ix) * 4 + 2] = texture->data[((iy)* RealWidth + ix) * 3 + 2];
			if (Data[(iy*Width + ix) * 4 + 0] == VoidColor[0] &&
				Data[(iy*Width + ix) * 4 + 1] == VoidColor[1] &&
				Data[(iy*Width + ix) * 4 + 2] == VoidColor[2] &&
				TransparencyVoid)
			{
				Data[(iy*Width + ix) * 4 + 0] = 0;
				Data[(iy*Width + ix) * 4 + 1] = 0;
				Data[(iy*Width + ix) * 4 + 2] = 0;
				Data[(iy*Width + ix) * 4 + 3] = 0;
			}
			else Data[(iy*Width + ix) * 4 + 3] = 255;
		}
	RealWidth -= 2;
	RealHeight -= 2;
	glGenTextures(1, &TextureIndex);
	glBindTexture(GL_TEXTURE_2D, TextureIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
}
void Image::Draw(double x, double y, Layer* layer, double angle, double wScale, double hScale, bool OverScreen)
{
	layer->Add(*(new ImageExemplar(this, x, y, angle, wScale, hScale, OverScreen)));
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
		ReadedImage = (Image**)((char*)(ReadedImage)+sizeof(Image*) + sizeof(int));
		ReadedTime = (int*)((char*)(ReadedTime)+sizeof(Image*) + sizeof(int));
	}
}
void AnimatedImage::Draw(int ImageNumber, int x, int y, Layer* layer, double angle, double wScale, double hScale, bool OverScreen)
{
	Images[ImageNumber]->Draw(x, y, layer, angle, wScale, hScale, OverScreen);
}
AnimatedImage::~AnimatedImage()
{
	delete[] Images;
	delete[] ImagesTime;
}

std::list<Layer*> Layer::LayersList;
std::list<ModelExemplar*> ModelsExemplarsList;
Layer::Layer(bool IsInterface):
	IsInterface(IsInterface)
{
	//typedef void(__stdcall *GL_GEN_FRAMEBUFFERS) (GLsizei, GLuint*);
	//GL_GEN_FRAMEBUFFERS glGenFramebuffers = (GL_GEN_FRAMEBUFFERS)wglGetProcAddress("glGenFramebuffers");
	//typedef void(__stdcall *GL_BIND_FRAMEBUFFER) (GLenum target, GLuint framebuffer);
	//GL_BIND_FRAMEBUFFER glBindFramebuffer = (GL_BIND_FRAMEBUFFER)wglGetProcAddress("glBindFramebuffer");
	//glGenFramebuffers(1, &FrameBuffer);
	//glBindFramebuffer(36160, FrameBuffer); //36160 is GL_FRAMEBUFFER
	//glGenTextures(1, &Texture);
	//glBindTexture(GL_TEXTURE_2D, Texture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, MainCamera.Width, MainCamera.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
Layer* Layer::New(bool IsInterface)
{
	Layer* NewLayer = new Layer(IsInterface);
	LayersList.push_back(NewLayer);
	NewLayer->Iterator = std::prev(LayersList.end());
	return NewLayer;
}
Layer* Layer::Higher(Layer* Other, bool IsInterface)
{
	Layer* NewLayer = new Layer(IsInterface);
	NewLayer->Iterator = LayersList.insert(std::next(Other->Iterator), NewLayer);
	return NewLayer;
}
Layer* Layer::Lower(Layer* Other, bool IsInterface)
{
	Layer* NewLayer = new Layer(IsInterface);
	NewLayer->Iterator = LayersList.insert(Other->Iterator, NewLayer);
	return NewLayer;
}

GraphicUnit::GraphicUnit(Image* image, Layer* layer):
	image(image),
	layer(layer)
{}
void GraphicUnit::Draw()
{
	if (image == NULL) return;
	image->Draw(x, y, layer, angle);
}
bool GraphicUnit::InScreen()
{
	if (x<MainCamera.x - MainCamera.Width / 2 * MainCamera.Scale || y<MainCamera.y - MainCamera.Height / 2 * MainCamera.Scale ||
		x>MainCamera.x + MainCamera.Width / 2 * MainCamera.Scale || y>MainCamera.y + MainCamera.Height / 2 * MainCamera.Scale)
		return false;
	else return true;
}
GraphicUnit::~GraphicUnit()
{}

AnimatedGraphicUnit::AnimatedGraphicUnit(AnimatedImage* Image, Layer* layer, bool Cicled):
	Image(Image),
	layer(layer),
	Cicled(Cicled),
	CurrentImage(0),
	CurrentTime(0)
{}
void AnimatedGraphicUnit::Draw()
{
	Image->Draw(CurrentImage, x, y, layer, angle);
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
bool AnimatedGraphicUnit::InScreen()
{
	if (x<-MainCamera.Width / 2 || y<-MainCamera.Height / 2 || x>MainCamera.Width / 2 || y>MainCamera.Height / 2) return false;
	else return true;
}
AnimatedGraphicUnit::~AnimatedGraphicUnit()
{}

Camera MainCamera;
extern void CameraChanged();

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
	wglMakeCurrent(hdc, hRC);
}
void EndDraw()
{
	InvalidateRect(hWnd, NULL, true);
	wglMakeCurrent(NULL, NULL);
	LeaveCriticalSection(&cs);
}

GLuint base;
GLYPHMETRICSFLOAT gmf[256];
HFONT Font;

GLuint VBO;

Model::Model(string Path, string FileName)
{
	glGenBuffers(1, &VboIndex);
	glGenBuffers(1, &ColorsIndex);
	glGenBuffers(1, &NormalsIndex);
	ifstream File(Path + FileName);
	map<string, Material*> Materials;
	Material* CurrentMaterial;
	do
	{
		string Line;
		getline(File, Line);
		stringstream LineStream(Line);
		string Command;
		LineStream >> Command;
		if (Command == "mtllib"s)
		{
			string MtlFileName;
			LineStream >> MtlFileName;
			ifstream MtlFile(Path + MtlFileName);
			string CurrentMaterialName;
			do
			{
				string Line;
				getline(MtlFile, Line);
				stringstream LineStream(Line);
				string Command;
				LineStream >> Command;
				if (Command == "newmtl"s)
				{
					LineStream >> CurrentMaterialName;
					Materials[CurrentMaterialName] = new Material();
				}
				if (Command == "Kd"s)
				{
					LineStream >>
						Materials[CurrentMaterialName]->Diffuse.r >>
						Materials[CurrentMaterialName]->Diffuse.g >>
						Materials[CurrentMaterialName]->Diffuse.b;
				}
			} while (MtlFile);
		}
		if (Command == "v"s)
		{
			Vertex v;
			LineStream >> v.x >> v.y >> v.z;
			Vertexes.push_back(v);
		}
		if (Command == "vt"s)
		{
			TextureVertex v;
			LineStream >> v.u >> v.v;
			TextureVertexes.push_back(v);
		}
		if (Command == "vn"s)
		{
			Normale n;
			LineStream >> n.x >> n.y >> n.z;
			double r = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
			n.x /= r;
			n.y /= r;
			n.z /= r;
			Normales.push_back(n);
		}
		if (Command == "f"s)
		{
			Face f;
			char slash;
			for (int i = 0; i < 3; i++)
			{
				LineStream >> f.Vertexes[i].VertexIndex;
				if ((char)LineStream.peek() == '/')
				{
					LineStream >> slash;
					if ((char)LineStream.peek() == '/')
					{
						LineStream >> slash >> f.Vertexes[i].NormalIndex;
					}
					else
					{
						LineStream >> f.Vertexes[i].TextureIndex;
						if ((char)LineStream.peek() == '/')
						{
							LineStream >> slash >> f.Vertexes[i].NormalIndex;
						}
					}
				}
			}
			if (f.Vertexes[0].NormalIndex == 0)
			{
				Normale n;
				double a[3] = {
					Vertexes[f.Vertexes[1].VertexIndex - 1].x
					- Vertexes[f.Vertexes[0].VertexIndex - 1].x,
					Vertexes[f.Vertexes[1].VertexIndex - 1].y
					- Vertexes[f.Vertexes[0].VertexIndex - 1].y,
					Vertexes[f.Vertexes[1].VertexIndex - 1].z
					- Vertexes[f.Vertexes[0].VertexIndex - 1].z, };
				double b[3] = {
					Vertexes[f.Vertexes[2].VertexIndex - 1].x
					- Vertexes[f.Vertexes[0].VertexIndex - 1].x,
					Vertexes[f.Vertexes[2].VertexIndex - 1].y
					- Vertexes[f.Vertexes[0].VertexIndex - 1].y,
					Vertexes[f.Vertexes[2].VertexIndex - 1].z
					- Vertexes[f.Vertexes[0].VertexIndex - 1].z, };
				n.x = a[1] * b[2] - a[2] * b[1];
				n.y = a[2] * b[0] - a[0] * b[2];
				n.z = a[0] * b[1] - a[1] * b[0];
				double r = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
				n.x /= r;
				n.y /= r;
				n.z /= r;
				Normales.push_back(n);
				f.Vertexes[0].NormalIndex = Normales.size();
				f.Vertexes[1].NormalIndex = Normales.size();
				f.Vertexes[2].NormalIndex = Normales.size();
			}
			f.Material = CurrentMaterial;
			Faces.push_back(f);
		}
		if (Command == "usemtl"s)
		{
			string MtlName;
			LineStream >> MtlName;
			CurrentMaterial = Materials[MtlName];
		}

	} while (File);
	GLfloat* VboData = new GLfloat[Faces.size() * 3 * 3];
	GLfloat* ColorsData = new GLfloat[Faces.size() * 3 * 3];
	GLfloat* NormalsData = new GLfloat[Faces.size() * 3 * 3];
	int i = 0;
	for (Face f : Faces)
	{
		for (int k = 0; k < 3; k++)
		{
			VboData[i * 3 * 3 + k * 3 + 0] = Vertexes[f.Vertexes[k].VertexIndex - 1].x;
			VboData[i * 3 * 3 + k * 3 + 1] = Vertexes[f.Vertexes[k].VertexIndex - 1].y;
			VboData[i * 3 * 3 + k * 3 + 2] = Vertexes[f.Vertexes[k].VertexIndex - 1].z;
			ColorsData[i * 3 * 3 + k * 3 + 0] = f.Material->Diffuse.r;
			ColorsData[i * 3 * 3 + k * 3 + 1] = f.Material->Diffuse.g;
			ColorsData[i * 3 * 3 + k * 3 + 2] = f.Material->Diffuse.b;
			NormalsData[i * 3 * 3 + k * 3 + 0] = Normales[f.Vertexes[k].NormalIndex - 1].x;
			NormalsData[i * 3 * 3 + k * 3 + 1] = Normales[f.Vertexes[k].NormalIndex - 1].y;
			NormalsData[i * 3 * 3 + k * 3 + 2] = Normales[f.Vertexes[k].NormalIndex - 1].z;
		}
		i++;
	}
	glBindBuffer(GL_ARRAY_BUFFER, VboIndex);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat)) * Faces.size() * 3 * 3, VboData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, ColorsIndex);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat)) * Faces.size() * 3 * 3, ColorsData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, NormalsIndex);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat)) * Faces.size() * 3 * 3, NormalsData, GL_STATIC_DRAW);
	delete[] VboData;
	delete[] ColorsData;
	delete[] NormalsData;
}
void Model::Draw(double x, double y, double z, double angle)
{
	ModelsExemplarsList.push_back(new ModelExemplar(this, x, y, z, angle));
}

ModelExemplar::ModelExemplar(Model* model, double x, double y, double z, double angle):
	model(model),
	x(x),
	y(y),
	z(z),
	angle(angle)
{}
void ModelExemplar::operator()()
{
	glPushMatrix();
	glTranslated(x, y, z);
	glRotated(angle, 0, 0, 1);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, model->ColorsIndex);
	glColorPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, model->NormalsIndex);
	glNormalPointer(GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, model->VboIndex);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, model->Faces.size() * 3);
	glFlush();
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
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
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hInstance = ghInst;
	RegisterClassEx(&wc);

	RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	hWnd = CreateWindow(
		L"Class1",
		WindowCaption,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		(HWND)NULL,
		NULL,
		HINSTANCE(ghInst),
		NULL);

	hdc = GetDC(hWnd);

	MainCamera.Width = rect.right - rect.left;
	MainCamera.Height = rect.bottom - rect.top;

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

	glewInit();

	glClearColor(0, 0, 0, 0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	GLfloat AmbientColor[4] = { 0.1, 0.1, 0.1, 1 };
	GLfloat DiffuseColor[4] = { 0.6, 0.6, 0.4, 1 };
	//GLfloat Position[4] = { 0.6, 0, -0.8, 0 };
	GLfloat Position[4] = { 0.1, 0.6, -0.8, 0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);
	glLightfv(GL_LIGHT0, GL_POSITION, Position);

	glDepthFunc(GL_LEQUAL);
	glAlphaFunc(GL_GREATER, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, MainCamera.Width, MainCamera.Height);
	glMatrixMode(GL_MODELVIEW);

	DWORD TaskIndex = 0;

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
	if (NeedFullscreen)
	{
		BeginDraw();
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);
		ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		NeedFullscreen = false;
		EndDraw();
	}
	if (NeedWindowed)
	{
		BeginDraw();
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		NeedWindowed = false;
		EndDraw();
	}
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
		glClearColor(0.5, 0.5, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glViewport(0, 0, MainCamera.Width, MainCamera.Height);
		double n = 150;
		double f = 1100;
		double c = n / f;
		glFrustum(-MainCamera.Width / 2 * MainCamera.Scale * c, MainCamera.Width / 2 * MainCamera.Scale* c,
			-MainCamera.Height / 2 * MainCamera.Scale* c, MainCamera.Height / 2 * MainCamera.Scale* c, n, f);
		SetCamera();
		
		glTranslatef(0, 0, -300);
		
		glRotatef(-75, 1, 0, 0);
		glTranslatef(0, -100, 0);
		glRotatef(MainCamera.angle, 0, 0, 1);
		glTranslatef(-MainCamera.x, -MainCamera.y, 0);

		glPushMatrix();
		//glRotatef(-MainCamera.angle, 0, 0, 1);
		//glRotatef(75, 1, 0, 0);
		//GLfloat Position[4] = { dcos(-MainCamera.angle), 1, dsin(-MainCamera.angle), 0 };
		GLfloat Position[4] = { 0.1, -1, 0, 0 };
		//glLightfv(GL_LIGHT0, GL_POSITION, Position);
		/*glBegin(GL_LINES);
		glColor3f(1, 1, 1);
		glVertex3f(Position[0] * 50, Position[1] * 50, Position[2] * 50);
		glColor3f(1, 1, 0);
		glVertex3f(-Position[0] * 50, -Position[1] * 50, -Position[2] * 50);
		glEnd();*/
		glPopMatrix();
		glEnable(GL_CULL_FACE);
		for (auto Primitive : ModelsExemplarsList)
		{
			(*Primitive)();
			delete Primitive;
		}
		glDisable(GL_CULL_FACE);
		ModelsExemplarsList.clear();
		double z = 0;
		for (auto layer : Layer::LayersList)
		{
			glTranslated(0, 0, z);
			if (layer->IsInterface)
			{
				glDisable(GL_LIGHTING);
				glDisable(GL_DEPTH_TEST);
				glPushMatrix();
				glLoadIdentity();
				glViewport(0, 0, MainCamera.Width, MainCamera.Height);
				glOrtho(-MainCamera.Width / 2, MainCamera.Width / 2, -MainCamera.Height / 2, MainCamera.Height / 2, 0, -100);
			}
			for (auto Primitive : layer->Content)
			{
				(*Primitive)();
			}
			if (layer->IsInterface)
			{
				glPopMatrix();
				glEnable(GL_LIGHTING);
				glEnable(GL_DEPTH_TEST);
			}
			for (auto Primitive : layer->Content) delete Primitive;
			layer->Content.clear();
			z += 0.001;
		}
		/*glPushAttrib(GL_LIST_BIT);
		glListBase(base);
		LPSTR text = "Quick brown fox jump over lazy dog";
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
		glPopAttrib();*/

		SwapBuffers(hdc);
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
	else if (uMsg == WM_SIZE)
	{
		MainCamera.Width = LOWORD(lParam);
		MainCamera.Height = HIWORD(lParam);
		CameraChanged();
	}
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	return NULL;
}