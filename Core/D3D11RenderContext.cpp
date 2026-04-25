#include "pch.h"
#include "D3D11RenderContext.h"
#include "D3D11RenderEngine.h" // 사용되는 engine의 public API 접근

#include "HighResTimer.h"

#include "../Modules/D3D11EngineInterface/IDeviceEventListener.h"
#include "../Modules/D3D11EngineInterface/IResizeEventListener.h"

D3D11RenderContext::D3D11RenderContext(D3D11RenderEngine* engine)
	: m_engine(engine)
	, m_timer(new HighResTimer)
{
}

D3D11RenderContext::~D3D11RenderContext()
{
	Shutdown();
}

bool D3D11RenderContext::Initialize(void* windowHandle)
{
	if (!m_engine)
		return false;

	if (m_initialized)
		return false;

	HWND hWnd = reinterpret_cast<HWND>(windowHandle);
	if (!IsWindow(hWnd))
		return false;

	m_hwnd = hWnd;

	RECT rc;
	::GetClientRect(hWnd, &rc);
	m_width = rc.right - rc.left;
	m_height = rc.bottom - rc.top;
	if (m_width == 0) m_width = 1;
	if (m_height == 0) m_height = 1;

	if (!IsEngineDeviceReady())
		return false; // engine must be initialized and device available


	if (FAILED(CreateSwapChain(m_width, m_height)))
		return false;


	if (FAILED(CreateBackBufferResources()))
	{
		return false;
	}

	if (!CreateDeviceListener())
		return false;

	if (!CreateResizeListener())
		return false;

	m_initialized = true;

	return true;
}

void D3D11RenderContext::Shutdown()
{
	DestroyDeviceListener();
	DestroyResizeListener();

	ReleaseAllResources();

	if (m_timer)
	{
		delete m_timer;
		m_timer = nullptr;
	}

	m_hwnd = nullptr;
	m_width = m_height = 0;
	m_initialized = false;
	m_resourcesCreated = false;
}

bool D3D11RenderContext::IsEngineDeviceReady() const
{
	if (!m_engine) return false;

	return m_engine->IsInitialized()
		&& m_engine->IsDeviceAvailable()
		&& m_engine->GetD3DDevice()
		&& m_engine->GetD2DDevice()
		&& m_engine->GetDXGIFactory()
		&& m_engine->GetWriteFactory();
}

bool D3D11RenderContext::IsSwapChainReady() const
{
	return m_swapChain && m_rtv && m_resourcesCreated;
}

bool D3D11RenderContext::CreateDeviceListener()
{
	m_deviceListeners = new PtrArray<IDeviceEventListener>();
	if (!m_deviceListeners)
		return false;

	m_deviceListeners->Init();

	return true;
}

void D3D11RenderContext::DestroyDeviceListener()
{
	if (m_deviceListeners)
	{
		m_deviceListeners->Clear();
		delete m_deviceListeners;
		m_deviceListeners = nullptr;
	}
}

void D3D11RenderContext::AddDeviceListener(IDeviceEventListener* listener)
{
	if (!m_deviceListeners)
	{
		CreateDeviceListener();
	}

	for (uint32_t i = 0; i < m_deviceListeners->size; ++i)
	{
		if (m_deviceListeners->data[i] == listener)
			return;
	}

	m_deviceListeners->Push(listener);
}

void D3D11RenderContext::RemoveDeviceListener(IDeviceEventListener* listener)
{
	if (!m_deviceListeners || !listener)
		return;

	for (uint32_t i = 0; i < m_deviceListeners->size; ++i)
	{
		if (m_deviceListeners->data[i] == listener)
		{
			m_deviceListeners->data[i] =
				m_deviceListeners->data[m_deviceListeners->size - 1];
			--m_deviceListeners->size;
			return;
		}
	}
}

void D3D11RenderContext::NotifyDeviceLost()
{
	if (!m_deviceListeners)
		return;

	for (uint32_t i = 0; i < m_deviceListeners->size; i++)
	{
		m_deviceListeners->data[i]->OnDeviceLost();
	}
}

void D3D11RenderContext::NotifyDeviceRestored()
{
	if (!m_deviceListeners)
		return;

	for (uint32_t i = 0; i < m_deviceListeners->size; ++i)
	{
		m_deviceListeners->data[i]->OnDeviceRestored();
	}
}

bool D3D11RenderContext::CreateResizeListener()
{
	m_resizeListeners = new PtrArray<IResizeEventListener>();
	if (!m_resizeListeners)
		return false;

	m_resizeListeners->Init();

	return true;
}

