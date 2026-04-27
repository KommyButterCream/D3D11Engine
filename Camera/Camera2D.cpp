#include "pch.h"
#include "Camera2D.h"

#include <math.h>
#include "../../../Module/Core/ShapeType/Rect2i.h"

#include <algorithm>

using std::max;
using std::min;

Camera2D::Camera2D()
{
	Reset();
}

/*---------------------------------------------------------
	Size
---------------------------------------------------------*/
void Camera2D::SetViewSize(uint32_t width, uint32_t height)
{
	m_viewWidth = max(1u, width);
	m_viewHeight = max(1u, height);
}

void Camera2D::SetImageSize(uint32_t width, uint32_t height)
{
	m_imageWidth = max(1u, width);
	m_imageHeight = max(1u, height);
}

void Camera2D::GetImageSize(uint32_t& width, uint32_t& height) const
{
	width = m_imageWidth;
	height = m_imageHeight;
}

/*---------------------------------------------------------
	Reset / Fit
---------------------------------------------------------*/
void Camera2D::Reset()
{
	m_zoom.Snap(1.0f);
	m_offsetX.Snap(0.0f);
	m_offsetY.Snap(0.0f);

	m_zoomAnchor.active = false;
}

void Camera2D::Fit()
{
	// 목표 배율 계산
	const float sx = (float)m_viewWidth / (float)m_imageWidth;
	const float sy = (float)m_viewHeight / (float)m_imageHeight;
	const float zoomScale = min(sx, sy);

	// 현재 화면의 정중앙(pivot)이 가리키고 있는 "이미지 상의 좌표"를 역계산
	// 공식: 현재_이미지_중앙 = 현재_오프셋 + (화면_반폭 / 현재_줌)
	const float currentImageCenterX = m_offsetX.current + (m_viewWidth * 0.5f) / m_zoom.current;
	const float currentImageCenterY = m_offsetY.current + (m_viewHeight * 0.5f) / m_zoom.current;

	// 앵커 이미지 좌표를 현재 중앙 지점으로 즉시 이동(Snap)
	m_anchorImageX.Snap(currentImageCenterX);
	m_anchorImageY.Snap(currentImageCenterY);

	// 앵커의 목적지를 "이미지의 정중앙"으로 설정
	m_anchorImageX.target = m_imageWidth * 0.5f;
	m_anchorImageY.target = m_imageHeight * 0.5f;

	// 화면상의 고정점(Pivot)은 화면 정중앙으로 설정
	m_zoomAnchor.pivotX = m_viewWidth * 0.5f;
	m_zoomAnchor.pivotY = m_viewHeight * 0.5f;

	// 줌 목표 설정 및 플래그 활성화
	m_zoom.target = zoomScale;
	m_zoomAnchor.active = true;
	m_fitActive = true; // 이동하는 앵커 로직 사용
}

void Camera2D::FitInstant()
{
	const float sx = (float)m_viewWidth / (float)m_imageWidth;
	const float sy = (float)m_viewHeight / (float)m_imageHeight;

	const float zoomScale = min(sx, sy);
	m_zoom.Snap(zoomScale);

	m_offsetX.Snap((m_imageWidth - m_viewWidth / zoomScale) * 0.5f);
	m_offsetY.Snap((m_imageHeight - m_viewHeight / zoomScale) * 0.5f);

	m_zoomAnchor.active = false;
}

void Camera2D::FitWidth()
{
	// 목표 배율 계산 (가로폭에 맞춤)
	const float zoomScale = (float)m_viewWidth / (float)m_imageWidth;

	// 현재 화면 중앙이 가리키는 이미지 좌표 역계산 (Fit과 동일)
	const float currentImageCenterX = m_offsetX.current + (m_viewWidth * 0.5f) / m_zoom.current;
	const float currentImageCenterY = m_offsetY.current + (m_viewHeight * 0.5f) / m_zoom.current;

	// 앵커 이미지 좌표 설정
	m_anchorImageX.Snap(currentImageCenterX);
	m_anchorImageY.Snap(currentImageCenterY);

	// 중앙 정렬
	m_anchorImageX.target = m_imageWidth * 0.5f;
	m_anchorImageY.target = m_imageHeight * 0.5f;

	// 화면상의 고정점은 화면 정중앙
	m_zoomAnchor.pivotX = m_viewWidth * 0.5f;
	m_zoomAnchor.pivotY = m_viewHeight * 0.5f;

	// 상태 활성화
	m_zoom.target = zoomScale;
	m_zoomAnchor.active = true;
	m_fitActive = true;
}

