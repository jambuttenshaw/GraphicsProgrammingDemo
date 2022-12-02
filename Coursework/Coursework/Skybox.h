#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

#include "DXF.h"

class Cubemap;


class Skybox
{
	struct VSBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	Skybox(ID3D11Device* device, Cubemap* cubemap);
	~Skybox();

	void Render(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

private:
	void LoadVS();
	void LoadPS();

private:
	ID3D11Device* m_Device = nullptr;

	CubeMesh* m_CubeMesh = nullptr;
	Cubemap* m_Cubemap = nullptr;

	ID3D11VertexShader* m_VS = nullptr;
	ID3D11PixelShader* m_PS = nullptr;

	ID3D11Buffer* m_VSBuffer = nullptr;
	ID3D11SamplerState* m_SamplerState = nullptr;
	
	ID3D11RasterizerState* m_RasterState = nullptr;
	ID3D11DepthStencilState* m_DepthState = nullptr;

	ID3D11InputLayout* m_InputLayout = nullptr;
	
};