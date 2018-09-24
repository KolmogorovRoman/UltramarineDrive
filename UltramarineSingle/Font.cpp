#include "Font.h"

void Font::FileHeader::Serialize()
{
	Serial(B, M, F, Version);
}
void Font::BlockHeader::Serialize()
{
	Serial(BlockTypeId, BlockSize);
}
void Font::InfoBlock::Serialize()
{
	Serial(FontSize, BitField, CharSet, StretchH, AA, PaddingUp, PaddingRight, PaddingDown,
		PaddingLeft, SpacingHoriz, SpacingVert, Outline);
	if (Deserialazing)
	{
		FontName = new char[BlockSize - SizeWithoutName];
		ReadBytes(FontName, BlockSize - SizeWithoutName);
	}
}
void Font::CommonBlock::Serialize()
{
	Serial(LineHeight, Base, ScaleW, ScaleH, Pages, BitField, AlphaChnl,
		RedChnl, GreenChnl, BlueChnl);
}
void Font::PagesBlock::Serialize()
{
	if (Deserialazing)
	{
		PageNames = new char[PageNames, BlockSize];
		ReadBytes(PageNames, BlockSize);
	}
}
void Font::CharBlock::Serialize()
{
	Serial(Id, X, Y, Width, Height, XOffset, YOffset, XAdvance, Page, Chnl);
}
void Font::KerningPairBlock::Serialize()
{
	Serial(First, Second, Amount);
}

Font::Char::Char(CharBlock& charblock):
	X(charblock.X), Y(charblock.Y), Width(charblock.Width), Height(charblock.Height),
	XOffset(charblock.XOffset), YOffset(charblock.YOffset), XAdvance(charblock.XAdvance),
	Page(charblock.Page), Chnl(charblock.Chnl)
{}

