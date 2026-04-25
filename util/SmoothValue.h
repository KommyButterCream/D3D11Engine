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

	inline bool IsAtTarget() const
	{
		return fabs(target - current) < 1e-4f;
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