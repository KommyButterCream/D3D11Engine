#include "pch.h"
#include "FontManager.h"
#include "FontManagerImpl.h"

FontManager::FontManager()
	: m_impl(new FontManagerImpl())
{
}

FontManager::~FontManager()
{
	Shutdown();

	if (m_impl)
	{
		delete m_impl;
		m_impl = nullptr;
	}
}

bool FontManager::Initialize(IDWriteFactory* factory)
{
	if (!factory)
		return false;

	if (!m_impl)
		return false;

	m_impl->Initialize(factory);

	return true;
}

void FontManager::Shutdown()
{
	if (m_impl)
	{
		m_impl->Clear();
	}
}

IDWriteTextFormat* FontManager::GetTextFormat(const wchar_t* fontName, float fontSize, DWRITE_FONT_WEIGHT weight)
{
	if (!m_impl)
		return nullptr;

	return m_impl->CreateTextFormat(fontName, fontSize, weight);
}

IDWriteFactory* FontManager::GetDWriteFactory()
{
	if (!m_impl)
		return nullptr;

	return m_impl->GetDWriteFactory();
}
