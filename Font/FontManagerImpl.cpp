#include "pch.h"
#include "FontManagerImpl.h"

FontManagerImpl::~FontManagerImpl()
{
	Clear();
}

void FontManagerImpl::Initialize(IDWriteFactory* factory)
{
	m_dxWriteFactory = factory;
}

IDWriteTextFormat* FontManagerImpl::CreateTextFormat(const wchar_t* fontName, float fontSize, DWRITE_FONT_WEIGHT weight)
{
	if (!m_dxWriteFactory)
		return nullptr;

	// Build a stable cache key.
	FontKey key{ fontName, static_cast<int32_t>(fontSize * 100.0f), weight };

	// Reuse an existing text format when possible.
	auto it = m_fontMap.find(key);
	if (it != m_fontMap.end())
	{
		return it->second;
	}

	// Create and cache a new text format.
	IDWriteTextFormat* textFormat = nullptr;
	HRESULT hr = m_dxWriteFactory->CreateTextFormat(
		fontName, nullptr, weight,
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		fontSize, L"", &textFormat
	);

	if (SUCCEEDED(hr))
	{
		textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		m_fontMap[key] = textFormat;
		return textFormat;
	}

	return nullptr;
}

void FontManagerImpl::Clear()
{
	for (auto& font : m_fontMap)
	{
		if (font.second)
		{
			SafeRelease(&font.second);
		}
	}
	m_fontMap.clear();
}

IDWriteFactory* FontManagerImpl::GetDWriteFactory()
{
	return m_dxWriteFactory;
}
