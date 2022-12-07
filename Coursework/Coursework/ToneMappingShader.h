#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"

using namespace DirectX;

class ToneMappingShader : public BaseFullScreenShader
{
private:
	struct ParamsBufferType
	{
		float avgL;
		float whitePoint;
		float blackPoint;
		float toe;
		float shoulder;
		float crossPoint;
		XMFLOAT2 padding;
	};

public:
	ToneMappingShader(ID3D11Device* device);
	~ToneMappingShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth);

	void SettingsGUI();

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

private:
	ID3D11Buffer* m_ParamsBuffer = nullptr;

	float m_AvgL = 1.0f;
	float m_WhitePoint = 1.5f;
	float m_BlackPoint = 0.0f;
	float m_Toe = 0.1f;
	float m_Shoulder = 0.15f;
	float m_CrossPoint = 0.0f;

};
