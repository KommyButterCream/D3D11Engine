#pragma once

#ifdef GRAPHICS_ENGINE_DLL
#define FONT_MANAGER_API __declspec(dllexport)
#else
#define FONT_MANAGER_API __declspec(dllimport)
#endif

struct IDWriteFactory;
struct IDWriteTextFormat;
enum DWRITE_FONT_WEIGHT : int;
class FontManagerImpl;

class FONT_MANAGER_API FontManager
{
public:
	FontManager();
	~FontManager();

public:
	bool Initialize(IDWriteFactory* factory);
	void Shutdown();

	IDWriteTextFormat* GetTextFormat(const wchar_t* fontName, float fontSize, DWRITE_FONT_WEIGHT weight);
	IDWriteFactory* GetDWriteFactory();

private:
	FontManagerImpl* m_impl = nullptr;
};

