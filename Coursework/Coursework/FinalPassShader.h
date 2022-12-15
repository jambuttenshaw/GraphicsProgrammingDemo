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
		float avgLumFactor;
		float whitePoint;
		float blackPoint;
		float toe;

		float shoulder;
		float crossPoint;

		// bloom
		int bloomLevels;
		float bloomStrength;
	};

public:
	FinalPassShader(ID3D11Device* device);
	~FinalPassShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth,
		ID3D11ShaderResourceView* luminance, unsigned int w, unsigned int h, ID3D11ShaderResourceView* bloom, int bloomLevels, float bloomStrength);

	void SettingsGUI();

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

private:
	ID3D11Buffer* m_ParamsBuffer = nullptr;
	ID3D11SamplerState* m_TrilinearSampler = nullptr;

	float m_WhitePoint = 2.0f;
	float m_BlackPoint = 0.0f;
	float m_Toe = 0.1f;
	float m_Shoulder = 0.1f;
	float m_CrossPoint = 0.15f;
};
