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
		float threshold;
		float smoothing;
		XMFLOAT2 padding;
	};

public:
	BloomShader(ID3D11Device* device, unsigned int width, unsigned int height, unsigned int levels);
	~BloomShader();

	void SettingsGUI();

	void Run(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input);

	inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }

	inline int GetLevels() const { return m_Levels; }
	inline float GetStrength() const { return m_Strength; }

private:
	void LoadCS(ID3D11Device* device, const wchar_t* filename, ID3D11ComputeShader** ppCS);

	inline ID3D11UnorderedAccessView* GetUAV(int mip) { assert(mip < m_Levels); return m_UAVs[mip]; }

private:
	ID3D11ComputeShader* m_Shader = nullptr;

	ID3D11Texture2D* m_BloomTex = nullptr;
	unsigned int m_Width = -1;
	unsigned int m_Height = -1;

	ID3D11ShaderResourceView* m_SRV = nullptr;

	// a UAV is needed for every texture in the mip chain
	std::vector<ID3D11UnorderedAccessView*> m_UAVs;
	int m_Levels = -1;

	ID3D11Buffer* m_CSBuffer = nullptr;

	ID3D11SamplerState* m_TrilinearSampler = nullptr;

	// params
	float m_Threshold = 0.95f;
	float m_Smoothing = 0.15f;
	float m_Strength = 0.2f;
};