void D3D11RenderContext::DestroyResizeListener()
{
	if (m_resizeListeners)
	{
		m_resizeListeners->Clear();
		delete m_resizeListeners;
		m_resizeListeners = nullptr;
	}
}

void D3D11RenderContext::AddResizeListener(IResizeEventListener* listener)
{
	if (!m_resizeListeners)
	{
		CreateResizeListener();
	}

	for (uint32_t i = 0; i < m_resizeListeners->size; ++i)
	{
		if (m_resizeListeners->data[i] == listener)
			return;
	}

	m_resizeListeners->Push(listener);
}

void D3D11RenderContext::RemoveResizeListener(IResizeEventListener* listener)
{
	if (!m_resizeListeners || !listener)
		return;

	for (uint32_t i = 0; i < m_resizeListeners->size; ++i)
	{
		if (m_resizeListeners->data[i] == listener)
		{
			m_resizeListeners->data[i] =
				m_resizeListeners->data[m_resizeListeners->size - 1];
			--m_resizeListeners->size;
			return;
		}
	}
}

void D3D11RenderContext::NotifyResize(uint32_t width, uint32_t height)
{
	if (!m_resizeListeners)
		return;

	for (uint32_t i = 0; i < m_resizeListeners->size; ++i)
	{
		m_resizeListeners->data[i]->OnResize(width, height);
	}
}

void D3D11RenderContext::Tick()
{
	m_timer->Tick();
}

float D3D11RenderContext::GetFPS() const
{
	return m_timer->GetFPS();
}

HRESULT D3D11RenderContext::CreateSwapChain(uint32_t width, uint32_t height)
{
	if (!m_engine) return E_POINTER;

	ID3D11Device1* device = m_engine->GetD3DDevice();
	IDXGIFactory2* factory = m_engine->GetDXGIFactory();
	if (!device || !factory) return E_FAIL;

	DXGI_SWAP_CHAIN_DESC1 sd = {};
	sd.Width = 0;   // HWND client size 사용
	sd.Height = 0;
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.Stereo = FALSE;
	sd.SampleDesc.Count = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	sd.BufferCount = 2;
	sd.Scaling = DXGI_SCALING_STRETCH;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	sd.Flags = 0;

	IDXGISwapChain1* swapChain = nullptr;
	HRESULT hr = factory->CreateSwapChainForHwnd(
		device,
		m_hwnd,
		&sd,
		nullptr,
		nullptr,
		&swapChain
	);

	DWORD error = ::GetLastError();
	if (FAILED(hr))
		return hr;

	factory->MakeWindowAssociation(
		m_hwnd,
		DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES
	);

	if (m_swapChain)
		SafeRelease(&m_swapChain);

	m_swapChain = swapChain;
	return S_OK;
}

HRESULT D3D11RenderContext::CreateBackBufferResources()
{
	if (!IsEngineDeviceReady())
		return E_FAIL;

	if (!m_swapChain)
		return E_FAIL;

	HRESULT hr = S_OK;

	// 1혹시 남아있는 backbuffer 리소스가 있다면 먼저 정리
	if (m_resourcesCreated)
		ReleaseBackBufferResources();   // ❗ swapchain은 건드리지 않음

	ID3D11Device1* device1 = m_engine->GetD3DDevice();
	if (!device1)
		return E_FAIL;

	// backbuffer texture
	hr = m_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&m_backBuffer)
	);
	if (FAILED(hr))
	{
		ReleaseBackBufferResources();
		return E_FAIL;
	}

	// RTV (D3D path용 – optional)
	hr = device1->CreateRenderTargetView(
		m_backBuffer,
		nullptr,
		&m_rtv
	);

	if (FAILED(hr))
	{
		// D2D only path면 fatal 아님
		SafeRelease(&m_rtv);
		return hr;
	}

	// DXGI surface (D2D interop)
	hr = m_backBuffer->QueryInterface(
		__uuidof(IDXGISurface),
		reinterpret_cast<void**>(&m_dxgiBackBufferSurface)
	);
	if (FAILED(hr))
	{
		ReleaseBackBufferResources();
		return hr;
	}

	// D2D device context
	ID2D1Device* d2dDevice = m_engine->GetD2DDevice();
	if (!d2dDevice)
	{
		ReleaseBackBufferResources();
		return hr;
	}

	if (!m_d2dContext)
	{
		hr = d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
			&m_d2dContext
		);

		if (FAILED(hr))
		{
			ReleaseBackBufferResources();
			return hr;
		}

		hr = m_d2dContext->QueryInterface(__uuidof(ID2D1DeviceContext5), reinterpret_cast<void**>(&m_d2dContext5));

		if (FAILED(hr))
		{
			ReleaseBackBufferResources();
			return hr;
		}
	}

	if (m_d2dContext)
	{
		m_d2dContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
	}


	// D2D bitmap target
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 0.f, 0.f);

	hr = m_d2dContext->CreateBitmapFromDxgiSurface(
		m_dxgiBackBufferSurface,
		&bitmapProperties,
		&m_d2dTargetBitmap
	);

	if (FAILED(hr))
	{
		ReleaseBackBufferResources();
		return hr;
	}

	// bind target
	m_d2dContext->SetTarget(m_d2dTargetBitmap);

	m_resourcesCreated = true;

	return S_OK;
}

