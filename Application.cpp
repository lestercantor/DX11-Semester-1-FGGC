#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBufferCube = nullptr;
	_pIndexBufferCube = nullptr;
    _pVertexBufferPyramid = nullptr;
    _pIndexBufferPyramid = nullptr;
    _pVertexBufferPlane = nullptr;
    _pIndexBufferPlane = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
    //objMeshData = OBJLoader::Load("sphere.obj", _pd3dDevice);
    //XMStoreFloat4x4(&_plane, XMMatrixIdentity());

    // Specular light position
    eyePosW = XMFLOAT3(0.1f, 10.0f, 0.0f);

    // Initialize the camera object
    _camera = Camera(
        XMFLOAT3(0.1f, 10.0f, 0.0f), 
        XMFLOAT3(0.0f, 0.0f, 0.0f), 
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        _WindowWidth, _WindowHeight, 0.01f, 100.0f);


    // Light direction from surface (XYZ)
    lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
    // Diffuse material properties (RGBA)
    diffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
    // Diffuse light colour (RGBA)
    diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // Ambient light material properties (RGBA)
    ambientMatieral = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
    // Ambient light colour (RGBA)
    ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);

    // Specular light material properties (RGBA)
    specularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    // Specular light colour (RGBA)
    specularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    // Specular light power
    specularPower = 10.0f;

    // Load texture
  //CreateDDSTextureFromFile(_pd3dDevice, L"Textures/Crate_COLOR.dds", nullptr, &_pTextureRV);

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

    objMeshData = OBJLoader::Load("OBJ/torusKnot.obj", _pd3dDevice);

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    D3D11_BUFFER_DESC bd;
    D3D11_SUBRESOURCE_DATA InitData;

    // Create vertex buffer for cube
    SimpleVertex cubeVertices[] =
    {
        // Face 1
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.4, -0.4, 0.2), XMFLOAT2(0.0f, 0.0f) }, // 0
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.25, -0.25, 0.5), XMFLOAT2(1.0f, 0.0f) }, // 1
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-0.25, 0.25, 0.5), XMFLOAT2(0.0f, 1.0f) }, // 2
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.4, 0.4, 0.2), XMFLOAT2(1.0f, 1.0f) }, // 3
        // Face 2 
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.25, -0.25, 0.5), XMFLOAT2(0.0f, 0.0f) }, // 4
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.4, -0.4, -0.2), XMFLOAT2(1.0f, 0.0f) },// 5
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.4, 0.4, 0.2), XMFLOAT2(0.0f, 1.0f) }, // 6
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.25, 0.25, -0.5), XMFLOAT2(1.0f, 1.0f) }, // 7
        // Face 3
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.4, -0.4, -0.2), XMFLOAT2(0.0f, 0.0f) }, // 8
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.5, -0.5, -0.25), XMFLOAT2(1.0f, 0.0f) }, // 9 
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.25, 0.25, -0.5), XMFLOAT2(0.0f, 1.0f) }, // 10
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-0.4, 0.4, -0.2), XMFLOAT2(1.0f, 1.0f) }, // 11
        // Face 4
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.5, -0.5, -0.25), XMFLOAT2(0.0f, 0.0f) }, // 12
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.4, -0.4, 0.2), XMFLOAT2(1.0f, 0.0f) }, // 13
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-0.4, 0.4, -0.2), XMFLOAT2(0.0f, 1.0f) }, // 14
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-0.25, 0.25, 0.5), XMFLOAT2(1.0f, 1.0f) }, // 15
        // Face 5
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-0.25, 0.25, 0.5), XMFLOAT2(0.0f, 0.0f) }, // 16
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.4, 0.4, 0.2), XMFLOAT2(1.0f, 0.0f) }, // 17  
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-0.4, 0.4, -0.2), XMFLOAT2(0.0f, 1.0f) }, // 18
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.25, 0.25, -0.5), XMFLOAT2(1.0f, 1.0f) }, // 19
        // Face 6
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.5, -0.5, -0.25), XMFLOAT2(0.0f, 0.0f) }, // 20
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.4, -0.4, -0.2), XMFLOAT2(1.0f, 0.0f) }, // 21
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.4, -0.4, 0.2), XMFLOAT2(0.0f, 1.0f) }, // 22 
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.25, -0.25, 0.5), XMFLOAT2(1.0f, 1.0f) }, // 23
    };

	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = cubeVertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferCube);

    if (FAILED(hr))
        return hr;

    //// Create vertex buffer for pyramid
    //SimpleVertex pyramidVertices[] =
    //{
    //    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.3f, -0.33f, 0.3f) },
    //    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-0.22f, -0.5f, 0.22f) },
    //    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-0.3f, -0.33f, -0.3f) },
    //    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.22f, -0.5f, -0.22f) },
    //    { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
    //};

    //ZeroMemory(&bd, sizeof(bd));
    //bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.ByteWidth = sizeof(SimpleVertex) * 5;
    //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    //bd.CPUAccessFlags = 0;

    //ZeroMemory(&InitData, sizeof(InitData));
    //InitData.pSysMem = pyramidVertices;

    //hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferPyramid);

    //if (FAILED(hr))
    //    return hr;

    //// Create vertex buffer for flat plane
    //SimpleVertex planeVertices[] =
    //{
    //    { XMFLOAT3(-6.0f, -1.0f, -6.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-6.0f, -1.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-6.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-6.0f, -1.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-6.0f, -1.0f, 6.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

    //    { XMFLOAT3(-3.0f, -1.0f, -6.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-3.0f, -1.0f, -3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-3.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-3.0f, -1.0f, 3.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    //    { XMFLOAT3(-3.0f, -1.0f, 6.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

    //    { XMFLOAT3(0.0f, -1.0f, -6.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(0.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(0.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(0.0f, -1.0f, 6.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },

    //    { XMFLOAT3(3.0f, -1.0f, -6.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(3.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(3.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(3.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(3.0f, -1.0f, 6.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },

    //    { XMFLOAT3(6.0f, -1.0f, -6.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(6.0f, -1.0f, -3.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(6.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(6.0f, -1.0f, 3.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(6.0f, -1.0f, 6.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
    //};

    //ZeroMemory(&bd, sizeof(bd));
    //bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.ByteWidth = sizeof(SimpleVertex) * 25;
    //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    //bd.CPUAccessFlags = 0;

    //ZeroMemory(&InitData, sizeof(InitData));
    //InitData.pSysMem = planeVertices;

    //hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferPlane);

    //if (FAILED(hr))
    //    return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    D3D11_BUFFER_DESC bd;
    D3D11_SUBRESOURCE_DATA InitData;

    // Create index buffer for cube
    WORD indicesCube[] =
    {
        // Face 1
        0,1,2,
        2,1,3,
        // Face 2
        4,5,6,
        6,5,7,
        // Face 3
        8,9,10,
        10,9,11,
        // Face 4
        12,13,14,
        14,13,15,
        // Face 5
        16,17,18,
        18,17,19,
        // Face 6
        20,21,22,
        22,21,23,
    };

	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * cubeIndex;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indicesCube;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferCube);

    if (FAILED(hr))
        return hr;

    //// Create index buffer for pyramid
    //WORD indicesPyr[] =
    //{
    //    // Base of pyramid
    //    0,1,3,
    //    3,1,2,
    //    // Face 1
    //    0,4,1,
    //    // Face 2
    //    1,4,2,
    //    // Face 3
    //    2,4,3,
    //    // Face 4
    //    3,4,0,
    //};

    //ZeroMemory(&bd, sizeof(bd));
    //bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.ByteWidth = sizeof(WORD) * pyramidIndex;
    //bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //bd.CPUAccessFlags = 0;

    //ZeroMemory(&InitData, sizeof(InitData));
    //InitData.pSysMem = indicesPyr;
    //hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferPyramid);

    //if (FAILED(hr))
    //    return hr;

    //// Create index buffer for plane
    //WORD indicesPlane[] =
    //{
    //    0,1,5,
    //    5,1,6,
    //    1,2,6,
    //    6,2,7,
    //    2,3,7,
    //    7,3,8,
    //    3,4,8,
    //    8,4,9,

    //    5,6,10,
    //    10,6,11,
    //    6,7,11,
    //    11,7,12,
    //    7,8,12,
    //    12,8,13,
    //    8,9,13,
    //    13,9,14,

    //    10,11,15,
    //    15,11,16,
    //    11,12,16,
    //    16,12,17,
    //    12,13,17,
    //    17,13,18,
    //    13,14,18,
    //    18,14,19,

    //    15,16,20,
    //    20,16,21,
    //    16,17,21,
    //    21,17,22,
    //    17,18,22,
    //    22,18,23,
    //    18,19,23,
    //    23,19,24,
    //};

    //ZeroMemory(&bd, sizeof(bd));
    //bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.ByteWidth = sizeof(WORD) * planeIndex;
    //bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //bd.CPUAccessFlags = 0;

    //ZeroMemory(&InitData, sizeof(InitData));
    //InitData.pSysMem = indicesPlane;
    //hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferPlane);

    //if (FAILED(hr))
    //    return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    // Defines depth/stencil buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    // Create depth/stencil view
    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    if (FAILED(hr))
        return hr;

    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);
    if (FAILED(hr))
        return hr;

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferCube, &stride, &offset);

	InitIndexBuffer();

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pIndexBufferCube, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    // Create a rasterizer state for wireframe rendering
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    // Create a rasterizer state for solid rendering
    D3D11_RASTERIZER_DESC sdesc;
    ZeroMemory(&sdesc, sizeof(D3D11_RASTERIZER_DESC));
    sdesc.FillMode = D3D11_FILL_SOLID;
    sdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&sdesc, &_solid);

    return S_OK;

}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBufferCube) _pVertexBufferCube->Release();
    if (_pIndexBufferCube) _pIndexBufferCube->Release();
    if (_pVertexBufferPyramid) _pVertexBufferPyramid->Release();
    if (_pIndexBufferPyramid) _pIndexBufferPyramid->Release();
    if (_pVertexBufferPlane) _pVertexBufferPlane->Release();
    if (_pIndexBufferPlane) _pIndexBufferPlane->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_wireFrame) _wireFrame->Release();
    if (_solid) _solid->Release();
}

