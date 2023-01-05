#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"

using namespace DirectX;

class FinalPassShader : public BaseFullScreenShader
{
private:
	struct ParamsBufferType
	{
		// tone mapping
		int enableTonemapping;
		float avgLumFactor;

		float hdrMax; 
		float contrast;
		float shoulder;
		float midIn; 
		float midOut;
		float crosstalk;
		float white;

		// bloom
		int enableBloom;
		float bloomStrength;

		float padding;
	};

public:
	FinalPassShader(ID3D11Device* device);
	~FinalPassShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth,
		ID3D11ShaderResourceView* luminance, unsigned int w, unsigned int h, ID3D11ShaderResourceView* bloom);

	void SettingsGUI();

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

private:
	ID3D11Buffer* m_ParamsBuffer = nullptr;
	ID3D11SamplerState* m_TrilinearSampler = nullptr;

	bool m_EnableTonemapping = true;

	float m_HDRMax = 16.0f;
	float m_Contrast = 2.0f;
	float m_Shoulder = 0.97f;
	float m_MidIn = 0.26f;
	float m_MidOut = 0.1f;
	float m_Crosstalk = 4.0f;
	float m_White = 1.0f;

	bool m_EnableBloom = true;
	float m_BloomStrength = 1.0f;
};
