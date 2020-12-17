#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "Structures.h"
#include "OBJLoader.h"
#include "Camera.h"

using namespace DirectX;

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightVecW;
	float gTime;

	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 AmbientLight;

	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	XMFLOAT3 EyePosW;
	float SpecularPower;
};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;

	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4				_world;

	// Set up the depth/stencil buffer 
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	// Set up render states
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solid;

	float gTime;

	// Lighting Variables
	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 ambientMatieral;
	XMFLOAT4 ambientLight;

	XMFLOAT4 specularMaterial;
	XMFLOAT4 specularLight;
	XMFLOAT3 eyePosW;
	float specularPower;

	// Texture variables
	ID3D11ShaderResourceView* _pTextureRV = nullptr;
	ID3D11SamplerState* _pSamplerLinear = nullptr;
	
	MeshData objMeshData;
	MeshData _plane;

	Camera _camera;
	Camera _camera2;
	
private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

