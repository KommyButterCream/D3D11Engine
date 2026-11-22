#pragma once

#ifdef GRAPHICS_ENGINE_DLL
#define IMMEDIATE_CONTEXT_GATE_API __declspec(dllexport)
#else
#define IMMEDIATE_CONTEXT_GATE_API __declspec(dllimport)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "../../../Module/D3D11EngineInterface/ID3D11ImmediateContextGate.h"

class IMMEDIATE_CONTEXT_GATE_API D3D11ImmediateContextGate final : public ID3D11ImmediateContextGate
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