void Camera2D::FitHeight()
{
	// 목표 배율 계산 (세로 높이에 맞춤)
	const float zoomScale = (float)m_viewHeight / (float)m_imageHeight;

	// 현재 화면 중앙이 가리키는 이미지 좌표 역계산
	const float currentImageCenterX = m_offsetX.current + (m_viewWidth * 0.5f) / m_zoom.current;
	const float currentImageCenterY = m_offsetY.current + (m_viewHeight * 0.5f) / m_zoom.current;

	// 앵커 이미지 좌표 설정
	m_anchorImageX.Snap(currentImageCenterX);
	m_anchorImageY.Snap(currentImageCenterY);

	// 중앙 정렬
	m_anchorImageX.target = m_imageWidth * 0.5f;
	m_anchorImageY.target = m_imageHeight * 0.5f;

	// 화면상의 고정점은 화면 정중앙
	m_zoomAnchor.pivotX = m_viewWidth * 0.5f;
	m_zoomAnchor.pivotY = m_viewHeight * 0.5f;

	// 상태 활성화
	m_zoom.target = zoomScale;
	m_zoomAnchor.active = true;
	m_fitActive = true;
}

/*---------------------------------------------------------
	Zoom
---------------------------------------------------------*/
void Camera2D::Zoom1to1()
{
	if (fabs(m_zoom.target - 1.0f) < 1e-4f)
		return;

	// anchor가 없다면 "현재 화면 중심" 기준으로 anchor 생성
	if (!m_zoomAnchor.active)
	{
		const float cx = m_viewWidth * 0.5f;
		const float cy = m_viewHeight * 0.5f;

		const float zoomScale = m_zoom.current;

		m_zoomAnchor.imageX = m_offsetX.current + cx / zoomScale;
		m_zoomAnchor.imageY = m_offsetY.current + cy / zoomScale;
		m_zoomAnchor.pivotX = cx;
		m_zoomAnchor.pivotY = cy;
		m_zoomAnchor.active = true;
	}

	// anchor 유지한 채 정확히 1:1로 이동
	m_zoom.target = 1.0f;
}

void Camera2D::Zoom1to1(float pivotX, float pivotY)
{
	Zoom(1.0f / m_zoom.target, pivotX, pivotY);
}

void Camera2D::Zoom(float delta)
{
	if (!m_zoomAnchor.active)
	{
		const float cx = m_viewWidth * 0.5f;
		const float cy = m_viewHeight * 0.5f;

		Zoom(delta, cx, cy);
		return;
	}

	m_zoom.target = std::clamp(
		m_zoom.target * delta,
		minZoom,
		maxZoom
	);
}

void Camera2D::Zoom(float delta, float pivotX, float pivotY)
{
	// 항상 "현재 화면에 보이는 상태" 기준
	m_fitActive = false;

	const float zoomScale = m_zoom.current;

	// 1?? pivot → image space (무조건 current 기준)
	m_zoomAnchor.imageX = m_offsetX.current + pivotX / zoomScale;
	m_zoomAnchor.imageY = m_offsetY.current + pivotY / zoomScale;
	m_zoomAnchor.pivotX = pivotX;
	m_zoomAnchor.pivotY = pivotY;

	// 2?? anchor는 항상 활성
	m_zoomAnchor.active = true;

	m_zoom.target = std::clamp(
		m_zoom.target * delta,
		minZoom,
		maxZoom
	);

	m_offsetX.target = m_offsetX.current;
	m_offsetY.target = m_offsetY.current;
}

