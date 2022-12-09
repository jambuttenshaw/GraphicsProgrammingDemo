#pragma once

#include "Cubemap.h"


class ShadowCubemap : public Cubemap
{
public:
	ShadowCubemap(ID3D11Device* device, unsigned int resolution);
	virtual ~ShadowCubemap();

	inline ID3D11DepthStencilView* GetDSV(int face) const { return m_DSVs[face]; }

protected:

	ID3D11DepthStencilView* m_DSVs[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

};