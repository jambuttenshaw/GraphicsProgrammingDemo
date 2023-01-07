#include "MeasureLuminanceShader.h"

#include <cmath>
#include <cassert>
#include <algorithm>

#include <d3dcompiler.h>


MeasureLuminanceShader::MeasureLuminanceShader(ID3D11Device* device, unsigned int backBufferW, unsigned int backBufferH)
{
	LoadCS(device, L"reduceto1d_cs.cso", &m_ReduceTo1DShader);
	LoadCS(device, L"reducetosingle_cs.cso", &m_ReduceToSingleShader);

	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(CSBufferType);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	hr = device->CreateBuffer(&bufferDesc, nullptr, &m_CSBuffer);
	assert(hr == S_OK);

	// create 2 structured buffers used for parallel reduction algorithm
	// these will act as ping-pong buffers
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = static_cast<int>(ceilf(backBufferW / 8.0f) * ceilf(backBufferH / 8.0f)) * sizeof(float);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(float);
	
	hr = device->CreateBuffer(&bufferDesc, nullptr, &m_ReductionBuffer0);
	assert(hr == S_OK);
	hr = device->CreateBuffer(&bufferDesc, nullptr, &m_ReductionBuffer1);
	assert(hr == S_OK);

	// create uav and srv for each buffer

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / sizeof(float);
	uavDesc.Buffer.Flags = 0;
	
	hr = device->CreateUnorderedAccessView(m_ReductionBuffer0, &uavDesc, &m_ReductionUAV0);
	assert(hr == S_OK);
	hr = device->CreateUnorderedAccessView(m_ReductionBuffer1, &uavDesc, &m_ReductionUAV1);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = uavDesc.Buffer.FirstElement;
	srvDesc.Buffer.NumElements = uavDesc.Buffer.NumElements;

	hr = device->CreateShaderResourceView(m_ReductionBuffer0, &srvDesc, &m_ReductionSRV0);
	assert(hr == S_OK);
	hr = device->CreateShaderResourceView(m_ReductionBuffer1, &srvDesc, &m_ReductionSRV1);
	assert(hr == S_OK);

}

MeasureLuminanceShader::~MeasureLuminanceShader()
{
	// release resources
	m_ReduceTo1DShader->Release();
	m_ReduceToSingleShader->Release();
	m_CSBuffer->Release();

	m_ReductionBuffer0->Release();
	m_ReductionBuffer1->Release();

	m_ReductionUAV0->Release();
	m_ReductionUAV1->Release();

	m_ReductionSRV0->Release();
	m_ReductionSRV1->Release();
}

void MeasureLuminanceShader::Run(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input, unsigned int inputW, unsigned int inputH)
{
	// calculate initial number of groups 
	unsigned int dimx = static_cast<int>(ceilf(inputW / 8.0f));
	unsigned int dimy = static_cast<int>(ceilf(inputH / 8.0f));

	// first CS pass is to reduce the 2D texture into a 1D buffer
	RunCS(deviceContext, m_ReduceTo1DShader, &input, &m_ReductionUAV0, XMUINT2{ inputW, inputH }, XMUINT2{ dimx, dimy });

	// subsequent passes reduce the buffer to a single summed luminance value

	unsigned int dim = dimx * dimy;
	// the number of thread groups used in the last reduction that still need reduced
	unsigned int numToReduce = dim;
	// how many thread groups to dispatch
	dim = static_cast<int>(ceilf(dim / 128.0f));
	if (numToReduce > 1)
	{
		while (true)
		{
			// run reduction pass
			RunCS(deviceContext, m_ReduceToSingleShader, &m_ReductionSRV0, &m_ReductionUAV1, XMUINT2{ numToReduce, 0 }, XMUINT2{ dim, 1 });

			// update num thread groups
			numToReduce = dim;
			dim = static_cast<int>(ceilf(dim / 128.0f));

			// loop until there is a single element in the buffer
			if (numToReduce == 1)
				break;

			// swap in and out buffers (ping-pong)
			std::swap(m_ReductionBuffer0, m_ReductionBuffer1);
			std::swap(m_ReductionUAV0, m_ReductionUAV1);
			std::swap(m_ReductionSRV0, m_ReductionSRV1);
		}
	}
	else
	{
		// no further reduction necessary
		// swap buffers
		std::swap(m_ReductionBuffer0, m_ReductionBuffer1);
		std::swap(m_ReductionUAV0, m_ReductionUAV1);
		std::swap(m_ReductionSRV0, m_ReductionSRV1);
	}

	// output is in the first element of buffer 0
}

void MeasureLuminanceShader::LoadCS(ID3D11Device* device, const wchar_t* filename, ID3D11ComputeShader** ppCS)
{
	// load compute shader from file
	ID3D10Blob* computeShaderBuffer;

	// Reads compiled shader into buffer (bytecode).
	HRESULT hr = D3DReadFileToBlob(filename, &computeShaderBuffer);
	assert(hr == S_OK && "Failed to load shader");

	// Create the compute shader from the buffer.
	hr = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, ppCS);
	assert(hr == S_OK);

	computeShaderBuffer->Release();
}

void MeasureLuminanceShader::RunCS(	ID3D11DeviceContext* deviceContext, ID3D11ComputeShader* cs,
									ID3D11ShaderResourceView** input, ID3D11UnorderedAccessView** output,
									XMUINT2 inputDims, XMUINT2 groupCount)
{
	// set shader and srv/uav
	deviceContext->CSSetShader(cs, nullptr, 0);
	deviceContext->CSSetShaderResources(0, 1, input);
	deviceContext->CSSetUnorderedAccessViews(0, 1, output, nullptr);

	// setup params constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_CSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CSBufferType* dataPtr = (CSBufferType*)mappedResource.pData;
	dataPtr->inputDims = inputDims;
	dataPtr->groupCount = groupCount;
	deviceContext->Unmap(m_CSBuffer, 0);
	deviceContext->CSSetConstantBuffers(0, 1, &m_CSBuffer);

	// dispatch
	deviceContext->Dispatch(groupCount.x, groupCount.y, 1);

	// unbind resources
	// (to prevent attempting to bind same resource as input and output simutaneously)
	ID3D11Buffer* nullCB = nullptr;
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetConstantBuffers(0, 1, &nullCB);
	deviceContext->CSSetShaderResources(0, 1, &nullSRV);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}

