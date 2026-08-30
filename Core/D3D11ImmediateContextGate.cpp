#include "pch.h"
#include "D3D11ImmediateContextGate.h"

bool D3D11ImmediateContextGate::Enter()
{
	if (::InterlockedCompareExchange(&m_enabled, FALSE, FALSE) == FALSE)
		return false;

	::AcquireSRWLockExclusive(&m_lock);
	return true;
}

void D3D11ImmediateContextGate::Leave()
{
	::ReleaseSRWLockExclusive(&m_lock);
}

bool D3D11ImmediateContextGate::IsEnabled() const
{
	return ::InterlockedCompareExchange(&m_enabled, FALSE, FALSE) != FALSE;
}

void D3D11ImmediateContextGate::SetEnabled(bool enabled)
{
	::InterlockedExchange(&m_enabled, enabled ? TRUE : FALSE);
}
