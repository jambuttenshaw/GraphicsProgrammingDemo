#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"

using namespace DirectX;

class DOFShader : public BaseFullScreenShader
{
public:
	DOFShader(ID3D11Device* device);
	~DOFShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth);

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

};
