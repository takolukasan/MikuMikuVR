#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
//#include "resource.h"

#include <stdio.h>

#include "LibOVR/Include/OVR.h"
#define OVR_D3D_VERSION 11
#include "LibOVR/Include/OVR_CAPI_D3D.h"
#undef OVR_D3D_VERSION
using namespace OVR;

#include <d3dx9.h>

#include "OculusRift.h"

#include <new>


static BOOL bLibOVRInitialized;


#define SAFE_RELEASE(p)	if(p) { (p)->Release(); (p) = (nullptr); }

#define BUFFER_LENGTH		(1024)
#define MIRRORWINDOW_PREFIX	L"Oculus Rift Mirror Window"


#define MIRROR_BACKBUFFER_WIDTH		(1920)
#define MIRROR_BACKBUFFER_HEIGHT	(1080)


static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message )
	{
		case WM_PAINT:
			hdc = BeginPaint( hWnd, &ps );
			EndPaint( hWnd, &ps );
			break;

		case WM_CLOSE:
			break;

		case WM_DESTROY:
			// PostQuitMessage( 0 );	/* これやるとアプリケーションが終了しちゃう */
			break;

		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
static HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}



OculusRiftDevice::OculusRiftDevice(int nDeviceIndex, ovrHmd hmd) : nMyDeviceIndex(nDeviceIndex), HMD(hmd)
{
	if( !hmd )
		throw (ovrResult)ovrError_InvalidParameter;

	/* メンバ変数初期化 ↑でやってもいいが好きなほうで。 */
	/* 漏れると結構な確立でバグる */

	// this->nMyDeviceIndex = nDeviceIndex;
	// this->HMD = hmd;
	this->bDebugDevice = false;

	this->HmdDesc = *hmd;
	this->uHmdCaps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	this->uTrackingCaps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position;

	this->pDXGI = nullptr;
	this->pAdapter = nullptr;
	this->pFactory = nullptr;

	this->pD3D11Device = nullptr;
	this->pD3D11Context = nullptr;

	this->hHeap = GetProcessHeap();

	this->hWndMirrorWindow = nullptr;
	this->uMirrorWindowWidth = MIRROR_BACKBUFFER_WIDTH;
	this->uMirrorWindowHeight = MIRROR_BACKBUFFER_HEIGHT;

	this->pMirrorSwapChain = nullptr;
	this->pMirrorTexture = nullptr;
	this->pMirrorBackBuffer = nullptr;
	this->pMirrorRTV = nullptr;
	this->pMirrorBackBufferDepth = nullptr;
	this->pMirrorDSV = nullptr;

	this->pMirrorVertexShader = nullptr;
	this->pMirrorPixelShader = nullptr;
	this->pMirrorVertexLayout = nullptr;
	this->pMirrorVertexBuffer = nullptr;
	this->pMirrorIndexBuffer = nullptr;
	this->pMirrorSamplerState = nullptr;

	int eye;

	for( eye = 0; eye < ovrEye_Count; eye++ ) {
		this->pTextureSet[eye] = nullptr;
		this->pD3D11RTVSet[eye] = nullptr;
		this->pMirrorTexSRV[eye] = nullptr;
		this->TexDepth[eye] = nullptr;
		this->pD3D11DSV[eye] = nullptr;
	}

	ZeroMemory(this->HmdToEyeViewOffset, sizeof(this->HmdToEyeViewOffset));
	ZeroMemory(this->eyeRenderViewport, sizeof(this->eyeRenderViewport));
	ZeroMemory(this->eyeRenderDesc, sizeof(this->eyeRenderDesc));
	ZeroMemory(this->EyeRenderPose, sizeof(this->EyeRenderPose));


	/* HMD初期化処理 */
	if( this->HmdDesc.HmdCaps & ovrHmdCap_DebugDevice )
		this->bDebugDevice = true;

	this->SetHmdCaps(ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

	this->eyeRenderDesc[ovrEye_Left] = ovrHmd_GetRenderDesc(this->HMD, ovrEye_Left, this->HMD->DefaultEyeFov[ovrEye_Left]);
	this->eyeRenderDesc[ovrEye_Right] = ovrHmd_GetRenderDesc(this->HMD, ovrEye_Right, this->HMD->DefaultEyeFov[ovrEye_Right]);
	this->HmdToEyeViewOffset[ovrEye_Left] = this->eyeRenderDesc[ovrEye_Left].HmdToEyeViewOffset;
	this->HmdToEyeViewOffset[ovrEye_Right] = this->eyeRenderDesc[ovrEye_Right].HmdToEyeViewOffset;

	if( !this->bDebugDevice ) {
 		ovrResult result;
		result = this->SetHmdTrackingCaps(ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position);
		if( !OVR_SUCCESS(result) ) {
			/* だんまりとどっちがいい？ */
			throw result;
		}
	}

}

OculusRiftDevice::~OculusRiftDevice()
{
	int eye;
	int i;

	for( eye = 0; eye < ovrEye_Count; eye++ ) {
		if( this->pTextureSet[eye] ) {
			for (i = 0; i < this->pTextureSet[eye]->TextureCount; ++i)
			{
				SAFE_RELEASE(pD3D11RTVSet[eye][i])
				SAFE_RELEASE(pMirrorTexSRV[eye][i])
			}
			ovrHmd_DestroySwapTextureSet(this->HMD, this->pTextureSet[eye]);
			this->pTextureSet[eye] = nullptr;
		}
		if( this->pD3D11RTVSet[eye] ) {
			HeapFree(this->hHeap, 0, this->pD3D11RTVSet[eye]);
			this->pD3D11RTVSet[eye] = nullptr;
		}
		if( this->pMirrorTexSRV[eye] ) {
			HeapFree(this->hHeap, 0, this->pMirrorTexSRV[eye]);
			this->pMirrorTexSRV[eye] = nullptr;
		}

		SAFE_RELEASE(this->pD3D11DSV[eye]);
		SAFE_RELEASE(this->TexDepth[eye]);

	}

	this->hHeap = nullptr;

	SAFE_RELEASE(this->pMirrorRTV);
	SAFE_RELEASE(this->pMirrorBackBuffer);
	
	SAFE_RELEASE(this->pMirrorDSV);
	SAFE_RELEASE(this->pMirrorBackBufferDepth);

    ovrHmd_DestroyMirrorTexture(this->HMD, this->pMirrorTexture);

	SAFE_RELEASE( this->pMirrorSwapChain );
	if( hWndMirrorWindow ) {
		DestroyWindow(hWndMirrorWindow);
		hWndMirrorWindow = nullptr;
	}

	SAFE_RELEASE( this->pD3D11Device );
	SAFE_RELEASE( this->pD3D11Context );

	SAFE_RELEASE( this->pDXGI );
	SAFE_RELEASE( this->pAdapter );
	SAFE_RELEASE( this->pFactory );

}

ovrResult OculusRiftDevice::GetOvrHmdDesc(ovrHmdDesc *ovrHmd)
{
	if( !ovrHmd )
		return ovrError_InvalidParameter;

	*ovrHmd = this->HmdDesc;

	return ovrSuccess;
}

HRESULT OculusRiftDevice::InitializeDirectX(ID3D11Device *pDevice, ID3D11DeviceContext *pContext)
{
	if( !pDevice || !pContext )
		return E_POINTER;

	/* DirectXインターフェース開放 */
	SAFE_RELEASE( this->pD3D11Device );
	SAFE_RELEASE( this->pD3D11Context );
	SAFE_RELEASE( this->pDXGI );
	SAFE_RELEASE( this->pAdapter );
	SAFE_RELEASE( this->pFactory );

	/* コピー */
	this->pD3D11Device = pDevice;
	this->pD3D11Device->AddRef();
	this->pD3D11Context = pContext;
	this->pD3D11Context->AddRef();


	HRESULT hr;

	hr = this->pD3D11Device->QueryInterface(IID_IDXGIDevice, (void **)&(this->pDXGI));
	if( FAILED(hr) ) {
		return hr;
	}
	hr = this->pDXGI->GetAdapter(&(this->pAdapter));
	if( FAILED(hr) ) {
		return hr;
	}
	hr = pAdapter->GetParent(IID_IDXGIFactory, (void **)&(this->pFactory));
	if( FAILED(hr) ) {
		return hr;
	}


	return ovrSuccess;
}

ovrResult OculusRiftDevice::SetupMirrorWindow(HWND *pWndMirrorWindow, HINSTANCE hInstance, UINT uWindowWidth, UINT uWindowHeight)
{

	if( !pWndMirrorWindow || !(*pWndMirrorWindow) ) {

		// Register class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof( WNDCLASSEX );
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
		wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
		wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = MIRRORWINDOW_PREFIX L" Class";
		wcex.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
		if( !RegisterClassEx( &wcex ) ) {
			// return ovrError_Initialize;
		}

		if( uWindowWidth == 0 || uWindowHeight == 0
		 || uWindowWidth > MIRROR_BACKBUFFER_WIDTH || uWindowHeight > MIRROR_BACKBUFFER_HEIGHT ) {
			uWindowWidth = MIRROR_BACKBUFFER_WIDTH;
			uWindowHeight = MIRROR_BACKBUFFER_HEIGHT;
		}

		// Create window
		WCHAR wString[BUFFER_LENGTH];
		RECT rc = { 0, 0, uWindowWidth, uWindowHeight };
		DXGI_ADAPTER_DESC Desc;

		ZeroMemory(&Desc, sizeof(Desc));

		if( this->pAdapter ) {
			this->pAdapter->GetDesc(&Desc);
		}

		AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

		/* この文字列はラッチすべし。FPS表示用に。 */
		swprintf_s(wString, BUFFER_LENGTH, L"%s / HMD Device %d%s / %s",
			MIRRORWINDOW_PREFIX, this->nMyDeviceIndex, this->bDebugDevice ? L" (Debug)" : L"" , Desc.Description);
		this->hWndMirrorWindow = CreateWindow( wcex.lpszClassName, wString, WS_OVERLAPPEDWINDOW,
							   CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
							   NULL );
		if( !this->hWndMirrorWindow )
			return ovrError_Initialize;

		ShowWindow( this->hWndMirrorWindow, SW_SHOW );

		if( pWndMirrorWindow )
			*pWndMirrorWindow = this->hWndMirrorWindow;
	}
	else {
		this->hWndMirrorWindow = *pWndMirrorWindow;

		RECT rcClient;
		GetClientRect(this->hWndMirrorWindow, &rcClient);

		uWindowWidth = rcClient.right - rcClient.left;
		uWindowHeight = rcClient.bottom - rcClient.top;
	}

	this->uMirrorWindowWidth = uWindowWidth;
	this->uMirrorWindowHeight = uWindowHeight;

	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = this->uMirrorWindowWidth;
    sd.BufferDesc.Height = this->uMirrorWindowHeight;
    // sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = this->hWndMirrorWindow;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
	hr = this->pFactory->CreateSwapChain(this->pD3D11Device, &sd, &(this->pMirrorSwapChain));


	if( FAILED(hr) ) {
		return ovrError_Initialize;
	}

	hr = this->pMirrorSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void **)&(this->pMirrorBackBuffer));
	if( FAILED(hr) ) {
		return ovrError_Initialize;
	}
	hr = this->pD3D11Device->CreateRenderTargetView(this->pMirrorBackBuffer, NULL, &(this->pMirrorRTV));
	if( FAILED(hr) ) {
		return ovrError_Initialize;
	}


	D3D11_TEXTURE2D_DESC td = { };
	td.ArraySize        = 1;
	// td.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.Width            = this->uMirrorWindowWidth;
	td.Height           = this->uMirrorWindowHeight;
	td.Usage            = D3D11_USAGE_DEFAULT;
	td.SampleDesc.Count = 1;
	td.MipLevels        = 1;

	ovrResult result;
	result = ovrHmd_CreateMirrorTextureD3D11(this->HMD, this->pD3D11Device, &td, &(this->pMirrorTexture));

	if( this->bDebugDevice ) {
		// Compile the vertex shader
		ID3DBlob* pVSBlob = NULL;
		hr = CompileShaderFromFile( MIRROR_EFFECT_FILE, MIRROR_EFFECT_VS, "vs_4_0", &pVSBlob );
		if( FAILED( hr ) )
		{
			MessageBox( NULL,
						L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
			return ovrError_Initialize;
		}

		// Create the vertex shader
		hr = this->pD3D11Device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &(this->pMirrorVertexShader) );
		if( FAILED( hr ) )
		{    
			pVSBlob->Release();
			return ovrError_Initialize;
		}

		// Define the input layout
		static const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE( layout );

		// Create the input layout
		hr = this->pD3D11Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
											  pVSBlob->GetBufferSize(), &(this->pMirrorVertexLayout) );
		pVSBlob->Release();
		if( FAILED( hr ) )
			return ovrError_Initialize;


		// Compile the pixel shader
		ID3DBlob* pPSBlob = NULL;
		hr = CompileShaderFromFile( MIRROR_EFFECT_FILE, MIRROR_EFFECT_PS, "ps_4_0", &pPSBlob );
		if( FAILED( hr ) )
		{
			MessageBox( NULL,
						L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
			return ovrError_Initialize;
		}

		// Create the pixel shader
		hr = this->pD3D11Device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &(this->pMirrorPixelShader) );
		pPSBlob->Release();
		if( FAILED( hr ) )
			return ovrError_Initialize;

		// Create vertex buffer
		static const MirrorVertexBuffer vertices[] =
		{
			{ XMFLOAT3( -1.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },	// 左上
			{ XMFLOAT3( +0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },	// 右上
			{ XMFLOAT3( +0.0f, +1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },	// 右下
			{ XMFLOAT3( -1.0f, +1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },	// 左下
			{ XMFLOAT3( +0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },	// 左上
			{ XMFLOAT3( +1.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },	// 右上
			{ XMFLOAT3( +1.0f, +1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },	// 右下
			{ XMFLOAT3( +0.0f, +1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },	// 左下
		};

		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( vertices );
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		hr = this->pD3D11Device->CreateBuffer( &bd, &InitData, &(this->pMirrorVertexBuffer) );
		if( FAILED( hr ) )
			return hr;


		// Create index buffer
		static const WORD indices[] =
		{
			3,1,0,
			2,1,3,
		};

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( indices );
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		hr = this->pD3D11Device->CreateBuffer( &bd, &InitData, &(this->pMirrorIndexBuffer) );
		if( FAILED( hr ) )
			return hr;

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = this->pD3D11Device->CreateSamplerState( &sampDesc, &(this->pMirrorSamplerState) );
		if( FAILED( hr ) )
			return hr;



		D3D11_TEXTURE2D_DESC dsDesc;
		dsDesc.Width = this->uMirrorWindowWidth;
		dsDesc.Height = this->uMirrorWindowHeight;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.CPUAccessFlags = 0;
		dsDesc.MiscFlags = 0;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		this->pD3D11Device->CreateTexture2D(&dsDesc, NULL, &(this->pMirrorBackBufferDepth));
		this->pD3D11Device->CreateDepthStencilView(this->pMirrorBackBufferDepth, NULL, &(this->pMirrorDSV));

	}

	return result;
}

ovrResult OculusRiftDevice::CreateEyeRenderTexture(OVR::Sizei EyeTexSize[ovrEye_Count])
{
	ovrResult result;

	if( EyeTexSize[ovrEye_Left].w <= 0 || EyeTexSize[ovrEye_Left].h <= 0 ||
		EyeTexSize[ovrEye_Right].w <= 0 || EyeTexSize[ovrEye_Right].h <= 0
	)
		return ovrError_InvalidParameter;


	D3D11_TEXTURE2D_DESC dsDesc;
	int eye;
	int i;

    for (eye = 0; eye < 2; eye++)
    {
        // pEyeRenderTexture[eye]      = new OculusTexture(HMD, idealSize);
		dsDesc.Width            = EyeTexSize[eye].w;
		dsDesc.Height           = EyeTexSize[eye].h;
		dsDesc.MipLevels        = 1;
		dsDesc.ArraySize        = 1;
		// dsDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		dsDesc.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
		dsDesc.SampleDesc.Count = 1;   // No multi-sampling allowed
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage            = D3D11_USAGE_DEFAULT;
		dsDesc.CPUAccessFlags   = 0;
		dsDesc.MiscFlags        = 0;
		dsDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		result = ovrHmd_CreateSwapTextureSetD3D11(this->HMD, this->pD3D11Device, &dsDesc, &(this->pTextureSet[eye]));
		if( !OVR_SUCCESS(result) )
			return result;

		this->pD3D11RTVSet[eye] = (ID3D11RenderTargetView **)HeapAlloc(this->hHeap, 0, sizeof(ID3D11RenderTargetView *) * pTextureSet[eye]->TextureCount);
		if( !this->pD3D11RTVSet[eye] ) {
			return ovrError_MemoryAllocationFailure;
		}

		this->pMirrorTexSRV[eye] = (ID3D11ShaderResourceView **)HeapAlloc(this->hHeap, 0, sizeof(ID3D11ShaderResourceView *) * pTextureSet[eye]->TextureCount);
		if( !this->pMirrorTexSRV[eye] ) {
			return ovrError_MemoryAllocationFailure;
		}

        for (i = 0; i < this->pTextureSet[eye]->TextureCount; ++i)
        {
            ovrD3D11Texture* tex = (ovrD3D11Texture*)&(this->pTextureSet[eye]->Textures[i]);
			this->pD3D11Device->CreateRenderTargetView(tex->D3D11.pTexture, NULL, &(this->pD3D11RTVSet[eye][i]));
			this->pD3D11Device->CreateShaderResourceView(tex->D3D11.pTexture, NULL, &(this->pMirrorTexSRV[eye][i]));
        }

		// pEyeDepthBuffer[eye]        = new DepthBuffer(DIRECTX.Device, idealSize);
		dsDesc.Width = EyeTexSize[eye].w;
		dsDesc.Height = EyeTexSize[eye].h;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.CPUAccessFlags = 0;
		dsDesc.MiscFlags = 0;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		this->pD3D11Device->CreateTexture2D(&dsDesc, NULL, &(this->TexDepth[eye]));
		this->pD3D11Device->CreateDepthStencilView(this->TexDepth[eye], NULL, &(this->pD3D11DSV[eye]));

        this->eyeRenderViewport[eye].Pos  = Vector2i(0, 0);
        this->eyeRenderViewport[eye].Size = EyeTexSize[eye];

		this->eyeRenderDesc[eye] = ovrHmd_GetRenderDesc(this->HMD, (ovrEyeType)eye, this->HMD->DefaultEyeFov[eye]);
   }

	return ovrSuccess;
}

ovrResult OculusRiftDevice::BeginFrame(ovrPosef *ppEyeRenderPose)
{
	ovrFrameTiming ftiming  = ovrHmd_GetFrameTiming(this->HMD, 0);
	ovrTrackingState hmdState = ovrHmd_GetTrackingState(this->HMD, ftiming.DisplayMidpointSeconds);
	ovr_CalcEyePoses(hmdState.HeadPose.ThePose, this->HmdToEyeViewOffset, this->EyeRenderPose);

	if( ppEyeRenderPose ) {
		ppEyeRenderPose[ovrEye_Left] = this->EyeRenderPose[ovrEye_Left];
		ppEyeRenderPose[ovrEye_Right] = this->EyeRenderPose[ovrEye_Right];
	}

	return ovrSuccess;
}

ovrResult OculusRiftDevice::BeginEyeRender(ovrEyeType eye, bool bClearRTAndDSV, const float fRTClearColor[4])
{
	int nTexIndex;

	this->pTextureSet[eye]->CurrentIndex = (this->pTextureSet[eye]->CurrentIndex + 1) % this->pTextureSet[eye]->TextureCount;
	nTexIndex = this->pTextureSet[eye]->CurrentIndex;

	if( bClearRTAndDSV ) {
		// DIRECTX.SetAndClearRenderTarget(pEyeRenderTexture[eye]->TexRtv[texIndex], pEyeDepthBuffer[eye]);
		this->pD3D11Context->OMSetRenderTargets(1, &(this->pD3D11RTVSet[eye][nTexIndex]), this->pD3D11DSV[eye]);
		this->pD3D11Context->ClearRenderTargetView((this->pD3D11RTVSet[eye][nTexIndex]), fRTClearColor);
		this->pD3D11Context->ClearDepthStencilView(this->pD3D11DSV[eye], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}

	// DIRECTX.SetViewport(Recti(eyeRenderViewport[eye]));
	Recti vp(this->eyeRenderViewport[eye]);
	D3D11_VIEWPORT D3Dvp;
	D3Dvp.Width = (float)vp.w;    D3Dvp.Height = (float)vp.h;
	D3Dvp.MinDepth = 0;           D3Dvp.MaxDepth = 1;
	D3Dvp.TopLeftX = (float)vp.x; D3Dvp.TopLeftY = (float)vp.y;
	this->pD3D11Context->RSSetViewports(1, &D3Dvp);

	return ovrSuccess;
}

ovrResult OculusRiftDevice::EndFrame()
{
	ovrResult result;

	// Initialize our single full screen Fov layer.
	ovrLayerEyeFov ld;
	ld.Header.Type  = ovrLayerType_EyeFov;
	ld.Header.Flags = 0;	// ovrLayerFlag_HighQuality

	for (int eye = 0; eye < ovrEye_Count; eye++)
	{
		ld.ColorTexture[eye] = this->pTextureSet[eye];
		ld.Viewport[eye]     = this->eyeRenderViewport[eye];
		ld.Fov[eye]          = this->HMD->DefaultEyeFov[eye];
		ld.RenderPose[eye]   = this->EyeRenderPose[eye];
	}


	ovrLayerHeader* layers = &ld.Header;

	result = ovrHmd_SubmitFrame(this->HMD, 0, nullptr, &layers, 1);


	HRESULT hr;
	const float white[4] = { 255, 255, 255, 255 };
	if( this->hWndMirrorWindow && this->pMirrorSwapChain && this->pMirrorTexture && !this->bDebugDevice ) {

		this->pD3D11Context->OMSetRenderTargets(1, &this->pMirrorRTV, this->pMirrorDSV);
		this->pD3D11Context->ClearRenderTargetView(this->pMirrorRTV, white);

		ovrD3D11Texture* tex = (ovrD3D11Texture *)this->pMirrorTexture;
		this->pD3D11Context->CopyResource(this->pMirrorBackBuffer, tex->D3D11.pTexture);
		hr = this->pMirrorSwapChain->Present(0, 0);

	}
	else if( this->bDebugDevice ) {
		this->pD3D11Context->OMSetRenderTargets(1, &this->pMirrorRTV, this->pMirrorDSV);
		this->pD3D11Context->ClearRenderTargetView(this->pMirrorRTV, white);
		this->pD3D11Context->ClearDepthStencilView(this->pMirrorDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

		D3D11_VIEWPORT D3Dvp;
		D3Dvp.Width = MIRROR_BACKBUFFER_WIDTH;    D3Dvp.Height = MIRROR_BACKBUFFER_HEIGHT;
		D3Dvp.MinDepth = 0.0f;    D3Dvp.MaxDepth = 1.0f;
		D3Dvp.TopLeftX = (float)0.0f; D3Dvp.TopLeftY = (float)0.0f;
		this->pD3D11Context->RSSetViewports(1, &D3Dvp);

		// Set the input layout
		this->pD3D11Context->IASetInputLayout( this->pMirrorVertexLayout );


		ID3D11Buffer *pBakVB,*pBakIB;
		UINT BakStride,BakOffsetVB,BakOffsetIB;
		DXGI_FORMAT BakFormat;

		this->pD3D11Context->IAGetVertexBuffers(0, 1, &pBakVB, &BakStride, &BakOffsetVB);
		this->pD3D11Context->IAGetIndexBuffer(&pBakIB, &BakFormat, &BakOffsetIB);

		// Set vertex buffer
		UINT stride = sizeof( MirrorVertexBuffer );
		UINT offset = 0;
		this->pD3D11Context->IASetVertexBuffers( 0, 1, &(this->pMirrorVertexBuffer), &stride, &offset );

		this->pD3D11Context->IASetIndexBuffer( this->pMirrorIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

		this->pD3D11Context->VSSetShader( this->pMirrorVertexShader, NULL, 0 );
		this->pD3D11Context->PSSetShader( this->pMirrorPixelShader, NULL, 0 );
		this->pD3D11Context->PSSetSamplers( 0, 1, &(this->pMirrorSamplerState) );

		// Set primitive topology
		this->pD3D11Context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		this->pD3D11Context->PSSetShaderResources( 0, 1, &(this->pMirrorTexSRV[ovrEye_Left][this->pTextureSet[ovrEye_Left]->CurrentIndex]) );
		this->pD3D11Context->DrawIndexed( 6, 0, 0 );
		this->pD3D11Context->PSSetShaderResources( 0, 1, &(this->pMirrorTexSRV[ovrEye_Right][this->pTextureSet[ovrEye_Right]->CurrentIndex]) );
		this->pD3D11Context->DrawIndexed( 6, 0, 4 );

		hr = this->pMirrorSwapChain->Present(0, 0);

		this->pD3D11Context->IASetVertexBuffers(0, 1, &pBakVB, &BakStride, &BakOffsetVB);
		this->pD3D11Context->IASetIndexBuffer(pBakIB, BakFormat, BakOffsetIB);

	}

	return result;
}

XMMATRIX * OculusRiftDevice::GetProjectionMatrix(XMMATRIX xmProjection[ovrEye_Count])
{
	/* DX9版と同じく、こっちも XMMatrixPerspectiveOffCenterLH() で綺麗にできる？ */

	XMMATRIX matProj[ovrEye_Count];

	/* Setup projection matrix */
	float projXScale;
	float projXOffset;
	float projYScale;
	float projYOffset;
	float zNear = 0.01f;
	float zFar = 1000.0f;

	if( !xmProjection )
		return nullptr;

	int i;

	for(i = 0; i < ovrEye_Count; i++ ) {
		projXScale = 2.0f / ( this->eyeRenderDesc[i].Fov.LeftTan + this->eyeRenderDesc[i].Fov.RightTan );
		projXOffset = ( this->eyeRenderDesc[i].Fov.LeftTan - this->eyeRenderDesc[i].Fov.RightTan ) * projXScale * 0.5f;
		projYScale = 2.0f / ( this->eyeRenderDesc[i].Fov.UpTan + this->eyeRenderDesc[i].Fov.DownTan );
		projYOffset = ( this->eyeRenderDesc[i].Fov.UpTan - this->eyeRenderDesc[i].Fov.DownTan ) * projYScale * 0.5f;

		// Produces X result, mapping clip edges to [-w,+w]
		matProj[i].m[0][0] = projXScale;
		matProj[i].m[0][1] = 0.0f;
		matProj[i].m[0][2] = projXOffset;
		matProj[i].m[0][3] = 0.0f;

		// Produces Y result, mapping clip edges to [-w,+w]
		// Hey - why is that YOffset negated?
		// It's because a projection matrix transforms from world coords with Y=up,
		// whereas this is derived from an NDC scaling, which is Y=down.
		matProj[i].m[1][0] = 0.0f;
		matProj[i].m[1][1] = projYScale;
		matProj[i].m[1][2] = -projYOffset;
		matProj[i].m[1][3] = 0.0f;

		// Produces Z-buffer result - app needs to fill this in with whatever Z range it wants.
		// We'll just use some defaults for now.
		matProj[i].m[2][0] = 0.0f;
		matProj[i].m[2][1] = 0.0f;
		matProj[i].m[2][2] = -1.0f * zFar / (zNear - zFar);
		matProj[i].m[2][3] = (zFar * zNear) / (zNear - zFar);

		// Produces W result (= Z in)
		matProj[i].m[3][0] = 0.0f;
		matProj[i].m[3][1] = 0.0f;
		matProj[i].m[3][2] = 1.0f;
		matProj[i].m[3][3] = 0.0f;

		XMMatrixTranspose(matProj[i]);	// 冗長というか↑加工すれば消せるけどまぁいいや
		xmProjection[i] = matProj[i];
	}

	return xmProjection;
}

XMMATRIX * OculusRiftDevice::GetViewMatrixFromLastPose(XMMATRIX xmEyePositionAndRotation[ovrEye_Count])
{
	XMMATRIX matRotate,matTrans;
	XMVECTOR q;

	if( !xmEyePositionAndRotation )
		return nullptr;

	q = XMVectorSet(
		this->EyeRenderPose[ovrEye_Left].Orientation.x, this->EyeRenderPose[ovrEye_Left].Orientation.y,
		-this->EyeRenderPose[ovrEye_Left].Orientation.z, this->EyeRenderPose[ovrEye_Left].Orientation.w);

	matRotate = XMMatrixRotationQuaternion(q);
	matTrans = XMMatrixTranslation(
		-this->EyeRenderPose[ovrEye_Left].Position.x * 10,
		-this->EyeRenderPose[ovrEye_Left].Position.y * 10,
		this->EyeRenderPose[ovrEye_Left].Position.z * 10);
	xmEyePositionAndRotation[ovrEye_Left] = matTrans * matRotate;

	q = XMVectorSet(
		this->EyeRenderPose[ovrEye_Left].Orientation.x, this->EyeRenderPose[ovrEye_Left].Orientation.y,
		-this->EyeRenderPose[ovrEye_Left].Orientation.z, this->EyeRenderPose[ovrEye_Left].Orientation.w);
	matRotate = XMMatrixRotationQuaternion(q);
	matTrans = XMMatrixTranslation(
		-this->EyeRenderPose[ovrEye_Right].Position.x * 10,
		-this->EyeRenderPose[ovrEye_Right].Position.y * 10,
		this->EyeRenderPose[ovrEye_Right].Position.z * 10);
	xmEyePositionAndRotation[ovrEye_Right] = matTrans * matRotate;

	return xmEyePositionAndRotation;
}


#ifdef OCULUSRIFT_SUPPORT_DIRECTX9

D3DXMATRIX * OculusRiftDevice::GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count])
{
	const float zNear = 1.0f;
	const float zFar = 1000.0f;

	int i;

	if( !matProjection )
		return nullptr;

	for(i = 0; i < ovrEye_Count; i++ ) {
		D3DXMatrixPerspectiveOffCenterLH(
			&matProjection[i],
			-this->eyeRenderDesc[i].Fov.LeftTan,
			this->eyeRenderDesc[i].Fov.RightTan,
			-this->eyeRenderDesc[i].Fov.DownTan,
			this->eyeRenderDesc[i].Fov.UpTan,
			zNear, zFar
		);
	}

#if 0	/* SDKからコピペしてたけど↑で動いたのでいらない？ */
	D3DXMATRIX matProj[ovrEye_Count];

	/* Setup projection matrix */
	float projXScale;
	float projXOffset;
	float projYScale;
	float projYOffset;

	for(i = 0; i < ovrEye_Count; i++ ) {
		projXScale = 2.0f / ( this->eyeRenderDesc[i].Fov.LeftTan + this->eyeRenderDesc[i].Fov.RightTan );
		projXOffset = ( this->eyeRenderDesc[i].Fov.LeftTan - this->eyeRenderDesc[i].Fov.RightTan ) * projXScale * 0.5f;
		projYScale = 2.0f / ( this->eyeRenderDesc[i].Fov.UpTan + this->eyeRenderDesc[i].Fov.DownTan );
		projYOffset = ( this->eyeRenderDesc[i].Fov.UpTan - this->eyeRenderDesc[i].Fov.DownTan ) * projYScale * 0.5f;

		// Produces X result, mapping clip edges to [-w,+w]
		matProj[i].m[0][0] = projXScale;
		matProj[i].m[0][1] = 0.0f;
		matProj[i].m[0][2] = projXOffset;
		matProj[i].m[0][3] = 0.0f;

		// Produces Y result, mapping clip edges to [-w,+w]
		// Hey - why is that YOffset negated?
		// It's because a projection matrix transforms from world coords with Y=up,
		// whereas this is derived from an NDC scaling, which is Y=down.
		matProj[i].m[1][0] = 0.0f;
		matProj[i].m[1][1] = projYScale;
		matProj[i].m[1][2] = -projYOffset;
		matProj[i].m[1][3] = 0.0f;

		// Produces Z-buffer result - app needs to fill this in with whatever Z range it wants.
		// We'll just use some defaults for now.
		matProj[i].m[2][0] = 0.0f;
		matProj[i].m[2][1] = 0.0f;
		matProj[i].m[2][2] = -1.0f * zFar / (zNear - zFar);
		matProj[i].m[2][3] = (zFar * zNear) / (zNear - zFar);

		// Produces W result (= Z in)
		matProj[i].m[3][0] = 0.0f;
		matProj[i].m[3][1] = 0.0f;
		matProj[i].m[3][2] = 1.0f;
		matProj[i].m[3][3] = 0.0f;

		D3DXMatrixTranspose(&matProjection[i], &matProj[i]);	// 冗長というか↑加工すれば消せるけどまぁいいや
	}
#endif

	return matProjection;
}

D3DXMATRIX * OculusRiftDevice::GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count], double CustomFovOffset)
{
	const float zNear = 1.0f;
	const float zFar = 1000.0f;

	int i;

	double fov_l,fov_r,fov_t,fov_b;
	double aspect;
	float fl_tan,fr_tan,ft_tan,fb_tan;

	if( !matProjection )
		return nullptr;

	for(i = 0; i < ovrEye_Count; i++ ) {
		D3DXMatrixPerspectiveOffCenterLH(
			&matProjection[i],
			-this->eyeRenderDesc[i].Fov.LeftTan,
			this->eyeRenderDesc[i].Fov.RightTan,
			-this->eyeRenderDesc[i].Fov.DownTan,
			this->eyeRenderDesc[i].Fov.UpTan,
			zNear, zFar
		);

		fov_l = atan((double)(this->eyeRenderDesc[i].Fov.LeftTan));
		fov_r = atan((double)(this->eyeRenderDesc[i].Fov.RightTan));
		fov_t = atan((double)(this->eyeRenderDesc[i].Fov.UpTan));
		fov_b = atan((double)(this->eyeRenderDesc[i].Fov.DownTan));
		aspect = (fov_t + fov_b) / (fov_l + fov_r);

		if( CustomFovOffset > 0.0 && CustomFovOffset < 1.0) {
			fov_l *= CustomFovOffset;
			fov_r *= CustomFovOffset;
			fov_t *= CustomFovOffset;
			fov_b *= CustomFovOffset;
		}

		fl_tan = (float)tan(fov_l);
		fr_tan = (float)tan(fov_r);
		ft_tan = (float)tan(fov_t);
		fb_tan = (float)tan(fov_b);

		D3DXMatrixPerspectiveOffCenterLH(
			&matProjection[i],
			-fl_tan,
			fr_tan,
			-ft_tan,
			fb_tan,
			zNear, zFar
		);
	}

	return matProjection;
}

D3DXMATRIX * OculusRiftDevice::GetViewMatrixFromLastPose(D3DXMATRIX matEyePositionAndRotation[ovrEye_Count])
{
	D3DXMATRIX matRotate,matTrans;
	D3DXQUATERNION q;

	if( !matEyePositionAndRotation )
		return nullptr;

	q = D3DXQUATERNION(
		this->EyeRenderPose[ovrEye_Left].Orientation.x, this->EyeRenderPose[ovrEye_Left].Orientation.y,
		-this->EyeRenderPose[ovrEye_Left].Orientation.z, this->EyeRenderPose[ovrEye_Left].Orientation.w);
	D3DXMatrixRotationQuaternion(&matRotate, &q);
	D3DXMatrixTranslation(
		&matTrans,
		-this->EyeRenderPose[ovrEye_Left].Position.x * 10,
		-this->EyeRenderPose[ovrEye_Left].Position.y * 10,
		this->EyeRenderPose[ovrEye_Left].Position.z * 10);
	matEyePositionAndRotation[ovrEye_Left] = matTrans * matRotate;

	q = D3DXQUATERNION(
		this->EyeRenderPose[ovrEye_Left].Orientation.x, this->EyeRenderPose[ovrEye_Left].Orientation.y,
		-this->EyeRenderPose[ovrEye_Left].Orientation.z, this->EyeRenderPose[ovrEye_Left].Orientation.w);
	D3DXMatrixRotationQuaternion(&matRotate, &q);
	D3DXMatrixTranslation(
		&matTrans,
		-this->EyeRenderPose[ovrEye_Right].Position.x * 10,
		-this->EyeRenderPose[ovrEye_Right].Position.y * 10,
		this->EyeRenderPose[ovrEye_Right].Position.z * 10);
	matEyePositionAndRotation[ovrEye_Right] = matTrans * matRotate;

	return matEyePositionAndRotation;
}

#endif


OculusRift::OculusRift()
{
	int i;

	for(i = 0; i < OculusRift_MaxDevices; i++ ) {
		this->pDevices[i] = nullptr;
	}
}

OculusRift::~OculusRift()
{

}

ovrResult OculusRift::Initialize()
{
	ovrResult ovrInitializeResult;
	ovrInitializeResult = ovr_Initialize(nullptr);
	if( ovrSuccess == ovrInitializeResult )
		bLibOVRInitialized = TRUE;
	return ovrInitializeResult;
}

void OculusRift::UnInitialize()
{
	bLibOVRInitialized = FALSE;
	return ovr_Shutdown();
}

int OculusRift::GetNumOfOculusRift()
{
	ovrResult result;
	result = ovrHmd_Detect();

	if( !OVR_SUCCESS(result) )
		return 0;

	return (int)result;
}

ovrResult OculusRift::CreateDevice(int nDeviceIndex, OculusRiftDevice **ppDevice)
{
	ovrResult result;
	ovrHmd hmd;
	OculusRiftDevice *pDeviceReturn;

	if( !bLibOVRInitialized || nDeviceIndex < 0 || nDeviceIndex >= OculusRift_MaxDevices || !ppDevice )
		return ovrError_InvalidParameter;

	if( this->pDevices[nDeviceIndex] )
		return ovrError_InvalidParameter;

	result = ovrHmd_Create(nDeviceIndex, &hmd);
	if( ovrSuccess == result ) {
		try {
			pDeviceReturn = new OculusRiftDevice(nDeviceIndex, hmd);
		}
		catch(std::bad_alloc) {
			ovrHmd_Destroy(hmd);
			return ovrError_MemoryAllocationFailure;
		}
		*ppDevice = pDeviceReturn;
		this->pDevices[nDeviceIndex] = pDeviceReturn;
	}

	return result;
}

ovrResult OculusRift::CreateDebugDevice(int nDeviceIndex, OculusRiftDevice **ppDevice)
{
	ovrResult result;
	ovrHmd hmd;
	OculusRiftDevice *pDeviceReturn;

	if( !bLibOVRInitialized || nDeviceIndex < 0 || nDeviceIndex >= OculusRift_MaxDevices || !ppDevice )
		return ovrError_InvalidParameter;

	if( this->pDevices[nDeviceIndex] )
		return ovrError_InvalidParameter;

	result = ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
	if( ovrSuccess == result ) {
		try {
			pDeviceReturn = new OculusRiftDevice(nDeviceIndex, hmd);
		}
		catch(std::bad_alloc) {
			ovrHmd_Destroy(hmd);
			return ovrError_MemoryAllocationFailure;
		}
		*ppDevice = pDeviceReturn;
		this->pDevices[nDeviceIndex] = pDeviceReturn;
	}

	return result;
}

ovrResult OculusRift::DestoryDevice(int nDeviceIndex)
{
	if( nDeviceIndex < 0 || nDeviceIndex >= OculusRift_MaxDevices )
		return ovrError_InvalidParameter;

	if( !bLibOVRInitialized || !this->pDevices[nDeviceIndex] )
		return ovrError_InvalidParameter;

	delete this->pDevices[nDeviceIndex];
	this->pDevices[nDeviceIndex] = nullptr;

	return ovrSuccess;
}

