#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <cassert>
#include <vector>


class BloomShader
{
private:

	struct CSBufferType
	{
		XMFLOAT4 params;
	};

public:
	BloomShader(ID3D11Device* device, unsigned int width, unsigned int height, unsigned int levels);
	~BloomShader();

	void SettingsGUI();

	void Run(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input);

	inline ID3D11ShaderResourceView* GetSRV() const { return m_Levels[0].srv1; }
	
private:
	void LoadCS(ID3D11Device* device, const wchar_t* filename, ID3D11ComputeShader** ppCS);

	void RunCS(ID3D11DeviceContext* deviceContext, ID3D11ComputeShader* shader, ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output, int outputLevel, void* csBufferData, size_t csBufferDataSize);

private:
	ID3D11ComputeShader* m_BrightpassShader = nullptr;
	ID3D11ComputeShader* m_CombineShader = nullptr;
	ID3D11ComputeShader* m_HorizontalBlurShader = nullptr;
	ID3D11ComputeShader* m_VerticalBlurShader = nullptr;

	unsigned int m_Width = -1;
	unsigned int m_Height = -1;

	struct BloomLevel
	{
		ID3D11ShaderResourceView* srv1 = nullptr;
		ID3D11ShaderResourceView* srv2 = nullptr;
		ID3D11UnorderedAccessView* uav1 = nullptr;
		ID3D11UnorderedAccessView* uav2 = nullptr;

		void Release()
		{
			if (srv1) srv1->Release();
			if (srv2) srv2->Release();
			if (uav1) uav1->Release();
			if (uav2) uav2->Release();
		}
	};
	std::vector<BloomLevel> m_Levels;
	int m_LevelCount = -1;

	ID3D11Buffer* m_CSBuffer = nullptr;

	ID3D11SamplerState* m_TrilinearSampler = nullptr;

	// params
	float m_Threshold = 1.5f;
};
