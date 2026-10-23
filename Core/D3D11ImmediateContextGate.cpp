#include "pch.h"
#include "D3D11ImmediateContextGate.h"

bool D3D11ImmediateContextGate::Enter()
{
	if (::ReadAcquire(&m_enabled) == FALSE)
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
	return ::ReadAcquire(&m_enabled) != FALSE;
}

void D3D11ImmediateContextGate::SetEnabled(bool enabled)
{
	::InterlockedExchange(&m_enabled, enabled ? TRUE : FALSE);
}