Font::Font(string Dir, string FileName)
{
	ifstream File(Dir + "\\" + FileName + ".fnt", ios::in | ios::binary);
	if (!File.is_open()) throw "File not found";
	File.seekg(0, ios_base::end);
	unsigned int FileSize = File.tellg();
	File.seekg(0, ios_base::beg);
	BytesArray* Array = BytesArray::New(FileSize);
	Array->Reverse = !IsLittleEndian();
	File.read((char*)Array->Array, Array->Size);
	FileHeader fileheader;
	Array->Deserialize(fileheader);
	while (Array->Pointer < Array->Size)
	{
		BlockHeader Header;
		Array->Deserialize(Header);
		if (Header.BlockTypeId == InfoBlock::TypeID)
		{
			(BlockHeader&)Info = Header;
			Array->Deserialize(Info);
		}
		if (Header.BlockTypeId == CommonBlock::TypeID)
		{
			(BlockHeader&)Common = Header;
			Array->Deserialize(Common);
		}
		if (Header.BlockTypeId == PagesBlock::TypeID)
		{
			PagesBlock pagesblock;
			(BlockHeader&)pagesblock = Header;
			Array->Deserialize(pagesblock);
			int pagenamelen = strlen(pagesblock.PageNames);
			int pagescount = pagesblock.BlockSize / pagenamelen;
			GLuint* Textures = new GLuint[pagescount];
			glGenTextures(pagescount, Textures);
			Pages = new Page[pagescount];
			for (int i = 0; i < pagescount; i++)
			{
				Pages[i].Texture = Textures[i];
				char* FileName = pagesblock.PageNames + pagenamelen*i;
				FileName[strlen(FileName) - 3] = 0;
				AUX_RGBImageRec* ImageRec = auxDIBImageLoadA(
					(Dir + "\\" + (pagesblock.PageNames + pagenamelen*i) + "bmp").c_str());
				Pages[i].Width = ImageRec->sizeX;
				Pages[i].Height = ImageRec->sizeY;
				GLubyte* Data = new GLubyte[ImageRec->sizeX*ImageRec->sizeY * 4];
				for (int i = 0; i < ImageRec->sizeX*ImageRec->sizeY; i++)
				{
					if (ImageRec->data[i * 3 + 0] == 0 &&
						ImageRec->data[i * 3 + 1] == 0 &&
						ImageRec->data[i * 3 + 2] == 0)
					{
						Data[i * 4 + 0] = 0;
						Data[i * 4 + 1] = 0;
						Data[i * 4 + 2] = 0;
						Data[i * 4 + 3] = 0;
					}
					else
					{
						Data[i * 4 + 0] = 0xFF;
						Data[i * 4 + 1] = 0xFF;
						Data[i * 4 + 2] = 0xFF;
						Data[i * 4 + 3] = 0xFF;
					}
				}
				glBindTexture(GL_TEXTURE_2D, Textures[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, 4, ImageRec->sizeX, ImageRec->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
				delete[] Data;
			}
			delete[] Textures;
		}
		if (Header.BlockTypeId == CharBlock::TypeID)
		{
			int CharsCount = Header.BlockSize / CharBlock::Size;
			for (int i = 0; i < CharsCount; i++)
			{
				CharBlock charblock;
				Array->Deserialize(charblock);
				Chars.insert(make_pair((char)charblock.Id, new Char(charblock)));
			}
		}
		if (Header.BlockTypeId == KerningPairBlock::TypeID)
		{
			int PairsCount = Header.BlockSize / KerningPairBlock::Size;
			for (int i = 0; i < PairsCount; i++)
			{
				KerningPairBlock pairblock;
				Array->Deserialize(pairblock);
				KerningTable.insert(make_pair(make_pair((char)pairblock.First, (char)pairblock.Second),
					pairblock.Amount));
			}
		}
	}
}
int Font::TextWidth(string Text, bool Indents)
{
	int Width = 0;
	int Len = Text.size();
	for (int i = 0; i < Len; i++)
	{
		char c = Text[i];
		Char* C = Chars[c];
		Width += C->XAdvance;
		if (i + 1 < Len)
		{
			auto res = KerningTable.find(make_pair(c, Text[i + 1]));
			if (res != KerningTable.end())
				Width += (*res).second;
		}
	}
	if (Indents && Len > 0)
	{
		Char* Space = Chars[' '];
		Width += Space->Width;
	}
	if (!Indents && Len > 0)
	{
		char last = Text[Len - 1];
		Char* Last = Chars[last];
		Width -= Last->XAdvance - Last->Width;
	}
	return Width;
}
void Font::Draw(string Text, double X, double Y, Layer* layer, bool CenterX,
	bool Base, bool Indents, double R, double G, double B, double A)
{
	layer->Add([this, Text, X, Y, CenterX, Base, Indents, R, G, B, A]()
	{
		glPushMatrix();
		if (CenterX) glTranslated(-TextWidth(Text, Indents) / 2, 0, 0);
		int Len = Text.size();
		if (Base) glTranslated(0, Common.Base, 0);
		if (Indents && Len>0)
		{
			Char* Space = Chars[' '];
			glTranslated(Space->Width, 0, 0);
		}
		for (int i = 0; i < Len; i++)
		{
			char c = Text[i];
			Char* C = Chars[c];
			Page P = Pages[C->Page];
			glPushMatrix();
			glTranslated(C->XOffset, -C->YOffset, 0);
			glBindTexture(GL_TEXTURE_2D, P.Texture);
			glColor4f(R, G, B, A);

			glBegin(GL_QUADS);
			glTexCoord2d(C->X / P.Width, 1 - C->Y / P.Height);
			glVertex2i(X, Y);

			glTexCoord2d((C->X + C->Width) / P.Width, 1 - C->Y / P.Height);
			glVertex2i(X + C->Width, Y);

			glTexCoord2d((C->X + C->Width) / P.Width, 1 - (C->Y + C->Height) / P.Height);
			glVertex2i(X + C->Width, Y - C->Height);

			glTexCoord2d(C->X / P.Width, 1 - (C->Y + C->Height) / P.Height);
			glVertex2i(X, Y - C->Height);
			glEnd();

			glPopMatrix();
			glTranslated(C->XAdvance, 0, 0);
			if (i + 1 < Len)
			{
				auto res = KerningTable.find(make_pair(c, Text[i + 1]));
				if (res != KerningTable.end())
					glTranslated((*res).second, 0, 0);
			}
		}
		glPopMatrix();
	});
}