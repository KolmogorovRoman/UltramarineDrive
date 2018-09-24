#pragma once
#include "Serialization.h"
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include "Graphic.h"
using namespace std;

class Font
{
	struct FileHeader:public Serialazable
	{
		UINT8 B;
		UINT8 M;
		UINT8 F;
		UINT8 Version;
		void Serialize();
		static const int Size = 4;
	};
	struct BlockHeader: public Serialazable
	{
	public:
		UINT8 BlockTypeId;
		UINT32 BlockSize;
		static const int Size = 5;
		void Serialize();
	};
	struct InfoBlock: public BlockHeader, public Serialazable
	{
		INT16 FontSize;
		UINT8 BitField;
		UINT8 CharSet;
		UINT16 StretchH;
		UINT8 AA;
		UINT8 PaddingUp;
		UINT8 PaddingRight;
		UINT8 PaddingDown;
		UINT8 PaddingLeft;
		UINT8 SpacingHoriz;
		UINT8 SpacingVert;
		UINT8 Outline;
		char* FontName;
		void Serialize();
		static const int SizeWithoutName = 14;
		static const int TypeID = 1;
	};
	struct CommonBlock:public BlockHeader, public Serialazable
	{
		UINT16 LineHeight;
		UINT16 Base;
		UINT16 ScaleW;
		UINT16 ScaleH;
		UINT16 Pages;
		UINT8 BitField;
		UINT8 AlphaChnl;
		UINT8 RedChnl;
		UINT8 GreenChnl;
		UINT8 BlueChnl;
		void Serialize();
		static const int TypeID = 2;
	};
	struct PagesBlock:public BlockHeader, public Serialazable
	{
		char* PageNames;
		void Serialize();
		static const int TypeID = 3;
	};
	struct CharBlock: public Serialazable
	{
		UINT32 Id;
		UINT16 X;
		UINT16 Y;
		UINT16 Width;
		UINT16 Height;
		INT16 XOffset;
		INT16 YOffset;
		INT16 XAdvance;
		UINT8 Page;
		UINT8 Chnl;
		void Serialize();
		static const int Size = 20;
		static const int TypeID = 4;
	};
	struct KerningPairBlock: public Serialazable
	{
		UINT32 First;
		UINT32 Second;
		INT16 Amount;
		void Serialize();
		static const int Size = 10;
		static const int TypeID = 5;
	};

	struct Page
	{
		GLuint Texture;
		double Width;
		double Height;
	};
	Page* Pages;
	class Char
	{
	public:
		UINT16 X;
		UINT16 Y;
		UINT16 Width;
		UINT16 Height;
		INT16 XOffset;
		INT16 YOffset;
		INT16 XAdvance;
		UINT8 Page;
		UINT8 Chnl;
		Char(CharBlock& charblock);
		void Draw();
	};
	map<char, Char*> Chars;
public:
	map<pair<char, char>, int> KerningTable;
	InfoBlock Info;
	CommonBlock Common;
	Font(string Dir, string FileName);
	int TextWidth(string Text, bool Indents=true);
	void Draw(string Text, double X, double Y, Layer* layer, bool CenterX = false, 
		bool Base = false, bool Indents =true, double R = 1, double G = 1, double B = 1, double A = 1);
};