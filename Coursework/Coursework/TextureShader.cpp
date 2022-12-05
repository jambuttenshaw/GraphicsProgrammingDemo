#include "TextureShader.h"

TextureShader::TextureShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"texture_vs.cso", L"texture_ps.cso");
}

TextureShader::~TextureShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void TextureShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc, lightMatrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	lightMatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightMatrixBufferDesc.ByteWidth = sizeof(LightMatrixBufferType);
	lightMatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightMatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightMatrixBufferDesc.MiscFlags = 0;
	lightMatrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightMatrixBufferDesc, NULL, &lightMatrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}


void TextureShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, const XMMATRIX& lightProj)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);
	XMMATRIX tlightproj = XMMatrixTranspose(lightProj);

	{
		// Send matrix data
		MatrixBufferType* dataPtr;
		result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr = (MatrixBufferType*)mappedResource.pData;
		dataPtr->world = tworld;// worldMatrix;
		dataPtr->view = tview;
		dataPtr->projection = tproj;
		deviceContext->Unmap(matrixBuffer, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	}
	{
		LightMatrixBufferType* dataPtr;
		result = deviceContext->Map(lightMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr = (LightMatrixBufferType*)mappedResource.pData;
		dataPtr->projection = tlightproj;
		deviceContext->Unmap(lightMatrixBuffer, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &lightMatrixBuffer);
	}


	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}





