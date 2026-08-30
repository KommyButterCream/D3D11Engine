#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "../../../Module/D3D11EngineInterface/ID3D11ImmediateContextGate.h"

class D3D11ImmediateContextGate final : public ID3D11ImmediateContextGate
{
public:
	D3D11ImmediateContextGate() = default;
	~D3D11ImmediateContextGate() override = default;

	D3D11ImmediateContextGate(const D3D11ImmediateContextGate&) = delete;
	D3D11ImmediateContextGate& operator=(const D3D11ImmediateContextGate&) = delete;

	bool Enter() override;
	void Leave() override;
	bool IsEnabled() const override;

	void SetEnabled(bool enabled);

private:
	SRWLOCK m_lock = SRWLOCK_INIT;
	mutable volatile LONG m_enabled = FALSE;
};
