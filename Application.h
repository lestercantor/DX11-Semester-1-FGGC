#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	float gTime;
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

	// Vertex and index buffers for cube
	ID3D11Buffer*           _pVertexBufferCube;
	ID3D11Buffer*           _pIndexBufferCube;
	int cubeIndex = 36;

	// Vertex and index buffers for pyramid
	ID3D11Buffer* _pVertexBufferPyramid;
	ID3D11Buffer* _pIndexBufferPyramid;
	int pyramidIndex = 18;

	// Vertex and index buffers for flat plane
	ID3D11Buffer* _pVertexBufferPlane;
	ID3D11Buffer* _pIndexBufferPlane;
	int planeIndex = 96;

	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _sun, _world1, _world2, _moon1, _moon2, _moon3, _moon4, _plane;
	XMFLOAT4X4				_asteroidBelt[100];
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;

	// Set up the depth/stencil buffer 
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	// Set up render states
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solid;

	float gTime;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