/*---------------------------------------------------------
	Pan
---------------------------------------------------------*/
void Camera2D::BeginPan(float mouseX, float mouseY)
{
	m_panState = PanState::Dragging;
	m_zoomAnchor.active = false;
	m_prevMouseX = mouseX;
	m_prevMouseY = mouseY;
}

void Camera2D::UpdatePan(float mouseX, float mouseY, float dt)
{
	if (m_panState != PanState::Dragging)
		return;

	const float dx = mouseX - m_prevMouseX;
	const float dy = mouseY - m_prevMouseY;

	m_prevMouseX = mouseX;
	m_prevMouseY = mouseY;

	m_offsetX.current -= dx / m_zoom.current;
	m_offsetY.current -= dy / m_zoom.current;

	m_offsetX.target = m_offsetX.current;
	m_offsetY.target = m_offsetY.current;
}

void Camera2D::EndPan()
{
	if (m_panState != PanState::Dragging)
		return;

	m_panState = PanState::Idle;
}

/*---------------------------------------------------------
	Update (per-frame)
---------------------------------------------------------*/
bool Camera2D::Update(float dt)
{
	constexpr float interpolationSpeed = 14.0f;

	// Zoom 보간
	m_zoom.Update(dt, interpolationSpeed);

	// Anchor 기반 업데이트 (Zoom Fit 또는 일반 Zoom)
	if (m_zoomAnchor.active)
	{
		// 앵커 좌표 결정
		if (m_fitActive)
		{
			m_anchorImageX.Update(dt, interpolationSpeed);
			m_anchorImageY.Update(dt, interpolationSpeed);
		}

		float currentAnchorX = m_fitActive ? m_anchorImageX.current : m_zoomAnchor.imageX;
		float currentAnchorY = m_fitActive ? m_anchorImageY.current : m_zoomAnchor.imageY;

		// 3Offset 실시간 계산 및 타겟 동기화
		m_offsetX.current = currentAnchorX - (m_zoomAnchor.pivotX / m_zoom.current);
		m_offsetY.current = currentAnchorY - (m_zoomAnchor.pivotY / m_zoom.current);

		m_offsetX.target = m_offsetX.current;
		m_offsetY.target = m_offsetY.current;

		// 모든 보간이 완료되었는지 확인
		const bool zoomDone = m_zoom.IsAtTarget();
		const bool anchorDone = !m_fitActive || (m_anchorImageX.IsAtTarget() && m_anchorImageY.IsAtTarget());

		if (zoomDone && anchorDone)
		{
			// 부동소수점 오차 방지를 위한 최종 값 스냅
			m_zoom.current = m_zoom.target;
			if (m_fitActive)
			{
				m_anchorImageX.current = m_anchorImageX.target;
				m_anchorImageY.current = m_anchorImageY.target;
			}

			m_zoomAnchor.active = false;
			m_fitActive = false;

			return false; // 더 이상 애니메이션 중이 아님 (animating = false)
		}

		return true;
	}
	else
	{
		// 일반 Pan 모드
		m_offsetX.Update(dt, interpolationSpeed);
		m_offsetY.Update(dt, interpolationSpeed);

		const bool animating =
			!m_zoom.IsAtTarget() ||
			!m_offsetX.IsAtTarget() ||
			!m_offsetY.IsAtTarget();

		return animating;
	}
}