void D3D11RenderContext::ReleaseBackBufferResources()
{
	// D3D - Unbind OM targets
	if (m_engine)
	{
		if (ID3D11DeviceContext1* context = m_engine->GetD3DDeviceContext())
		{
			context->OMSetRenderTargets(0, nullptr, nullptr);
		}
	}

	// D2D Set target null before releasing
	if (m_d2dContext && m_d2dDrawing)
	{
		m_d2dContext->EndDraw();
		m_d2dDrawing = false;
		m_inOverlay = false;
	}

	if (m_d2dContext)
	{
		m_d2dContext->SetTarget(nullptr);
	}

	// D2D
	SafeRelease(&m_d2dTargetBitmap);
	SafeRelease(&m_dxgiBackBufferSurface);

	// D3D
	SafeRelease(&m_rtv);
	SafeRelease(&m_backBuffer);

	m_resourcesCreated = false;
}

void D3D11RenderContext::ReleaseAllResources()
{
	ReleaseBackBufferResources();
	SafeRelease(&m_swapChain);
	SafeRelease(&m_d2dContext);
	SafeRelease(&m_d2dContext5);

	//m_d2dContext5 = nullptr;

	if (m_engine)
	{
		if (auto* ctx = m_engine->GetD3DDeviceContext())
		{
			ctx->ClearState();
			ctx->Flush();
		}
	}
}

bool D3D11RenderContext::HandleDeviceLost(HRESULT hr)
{
	if (hr != DXGI_ERROR_DEVICE_REMOVED &&
		hr != DXGI_ERROR_DEVICE_RESET)
		return false;

	// 모든 리스너에게 알림을 먼저 전달
	NotifyDeviceLost();

	// 1. 모든 backbuffer / context 리소스 해제
	ReleaseBackBufferResources();

	// 2. swapchain, d2d context 해제
	SafeRelease(&m_swapChain);
	SafeRelease(&m_d2dContext);
	SafeRelease(&m_d2dContext5);
	//m_d2dContext5 = nullptr;

	// 3. engine device 재생성
	if (m_engine)
	{
		m_engine->DiscardDevice();

		if (!m_engine->RecreateDevice())
			return false;
	}

	// 4. swapchain 재생성
	if (FAILED(CreateSwapChain(m_width, m_height)))
		return false;

	// 5. backbuffer 리소스 재생성
	if (FAILED(CreateBackBufferResources()))
		return false;

	// 6. 리스너에게 복구 완료 알림
	NotifyDeviceRestored();

	return true;
}

void D3D11RenderContext::RequestResize(uint32_t newWidth, uint32_t newHeight)
{
	::InterlockedExchange(&m_pendingResize.width, newWidth);
	::InterlockedExchange(&m_pendingResize.height, newHeight);
	::InterlockedExchange(&m_pendingResize.requested, 1);
}

