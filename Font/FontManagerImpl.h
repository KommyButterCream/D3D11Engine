#pragma once

#include <map>
#include <string>
#include <tuple>
#include <stdint.h>

enum DWRITE_FONT_WEIGHT : int;
struct IDWriteTextFormat;
struct IDWriteFactory;

struct FontKey
{
	std::wstring name;
	int32_t size = 0;
	DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL;

	// Ordering key for std::map caching.
	bool operator<(const FontKey& other) const
	{
		return std::tie(name, size, weight) < std::tie(other.name, other.size, other.weight);
	}
};

class FontManagerImpl
{
public:
	FontManagerImpl() = default;
	~FontManagerImpl();

public:
	void Initialize(IDWriteFactory* factory);

	IDWriteTextFormat* CreateTextFormat(const wchar_t* fontName, float fontSize, DWRITE_FONT_WEIGHT weight);
	void Clear();

	IDWriteFactory* GetDWriteFactory();

private:
	std::map<FontKey, IDWriteTextFormat*> m_fontMap;
	IDWriteFactory* m_dxWriteFactory = nullptr;
};
