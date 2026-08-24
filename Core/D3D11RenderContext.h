#pragma once

#ifdef BUILD_D3D11_ENGINE_DLL
#define RENDER_ENGINE_API __declspec(dllexport)
#else
#define RENDER_ENGINE_API __declspec(dllimport)
#endif

#include "../../../Module/D3D11EngineInterface/IRenderContext.h"

#include "../util/PtrArray.h"

struct IDXGISwapChain1;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct IDXGISurface;
struct ID2D1DeviceContext;
struct ID2D1DeviceContext5;
struct ID2D1Bitmap1;

class D3D11RenderEngine;
class IRenderLayer;
class IRenderEngine;
class IResizeEventListener;
class IDeviceEventListener;
class HighResTimer;

struct PendingResize
{
	volatile LONG width = 0;
	volatile LONG height = 0;
	volatile LONG requested = 0;
};

#include <vector>

class RENDER_ENGINE_API D3D11RenderContext : public IRenderContext
{
public:
	// 생성자는 Engine 포인터를 필요로 함 (Engine이 동일 DLL 내에 존재해야 안전)
	D3D11RenderContext(D3D11RenderEngine* engine);
	virtual ~D3D11RenderContext();

	bool Initialize(void* windowHandle) override;
	void Shutdown() override;

	bool IsInitialized() const override { return m_initialized; }

	void RequestResize(uint32_t newWidth, uint32_t newHeight) override;
	bool Resize(uint32_t newWidth, uint32_t newHeight) override;

	uint32_t GetWidth() const override { return m_width; }
	uint32_t GetHeight() const override { return m_height; }

	bool BeginFrame() override;
	bool EndFrame() override;
	bool BeginOverlay() override;
	bool EndOverlay() override;

	float GetDeltaTime() const override;

	bool RecreateResources() override;

	IRenderEngine* GetEngine() override;

	ID2D1DeviceContext* GetD2DDeviceContext() override;
	ID2D1DeviceContext5* GetD2DDeviceContext5() override;
	IDWriteFactory* GetWriteFactory() override;

	// IDeviceEventListener override
	void AddDeviceListener(IDeviceEventListener* listener) override;
	void RemoveDeviceListener(IDeviceEventListener* listener) override;

	// IResizeEventListener override
	void AddResizeListener(IResizeEventListener* listener) override;
	void RemoveResizeListener(IResizeEventListener* listener) override;

public:
	void ProcessPendingResize();


public:
	HWND GetHWND() const { return m_hwnd; }

	ID3D11RenderTargetView* GetD3DRenderTargetView() const { return m_rtv; }

	// helpers
	bool IsEngineDeviceReady() const;
	bool IsSwapChainReady() const;

	// Listener
	bool CreateDeviceListener();
	void DestroyDeviceListener();
	void NotifyDeviceLost();
	void NotifyDeviceRestored();

	bool CreateResizeListener();
	void DestroyResizeListener();
	void NotifyResize(uint32_t width, uint32_t height);

	void Tick();
	float GetFPS() const;

	// 수직 동기. 기본 켜짐.
	//
	// 켜면 Present 가 vblank 에 정렬되어 애니메이션 간격이 균일해지지만
	// 표시가 최대 1 refresh 늦는다. 라이브 카메라처럼 지연이 중요하면
	// 끌 수 있다. 단 flip 모델에서 끈다는 것은 티어링이 아니라 프레임
	// 폐기를 의미하므로, 끌 경우 RenderThread::SetRenderFPS 로 프레임 수를
	// 화면 주사율 근처가 아닌 값으로 직접 제한해야 한다.
	void SetVSyncEnabled(bool enable) { m_vsyncEnabled = enable; }
	bool IsVSyncEnabled() const { return m_vsyncEnabled; }

private:
	// 내부 리소스 생성/해제
	HRESULT CreateBackBufferResources();
	void    ReleaseBackBufferResources();

	void    ReleaseAllResources();

public:
	// 테스트용 디바이스 로스트 유발. 실제 로스트 경로를 그대로 실행한다.
	bool SimulateDeviceLost();

private:
	bool HandleDeviceLost(HRESULT hr);

	// 실질적 SwapChain 생성
	HRESULT CreateSwapChain(uint32_t width, uint32_t height);

	bool ConsumeResize(LONG& outWidth, LONG& outHeight);



private:
	// Engine (소유권 없음)
	D3D11RenderEngine* m_engine = nullptr;

	// Window / size
	HWND    m_hwnd = nullptr;
	uint32_t    m_width = 0;
	uint32_t    m_height = 0;

	// Swap Chain & Backbuffer
	IDXGISwapChain1* m_swapChain = nullptr;
	ID3D11Texture2D* m_backBuffer = nullptr;
	ID3D11RenderTargetView* m_rtv = nullptr;

	PendingResize m_pendingResize = {};

	// DXGI Surface D2D Interop
	IDXGISurface* m_dxgiBackBufferSurface = nullptr;

	// D2D target
	ID2D1DeviceContext* m_d2dContext = nullptr;
	ID2D1DeviceContext5* m_d2dContext5 = nullptr;
	ID2D1Bitmap1* m_d2dTargetBitmap = nullptr;

	// Listener
	PtrArray<IDeviceEventListener>* m_deviceListeners = nullptr;
	PtrArray<IResizeEventListener>* m_resizeListeners = nullptr;

	HighResTimer* m_timer;


	// flags
	bool    m_initialized = false;
	bool    m_resourcesCreated = false;

	// Present 의 syncInterval 을 결정한다. SetVSyncEnabled 참고.
	bool m_vsyncEnabled = true;

	bool m_inResize = false;
	bool m_d2dDrawing = false;
	bool m_inOverlay = false;
};
