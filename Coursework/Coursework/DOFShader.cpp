#include "DOFShader.h"

DOFShader::DOFShader(ID3D11Device* device)
	: BaseFullScreenShader(device)
{
	Init(L"dof_ps.cso");
}

DOFShader::~DOFShader()
{
}

void DOFShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth)
{
	ID3D11ShaderResourceView* psSRVs[2] = { renderTextureColour, renderTextureDepth };
	deviceContext->PSSetShaderResources(0, 2, psSRVs);
}

void DOFShader::CreateShaderResources()
{
}

void DOFShader::UnbindShaderResources(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}
