#include "pch.h"
#include "D3D11RenderEngine.h"

#include "../Font/FontManager.h"

D3D11RenderEngine::D3D11RenderEngine()
{
}

D3D11RenderEngine::~D3D11RenderEngine()
{
	Shutdown();
}

bool D3D11RenderEngine::Initialize(const RenderEngineConfig& config)
{
	if (m_initialized)
		return true;

	m_config = config;

	HRESULT hr = CreateDeviceResources();
	if (FAILED(hr))
	{
		ReleaseDeviceResources();
		return false;
	}

	m_initialized = true;
	m_deviceAvailable = true;

	return true;
}

void D3D11RenderEngine::Shutdown()
{
	if (m_initialized)
	{
		ReleaseDeviceResources();
		m_initialized = false;
		m_deviceAvailable = false;
	}
}

HRESULT D3D11RenderEngine::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (m_config.initD3D)
	{
		// =========================================================
		// 1) DXGI Factory (DEBUG 지원)
		// =========================================================
		UINT factoryFlags = 0;

		if (m_config.initDebugLayer)
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		hr = CreateDXGIFactory2(
			factoryFlags,
			IID_PPV_ARGS(&m_dxgiFactory)
		);
		if (FAILED(hr))
			return hr;

		// =========================================================
		// 2) Adapter 선택
		// =========================================================
		IDXGIAdapter1* adapter = nullptr;
		hr = m_dxgiFactory->EnumAdapters1(0, &adapter);
		if (FAILED(hr))
			return hr;

		// =========================================================
		// 3) D3D11 Device / Context 생성
		// =========================================================
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		if (m_config.initDebugLayer)
			deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

		D3D_FEATURE_LEVEL levels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		D3D_FEATURE_LEVEL createdLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11Device* baseDevice = nullptr;
		ID3D11DeviceContext* baseContext = nullptr;

		hr = D3D11CreateDevice(
			adapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			deviceFlags,
			levels,
			ARRAYSIZE(levels),
			D3D11_SDK_VERSION,
			&baseDevice,
			&createdLevel,
			&baseContext
		);

		// 실패 시 WARP fallback
		if (FAILED(hr))
		{
			UINT warpFlags = deviceFlags & ~D3D11_CREATE_DEVICE_DEBUG;

			hr = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP,
				nullptr,
				warpFlags,
				levels,
				ARRAYSIZE(levels),
				D3D11_SDK_VERSION,
				&baseDevice,
				&createdLevel,
				&baseContext
			);
			if (FAILED(hr))
			{
				SafeRelease(adapter);
				return hr;
			}
		}

		SafeRelease(adapter);

		// =========================================================
		// 4) Device1 / Context1
		// =========================================================
		hr = baseDevice->QueryInterface(
			IID_PPV_ARGS(&m_device)
		);
		if (FAILED(hr))
		{
			SafeRelease(baseDevice);
			SafeRelease(baseContext);
			return hr;
		}

		hr = baseContext->QueryInterface(
			IID_PPV_ARGS(&m_deviceContext)
		);
		if (FAILED(hr))
		{
			SafeRelease(m_device);
			SafeRelease(baseDevice);
			SafeRelease(baseContext);
			return hr;
		}

		SafeRelease(baseDevice);
		SafeRelease(baseContext);
	}


	if (m_config.initD2D)
	{
		// =========================================================
	// 5) D2D Factory
	// =========================================================
		D2D1_FACTORY_OPTIONS d2dOpt = {};

		if (m_config.initDebugLayer)
			d2dOpt.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

		hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_MULTI_THREADED, // ⭐ 권장
			__uuidof(ID2D1Factory1),
			&d2dOpt,
			reinterpret_cast<void**>(&m_d2dFactory)
		);
		if (FAILED(hr))
			return hr;

		// =========================================================
		// 6) D2D Device
		// =========================================================
		IDXGIDevice2* dxgiDevice = nullptr;
		hr = m_device->QueryInterface(
			IID_PPV_ARGS(&dxgiDevice)
		);
		if (FAILED(hr))
			return hr;

		hr = m_d2dFactory->CreateDevice(
			dxgiDevice,
			&m_d2dDevice
		);

		SafeRelease(dxgiDevice);

		// =========================================================
		// 7) D2D Device
		// =========================================================
		hr = ::DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_writeFactory)
		);
	}

	if (m_config.initFontManager)
	{
		// =========================================================
		// 8) Text Manager
		// =========================================================
		m_fontManager = new FontManager;
		if (!m_fontManager)
		{
			return -1;
		}
		if (!m_fontManager->Initialize(GetWriteFactory()))
		{
			return -2;
		}
	}

	return hr;
}

void D3D11RenderEngine::ReleaseDeviceResources()
{
	if (m_fontManager)
	{
		delete m_fontManager;
		m_fontManager = nullptr;
	}

	if (m_deviceContext)
	{
		m_deviceContext->ClearState();
		m_deviceContext->Flush();
	}

	SafeRelease(m_writeFactory);
	SafeRelease(m_d2dDevice);
	SafeRelease(m_d2dFactory);

	SafeRelease(m_dxgiFactory);

	SafeRelease(m_deviceContext);
	SafeRelease(m_device);
}

bool D3D11RenderEngine::DiscardDevice()
{
	if (!m_initialized)
		return false;

	if (m_deviceAvailable)
		m_deviceAvailable = false;

	ReleaseDeviceResources();

	return true;
}

bool D3D11RenderEngine::RecreateDevice()
{
	if (!m_initialized)
		return false;

	HRESULT hr = CreateDeviceResources();
	if (FAILED(hr))
	{
		m_deviceAvailable = false;
		return false;
	}

	m_deviceAvailable = true;

	return true;
}
