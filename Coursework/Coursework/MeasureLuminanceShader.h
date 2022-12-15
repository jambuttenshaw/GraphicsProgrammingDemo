#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;


class MeasureLuminanceShader
{
private:
	struct CSBufferType
	{
		XMUINT2 inputDims;
		XMUINT2 groupCount;
	};

public:
	MeasureLuminanceShader(ID3D11Device* device, unsigned int backBufferW, unsigned int backBufferH);
	~MeasureLuminanceShader();

	void Run(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input, unsigned int inputW, unsigned int inputH);

	inline ID3D11ShaderResourceView* GetResult() const { return m_ReductionSRV1; }

private:
	void LoadCS(ID3D11Device* device, const wchar_t* filename, ID3D11ComputeShader** ppCS);

	void RunCS(ID3D11DeviceContext* deviceContext, ID3D11ComputeShader* cs, ID3D11ShaderResourceView** input, ID3D11UnorderedAccessView** output, XMUINT2 inputDims, XMUINT2 groupCount);

private:
	ID3D11ComputeShader* m_ReduceTo1DShader = nullptr;
	ID3D11ComputeShader* m_ReduceToSingleShader = nullptr;
	ID3D11Buffer* m_CSBuffer = nullptr;

	ID3D11Buffer* m_ReductionBuffer0 = nullptr;
	ID3D11Buffer* m_ReductionBuffer1 = nullptr;

	ID3D11UnorderedAccessView* m_ReductionUAV0 = nullptr;
	ID3D11UnorderedAccessView* m_ReductionUAV1 = nullptr;

	ID3D11ShaderResourceView* m_ReductionSRV0 = nullptr;
	ID3D11ShaderResourceView* m_ReductionSRV1 = nullptr;
};