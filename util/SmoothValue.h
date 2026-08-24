#pragma once

#include <math.h>

class SmoothValue
{
public:
	float current = 0.0f;
	float target = 0.0f;

	inline void Update(float dt, float speed = 12.0f)
	{
		float k = 1.0f - expf(-speed * dt);
		current += (target - current) * k;
	}

	inline void Snap(float value)
	{
		current = target = value;
	}

	inline float GetCurrent() const
	{
		return current;
	}

	inline void SetTarget(float value)
	{
		target = value;
	}

	// 지수 보간은 목표에 점근적으로만 접근하므로 꼬리가 무한히 길다.
	// 고정 임계값(1e-4)은 값의 스케일을 모르기 때문에, 화면에서 이미 무의미한
	// 구간을 수십 프레임 더 돌게 만든다. 호출자가 자기 단위에 맞는 허용치를
	// 넘길 수 있도록 인자를 받는다.
	inline bool IsAtTarget(float epsilon) const
	{
		return fabs(target - current) < epsilon;
	}

	inline bool IsAtTarget() const
	{
		return IsAtTarget(1e-4f);
	}
};

class SmoothColor
{
public:
	SmoothValue r, g, b, a;

	inline void Update(float dt, float speed)
	{
		r.Update(dt, speed);
		g.Update(dt, speed);
		b.Update(dt, speed);
		a.Update(dt, speed);
	}

	inline void Snap(const D2D1_COLOR_F& color)
	{
		r.Snap(color.r);
		g.Snap(color.g);
		b.Snap(color.b);
		a.Snap(color.a);
	}

	inline void SetTarget(const D2D1_COLOR_F& color)
	{
		r.SetTarget(color.r);
		g.SetTarget(color.g);
		b.SetTarget(color.b);
		a.SetTarget(color.a);
	}

	inline D2D1_COLOR_F GetColor() const
	{
		return D2D1::ColorF(
			Clamp01(r.current),
			Clamp01(g.current),
			Clamp01(b.current),
			Clamp01(a.current)
		);
	}

	inline bool IsAtTarget() const
	{
		return r.IsAtTarget() &&
			g.IsAtTarget() &&
			b.IsAtTarget() &&
			a.IsAtTarget();
	}

private:
	inline static float Clamp01(float v)
	{
		return (v < 0.f) ? 0.f : (v > 1.f ? 1.f : v);
	}
};

class Inertia2D
{
public:
	float vx = 0.f;
	float vy = 0.f;

	inline void Update(float dt, float damping = 10.0f)
	{
		float k = expf(-damping * dt);
		vx *= k;
		vy *= k;

		if (fabs(vx) < 0.001f) vx = 0.f;
		if (fabs(vy) < 0.001f) vy = 0.f;
	}
};