/*---------------------------------------------------------
	View Params (for VS constant buffer)
---------------------------------------------------------*/
void Camera2D::GetViewParams(ViewParams& out) const
{
	//// image center (image space)
	//const float imageCenterX = m_imageWidth * 0.5f;
	//const float imageCenterY = m_imageHeight * 0.5f;

	//// image center → screen
	//const float screenCenterX = (imageCenterX - m_offsetX.current) * m_zoom.current;
	//const float screenCenterY = (imageCenterY - m_offsetY.current) * m_zoom.current;

	//// screen → NDC
	//out.scaleX = (m_imageWidth * m_zoom.current) / m_viewWidth;
	//out.scaleY = (m_imageHeight * m_zoom.current) / m_viewHeight;

	//out.offsetX = 2.0f * screenCenterX / m_viewWidth - 1.0f;
	//out.offsetY = -2.0f * screenCenterY / m_viewHeight + 1.0f;


	// ---- 1차
	// pixel → NDC scale
	//out.scaleX = 2.0f * m_zoom.current / m_viewWidth;
	//out.scaleY = -2.0f * m_zoom.current / m_viewHeight;

	//// image fit 기준 화면 중앙 정렬
	//const float drawW = m_imageWidth * m_zoom.current;
	//const float drawH = m_imageHeight * m_zoom.current;

	//const float screenOffsetX = (m_viewWidth - drawW) * 0.5f;
	//const float screenOffsetY = (m_viewHeight - drawH) * 0.5f;

	//out.offsetX = -1.0f + 2.0f * screenOffsetX / m_viewWidth;
	//out.offsetY = 1.0f - 2.0f * screenOffsetY / m_viewHeight;


	// ---- 2차
	// 1. Scale: 픽셀 좌표를 NDC 크기로 변환
	// 윈도우 좌표계는 아래로 갈수록 +Y이지만, 
	// D3D NDC는 위로 갈수록 +Y이므로 Y축에 마이너스를 붙입니다.
	out.scaleX = (2.0f * m_zoom.current) / (float)m_viewWidth;
	out.scaleY = -(2.0f * m_zoom.current) / (float)m_viewHeight;

	// 2. Offset: 카메라의 Pan(m_offsetX)을 반영하여 좌상단(-1, 1) 기준으로 이동
	// 공식: NDC_Pos = (Pixel_Pos - Camera_Offset) * Scale - 1.0
	// 이를 쉐이더의 p * scale + offset 형태로 분해하면:
	out.offsetX = -1.0f - (m_offsetX.current * out.scaleX);
	out.offsetY = 1.0f - (m_offsetY.current * out.scaleY);
}

bool Camera2D::ScreenToImage(float screenX, float screenY, float& outImageX, float& outImageY) const
{
	if (m_zoom.current <= 0.0f)
		return false;

	outImageX = m_offsetX.current + screenX / m_zoom.current;
	outImageY = m_offsetY.current + screenY / m_zoom.current;

	// 이미지 범위 체크
	if (outImageX < 0.0f || outImageY < 0.0f ||
		outImageX >= m_imageWidth || outImageY >= m_imageHeight)
		return false;

	return true;
}

bool Camera2D::ScreenToImagePixel(float screenX, float screenY, int32_t& outImageX, int32_t& outImageY) const
{
	float ix(0.0f), iy(0.0f);
	if (!ScreenToImage(screenX, screenY, ix, iy))
		return false;

	outImageX = static_cast<int>(floor(ix));
	outImageY = static_cast<int>(floor(iy));

	return true;
}

Core::ShapeType::Rect2i Camera2D::GetViewImageRect() const
{
	Core::ShapeType::Rect2i rect = {};

	const float left = m_offsetX.current;
	const float top = m_offsetY.current;
	const float right = m_offsetX.current + m_viewWidth / m_zoom.current;
	const float bottom = m_offsetY.current + m_viewHeight / m_zoom.current;

	rect.left = static_cast<int32_t>(floor(left));
	rect.top = static_cast<int32_t>(floor(top));
	rect.right = static_cast<int32_t>(floor(right));
	rect.bottom = static_cast<int32_t>(floor(bottom));

	rect.left = std::clamp(rect.left, 0, static_cast<int32_t>(m_imageWidth));
	rect.top = std::clamp(rect.top, 0, static_cast<int32_t>(m_imageHeight));
	rect.right = std::clamp(rect.right, 0, static_cast<int32_t>(m_imageWidth));
	rect.bottom = std::clamp(rect.bottom, 0, static_cast<int32_t>(m_imageHeight));

	return rect;
}