bool D3D11RenderContext::Resize(uint32_t newWidth, uint32_t newHeight)
{
	if (!m_initialized || !IsEngineDeviceReady())
		return false;

	if (newWidth == 0) newWidth = 1;
	if (newHeight == 0) newHeight = 1;

	if (m_width == newWidth && m_height == newHeight)
		return true;

	m_width = newWidth;
	m_height = newHeight;

	m_inResize = true;

	ReleaseBackBufferResources();

	if (!m_swapChain)
		return false;

	// ResizeBuffers (must release references to backbuffer)
	HRESULT hr = m_swapChain->ResizeBuffers(0, m_width, m_height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
	{
		m_inResize = false;

		return HandleDeviceLost(hr);
	}

	m_inResize = false;

	// Recreate backbuffer / d2d target
	if (FAILED(CreateBackBufferResources()))
		return false;

	NotifyResize(newWidth, newHeight);

	return true;
}

bool D3D11RenderContext::BeginFrame()
{
	if (!m_initialized)
		return false;

	if (m_inResize)
		return false;

	ProcessPendingResize();

	if (!IsSwapChainReady())
	{
		return false;
	}

	ID3D11DeviceContext* context = m_engine->GetD3DDeviceContext();
	if (!context)
		return false;

	if (!m_rtv)
		return false;

	// OM
	context->OMSetRenderTargets(1, &m_rtv, nullptr);

	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<FLOAT>(m_width);
	vp.Height = static_cast<FLOAT>(m_height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0;

	context->RSSetViewports(1, &vp);

	// Clear
	float backgroundColor[4] = { 0.f, 0.f, 0.f, 0.f };
	context->ClearRenderTargetView(m_rtv, backgroundColor);

	return true;
}

bool D3D11RenderContext::EndFrame()
{
	if (!m_swapChain)
		return false;

	//uint32_t syncInterval = 0;
	//uint32_t flags = DXGI_PRESENT_DO_NOT_WAIT;

	uint32_t syncInterval = 0;
	//uint32_t flags = 0;

	//uint32_t syncInterval = 1;
	uint32_t flags = 0;

	HRESULT hr = m_swapChain->Present(syncInterval, flags);
	if (hr == DXGI_STATUS_OCCLUDED || hr == DXGI_ERROR_WAS_STILL_DRAWING)
	{
		return true;
	}
	else if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		// Device lost
		return HandleDeviceLost(hr);
	}

	return SUCCEEDED(hr);
}

bool D3D11RenderContext::BeginOverlay()
{
	if (!m_initialized)
		return false;

	if (!m_d2dContext || !m_d2dTargetBitmap)
		return false;

	if (m_inOverlay)
		return false;

	if (m_d2dDrawing)
		return false;

	m_inOverlay = true;

	// D3D OM unbind
	if (ID3D11DeviceContext1* context = m_engine->GetD3DDeviceContext())
	{
		context->OMSetRenderTargets(0, nullptr, nullptr);
	}
	m_d2dContext->SetTarget(m_d2dTargetBitmap);

	m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

	m_d2dContext->BeginDraw();

	m_d2dDrawing = true;

	return true;
}

bool D3D11RenderContext::EndOverlay()
{
	if (!m_d2dContext || !m_d2dDrawing)
		return true;

	HRESULT hr = m_d2dContext->EndDraw();
	m_d2dDrawing = false;
	m_inOverlay = false;

	if (hr == D2DERR_RECREATE_TARGET)
	{
		NotifyDeviceLost();
		ReleaseBackBufferResources();

		if (FAILED(CreateBackBufferResources()))
			return false;

		NotifyDeviceRestored();
		return true;
	}

	if (FAILED(hr))
	{
		return HandleDeviceLost(hr);
	}

	return true;
}

float D3D11RenderContext::GetDeltaTime() const
{
	return m_timer->GetDeltaSeconds();
}

bool D3D11RenderContext::RecreateResources()
{
	if (!m_initialized)
		return false;

	if (m_resourcesCreated)
		return true;

	if (!IsEngineDeviceReady())
		return false;

	return SUCCEEDED(CreateBackBufferResources());
}

IRenderEngine* D3D11RenderContext::GetEngine()
{
	return m_engine;
}

ID2D1DeviceContext* D3D11RenderContext::GetD2DDeviceContext()
{
	return m_d2dContext;
}

ID2D1DeviceContext5* D3D11RenderContext::GetD2DDeviceContext5()
{
	return m_d2dContext5;
}

IDWriteFactory* D3D11RenderContext::GetWriteFactory()
{
	if (!m_engine)
		return nullptr;

	return m_engine->GetWriteFactory();
}

void D3D11RenderContext::ProcessPendingResize()
{
	LONG width = 0, height = 0;

	if (!ConsumeResize(width, height))
		return;

	if (width == 0 || height == 0)
	{
		return;
	}

	Resize(width, height);
}

bool D3D11RenderContext::ConsumeResize(LONG& outWidth, LONG& outHeight)
{
	// 요청 없으면 바로 return
	if (InterlockedCompareExchange(
		&m_pendingResize.requested, 0, 1) != 1)
		return false;

	// 요청을 가져간 시점의 마지막 값
	outWidth = InterlockedExchange(&m_pendingResize.width, 0);
	outHeight = InterlockedExchange(&m_pendingResize.height, 0);

	return true;
}
