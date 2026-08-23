#pragma once

#ifdef GRAPHICS_ENGINE_DLL
#define CAMERA_2D_API __declspec(dllexport)
#else
#define CAMERA_2D_API __declspec(dllimport)
#endif

#include "../util/SmoothValue.h"

struct alignas(16) ViewParams
{
	float scaleX = 0.f;
	float scaleY = 0.f;
	float offsetX = 0.f;
	float offsetY = 0.f;
};

struct ZoomAnchor
{
	bool active = false;
	float imageX = 0.0f;
	float imageY = 0.0f;
	float pivotX = 0.0f;
	float pivotY = 0.0f;
};

enum class PanState
{
	Idle,
	Dragging,
	Inertia
};

namespace Core::ShapeType
{
	struct Rect2i;
}

class CAMERA_2D_API Camera2D
{
public:
	Camera2D();

	void SetViewSize(uint32_t width, uint32_t height);
	void SetImageSize(uint32_t width, uint32_t height);
	void GetImageSize(uint32_t& width, uint32_t& height) const;

	void Reset();
	void Fit();
	void FitInstant();
	void FitWidth();
	void FitHeight();

	// pivot Screen 좌표
	void Zoom1to1();
	void Zoom1to1(float pivotX, float pivotY);
	void Zoom(float delta);
	void Zoom(float delta, float pivotX, float pivotY);


	void BeginPan(float mouseX, float mouseY);
	void UpdatePan(float mouseX, float mouseY, float dt);
	void EndPan();

	bool Update(float dt);

	// 줌/팬 애니메이션이 모두 멈췄는지.
	// 타일 스트리밍에서 "움직이는 중에는 새 타일을 만들지 않는다"는
	// 모션 게이팅 판정에 사용한다.
	bool IsSettled() const;

	void GetViewParams(ViewParams& out) const;

	bool ScreenToImage(float screenX, float screenY, float& outImageX, float& outImageY) const;
	bool ScreenToImagePixel(float screenX, float screenY, int32_t& outImageX, int32_t& outImageY) const;

	float GetZoom() const { return m_zoom.current; }
	float GetZoomPercent() const { return m_zoom.target * 100.0f; }
	float GetOffsetX() const { return m_offsetX.current; }
	float GetOffsetY() const { return m_offsetY.current; }

	Core::ShapeType::Rect2i GetViewImageRect() const;

private:
	uint32_t m_viewWidth = 1;
	uint32_t m_viewHeight = 1;

	uint32_t m_imageWidth = 1;
	uint32_t m_imageHeight = 1;

	// 절대 하한. 다만 이미지가 뷰포트보다 훨씬 크면 fit 배율이 이 값보다도
	// 작아지므로(예: 40000px 이미지를 2160px 높이에 맞추면 0.027), 실제 하한은
	// GetMinZoom() 에서 fit 배율까지 내려준다. 그러지 않으면 큰 이미지에서
	// 휠 축소가 fit 보다 앞에서 막혀 화면이 튄다.
	static constexpr float minZoomAbsolute = 0.05f;
	static constexpr float maxZoom = 100.0f;

	float GetFitZoom() const;
	float GetMinZoom() const;

	// Zoom
	ZoomAnchor m_zoomAnchor; // 마우스 위치로 Zoom 을 하기 위해 사용
	SmoothValue m_zoom; // 1.0 이면 이미지 1픽셀이 화면 1픽셀로 계산 및 출력

	// Zoom(Fit)
	bool m_fitActive = false;
	SmoothValue m_anchorImageX; // Image Pixel 기준 좌표, Fit 수행시 부드러운 Fit 을 위해 사용
	SmoothValue m_anchorImageY; // Image Pixel 기준 좌표, Fit 수행시 부드러운 Fit 을 위해 사용

	// Pan
	PanState m_panState = PanState::Idle;
	float m_prevMouseX = 0.0f;
	float m_prevMouseY = 0.0f;

	// Zoom & Pan
	SmoothValue m_offsetX; // Image Pixel 기준 좌표, 이미지가 출력되는 Offset
	SmoothValue m_offsetY; // Image Pixel 기준 좌표, 이미지가 출력되는 Offset
};

