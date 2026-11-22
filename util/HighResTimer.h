#pragma once

class HighResTimer
{
public:
	HighResTimer();

	void Reset();     // Reset timer state.
	void Tick();      // Advance one frame.

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;

	float GetFPS() const;

private:
	LARGE_INTEGER m_freq;
	LARGE_INTEGER m_prev;
	LARGE_INTEGER m_curr;

	double m_deltaSec = 0.0;
	double m_totalSec = 0.0;

	// FPS bookkeeping.
	int    m_frameCount = 0;
	double m_fpsTimeAcc = 0.0;
	float  m_fps = 0.0f;
};