void Application::Update()
{
    _camera.Update();

    int matTrans;
    int matRota;
    int matScale;

    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    // For shader
    gTime = t;

    //
    // Animate the cube
    //
    XMStoreFloat4x4(&_world, XMMatrixRotationX(t));
    // Animate planets
    XMStoreFloat4x4(&_world1, XMMatrixRotationY(t * 1.3) * XMMatrixTranslation(15.0f, 0.0f, 1.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t * 1.1));
    XMStoreFloat4x4(&_world2, XMMatrixRotationY(t * 1.4) * XMMatrixTranslation(11.0f, 0.0f, 1.1f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t * 1.2));
    //// Animate moons for first planet
    //XMStoreFloat4x4(&_moon1, XMMatrixRotationY(t * 4) * XMMatrixTranslation(6.0f, 0.0f, 0.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMLoadFloat4x4(&_world1));
    //XMStoreFloat4x4(&_moon2, XMMatrixRotationY(t * 10) * XMMatrixTranslation(5.0f, 0.0f, 0.0f) * XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixRotationY(t * 1.2) * XMLoadFloat4x4(&_world1));
    //// Animate moons for second planet
    //XMStoreFloat4x4(&_moon3, XMMatrixRotationY(t * 5) * XMMatrixTranslation(6.0f, 0.0f, 0.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixRotationY(t * 0.8) * XMLoadFloat4x4(&_world2));
    //XMStoreFloat4x4(&_moon4, XMMatrixRotationY(t * 20) * XMMatrixTranslation(5.0f, 0.0f, 0.0f) * XMMatrixScaling(0.4f, 0.4f, 0.4f) * XMMatrixRotationY(t * 1.5) * XMLoadFloat4x4(&_world2));

    //for (int i = 0; i < 100; i++) {
    //    matTrans = rand() % 10 + 50;
    //    matRota = rand() % 2 + 6;
    //    matScale = rand() % 2 + 6;
    //    XMStoreFloat4x4(&_asteroidBelt[i], XMMatrixRotationY(t * matRota) * XMMatrixTranslation(10.0f + matTrans, 0.0f, 0.0f) * XMMatrixScaling(0.9f / matScale, 0.9f / matScale, 0.9f / matScale) * XMMatrixRotationY(t + matTrans));
    //}

    // Change rasterizer state with a key press
    if (GetAsyncKeyState(VK_UP)) 
        _pImmediateContext->RSSetState(_wireFrame);
    
    if (GetAsyncKeyState(VK_DOWN)) 
        _pImmediateContext->RSSetState(_solid);
    
    if (GetAsyncKeyState(VK_LEFT)) {
        for (int i = 0; i < 100; i++) {
            XMStoreFloat4x4(&_asteroidBelt[i], XMLoadFloat4x4(&_asteroidBelt[i]) * XMMatrixRotationY(-t) * XMMatrixRotationY(-t));
        }
    }
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[5] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_camera.getViewMatrix());
	XMMATRIX projection = XMLoadFloat4x4(&_camera.getProjectionMatrix());
    //
    // Update variables
    //
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
    cb.gTime = gTime;
    // Diffuse lighting
    cb.DiffuseLight = diffuseLight;
    cb.DiffuseMtrl = diffuseMaterial;
    cb.LightVecW = lightDirection;
    // Ambient lighting
    cb.AmbientLight = ambientLight;
    cb.AmbientMtrl = ambientMatieral;
    // Specular lighting
    cb.SpecularLight = specularLight;
    cb.SpecularMtrl = specularMaterial;
    cb.SpecularPower = specularPower;
    cb.EyePosW = eyePosW;


    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Switch to cube buffers
    _pImmediateContext->IASetVertexBuffers(0, 1, &objMeshData.VertexBuffer, &objMeshData.VBStride, &objMeshData.VBOffset);
    _pImmediateContext->IASetIndexBuffer(objMeshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //
    // Renders a triangle
    //
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0); 

    //// Converts XMFloat4x4 to XMMatrix and renders a new cube - first planet
    //world = XMLoadFloat4x4(&_world1);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(cubeIndex, 0, 0);

    //// Renders a third cube - second planet
    //world = XMLoadFloat4x4(&_world2);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(cubeIndex, 0, 0);

    //// Switch to pyramid buffers
    //_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferPyramid, &stride, &offset);
    //_pImmediateContext->IASetIndexBuffer(_pIndexBufferPyramid, DXGI_FORMAT_R16_UINT, 0);

    //// Renders a fourth cube - first moon
    //world = XMLoadFloat4x4(&_moon1);
    //cb.mWorld = XMMatrixTranspose(world);;
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(pyramidIndex, 0, 0);

    //// Renders a fifth cube - second moon
    //world = XMLoadFloat4x4(&_moon2);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(pyramidIndex, 0, 0);

    //// Renders a sixth cube - third moon
    //world = XMLoadFloat4x4(&_moon3);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(pyramidIndex, 0, 0);

    //// Renders a seventh cube - fourth moon
    //world = XMLoadFloat4x4(&_moon4);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(pyramidIndex, 0, 0);

    //// Renders 100 more cubes - asteroid belt
    //for (int i = 0; i < 100; i++) {
    //    world = XMLoadFloat4x4(&_asteroidBelt[i]);
    //    cb.mWorld = XMMatrixTranspose(world);
    //    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //    _pImmediateContext->DrawIndexed(pyramidIndex, 0, 0);
    //}

    //// Switch to plane buffers
    //_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferPlane, &stride, &offset);
    //_pImmediateContext->IASetIndexBuffer(_pIndexBufferPlane, DXGI_FORMAT_R16_UINT, 0);
    //
    //world = XMLoadFloat4x4(&_plane);
    //cb.mWorld = XMMatrixTranspose(world);
    //_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    //_pImmediateContext->DrawIndexed(planeIndex, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}