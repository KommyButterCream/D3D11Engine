#pragma once

#ifdef GRAPHICS_ENGINE_DLL
#define RENDER_ENGINE_API __declspec(dllexport)
#else
#define RENDER_ENGINE_API __declspec(dllimport)
#endif

#include "../Modules/D3D11EngineInterface/IRenderEngine.h"

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGIFactory2;
struct ID2D1Factory1;
struct ID2D1Device;
struct IDWriteFactory;

class FontManager;

class RENDER_ENGINE_API D3D11RenderEngine : public IRenderEngine
{
public:
	D3D11RenderEngine();
	virtual ~D3D11RenderEngine();

	// IRenderEngine override
	virtual bool Initialize(const RenderEngineConfig& config) override;
	virtual void Shutdown() override;

	virtual bool IsInitialized() const override { return m_initialized; }
	virtual bool IsDeviceAvailable() const override { return m_deviceAvailable; }

	virtual bool DiscardDevice() override;
	virtual bool RecreateDevice() override;

	// Getter
	ID3D11Device1* GetD3DDevice() const { return m_device; }
	ID3D11DeviceContext1* GetD3DDeviceContext() const { return m_deviceContext; }
	IDXGIFactory2* GetDXGIFactory() const { return m_dxgiFactory; }
	ID2D1Factory1* GetD2DFactory() const { return m_d2dFactory; }
	ID2D1Device* GetD2DDevice() const { return m_d2dDevice; }
	IDWriteFactory* GetWriteFactory() const { return m_writeFactory; }
	FontManager* GetFontManager() const override { return m_fontManager; }

private:
	HRESULT CreateDeviceResources();
	void ReleaseDeviceResources();

private:
	RenderEngineConfig m_config = {};

	bool m_initialized = false;
	bool m_deviceAvailable = false;

	// D3D
	ID3D11Device1* m_device = nullptr;
	ID3D11DeviceContext1* m_deviceContext = nullptr;

	// DXGI
	IDXGIFactory2* m_dxgiFactory = nullptr;

	// D2D
	ID2D1Device* m_d2dDevice = nullptr;
	ID2D1Factory1* m_d2dFactory = nullptr;

	// DWrite
	IDWriteFactory* m_writeFactory = nullptr;

	// Font
	FontManager* m_fontManager = nullptr;
};
