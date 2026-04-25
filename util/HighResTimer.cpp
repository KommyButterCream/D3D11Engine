#include "pch.h"
#include "HighResTimer.h"

HighResTimer::HighResTimer()
{
    QueryPerformanceFrequency(&m_freq);
    Reset();
}

void HighResTimer::Reset()
{
    QueryPerformanceCounter(&m_prev);
    m_curr = m_prev;

    m_deltaSec = 0.0;
    m_totalSec = 0.0;

    m_frameCount = 0;
    m_fpsTimeAcc = 0.0;
    m_fps = 0.0f;
}

void HighResTimer::Tick()
{
    QueryPerformanceCounter(&m_curr);

    LONGLONG deltaCnt = m_curr.QuadPart - m_prev.QuadPart;
    m_prev = m_curr;

    m_deltaSec = static_cast<double>(deltaCnt) /
        static_cast<double>(m_freq.QuadPart);

    // Clamp long pauses from breakpoints or debugger stops.
    if (m_deltaSec > 0.1)
        m_deltaSec = 0.1;

    m_totalSec += m_deltaSec;

    // Update FPS once per second.
    m_frameCount++;
    m_fpsTimeAcc += m_deltaSec;

    if (m_fpsTimeAcc >= 1.0)
    {
        m_fps = static_cast<float>(m_frameCount / m_fpsTimeAcc);
        m_frameCount = 0;
        m_fpsTimeAcc = 0.0;
    }
}

float HighResTimer::GetDeltaSeconds() const
{
    return static_cast<float>(m_deltaSec);
}

float HighResTimer::GetTotalSeconds() const
{
    return static_cast<float>(m_totalSec);
}

float HighResTimer::GetFPS() const
{
    return m_fps;
}
