#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

#include <stdio.h>
#include <new>

#include <d3dx9.h>

#include <OVR_CAPI_D3D.h>
#include <Kernel/OVR_Nullptr.h>


#include "OculusRift.h"


using namespace std;


static BOOL bLibOVRInitialized;


#define SAFE_RELEASE(p)	if(p) { (p)->Release(); (p) = (nullptr); }

#define BUFFER_LENGTH		(1024)
#define MIRRORWINDOW_PREFIX	L"Oculus Rift Mirror Window"


#define MIRROR_BACKBUFFER_WIDTH		(1920)
#define MIRROR_BACKBUFFER_HEIGHT	(1080)


static const float zNear = 0.01f;
static const float zFar = 100000.0f;


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

OculusRiftDevice::OculusRiftDevice(int nDeviceIndex, ovrSession session, ovrGraphicsLuid luid) : nMyDeviceIndex(nDeviceIndex), Session(session), luidGPU(luid)
{
	if( !session )
		throw (ovrResult)ovrError_InvalidParameter;

	/* メンバ変数初期化 ↑でやってもいいが好きなほうで。 */
	/* 漏れると結構な確立でバグる */

	// this->nMyDeviceIndex = nDeviceIndex;

	this->HmdDesc = ovr_GetHmdDesc(this->Session);
#if 0
	this->uHmdCaps = ovr_GetTrackingCaps(this->Session);
#endif
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
	this->MirrorTexture = nullptr;
	this->pMirrorBackBuffer = nullptr;
	this->pMirrorRTV = nullptr;
	this->pMirrorBackBufferDepth = nullptr;

	int eye;

	for( eye = 0; eye < ovrEye_Count; eye++ ) {
		this->EyeTextureSwapChain[eye] = nullptr;
		this->pEyeTexture[eye] = nullptr;
		this->pD3D11RTVSet[eye] = nullptr;
		this->TexDepth[eye] = nullptr;
		this->pD3D11DSV[eye] = nullptr;
	}

	ZeroMemory(this->HmdToEyeOffset, sizeof(this->HmdToEyeOffset));
	ZeroMemory(this->eyeRenderViewport, sizeof(this->eyeRenderViewport));
	ZeroMemory(this->eyeRenderDesc, sizeof(this->eyeRenderDesc));
	ZeroMemory(this->EyeRenderPose, sizeof(this->EyeRenderPose));

	this->eyeRenderDesc[ovrEye_Left] = ovr_GetRenderDesc(this->Session, ovrEye_Left, this->HmdDesc.DefaultEyeFov[ovrEye_Left]);
	this->eyeRenderDesc[ovrEye_Right] = ovr_GetRenderDesc(this->Session, ovrEye_Right, this->HmdDesc.DefaultEyeFov[ovrEye_Right]);
	this->HmdToEyeOffset[ovrEye_Left] = this->eyeRenderDesc[ovrEye_Left].HmdToEyeOffset;
	this->HmdToEyeOffset[ovrEye_Right] = this->eyeRenderDesc[ovrEye_Right].HmdToEyeOffset;

}

OculusRiftDevice::~OculusRiftDevice()
{
	int eye;
	int i;

	for( eye = 0; eye < ovrEye_Count; eye++ ) {
		if( this->EyeTextureSwapChain[eye] ) {
			int len = 0;
			len = ovr_GetTextureSwapChainLength(this->Session, this->EyeTextureSwapChain[eye], &len);
			for (i = 0; i < len; ++i)
			{
				SAFE_RELEASE(pEyeTexture[eye][i]);
				SAFE_RELEASE(pD3D11RTVSet[eye][i])
			}
			ovr_DestroyTextureSwapChain(this->Session, this->EyeTextureSwapChain[eye]);
			this->EyeTextureSwapChain[eye] = nullptr;
		}
		if( this->pEyeTexture[eye] ) {
			HeapFree(this->hHeap, 0, this->pEyeTexture[eye]);
			this->pEyeTexture[eye] = nullptr;
		}
		if( this->pD3D11RTVSet[eye] ) {
			HeapFree(this->hHeap, 0, this->pD3D11RTVSet[eye]);
			this->pD3D11RTVSet[eye] = nullptr;
		}

		SAFE_RELEASE(this->pD3D11DSV[eye]);
		SAFE_RELEASE(this->TexDepth[eye]);

	}

	this->hHeap = nullptr;

	SAFE_RELEASE(this->pMirrorRTV);
	SAFE_RELEASE(this->pMirrorBackBuffer);

	SAFE_RELEASE(this->pMirrorBackBufferDepth);

	if( this->MirrorTexture )
	    ovr_DestroyMirrorTexture(this->Session, this->MirrorTexture);

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
		swprintf_s(wString, BUFFER_LENGTH, L"%s / HMD Device %d / %s",
			MIRRORWINDOW_PREFIX, this->nMyDeviceIndex, Desc.Description);
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
		int nWidth,nHeight;
		GetClientRect(this->hWndMirrorWindow, &rcClient);
		nWidth = rcClient.right - rcClient.left;
		nHeight = rcClient.bottom - rcClient.top;

		// ミラー用バックバッファのサイズは引数,与えられたウィンドウ,デフォルトサイズ(FHD)のうち一番大きいものにしておく
		if( uWindowWidth < MIRROR_BACKBUFFER_WIDTH || uWindowHeight < MIRROR_BACKBUFFER_HEIGHT ) {
			uWindowWidth = MIRROR_BACKBUFFER_WIDTH;
			uWindowHeight = MIRROR_BACKBUFFER_HEIGHT;
		}
		if( uWindowWidth < (UINT)nWidth || uWindowHeight < (UINT)nHeight ) {
			uWindowWidth = (UINT)nWidth;
			uWindowHeight = (UINT)nHeight;
		}

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


    ovrMirrorTextureDesc mirrorDesc = {};
    mirrorDesc.Format = OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
    mirrorDesc.Width = this->uMirrorWindowWidth;
    mirrorDesc.Height = this->uMirrorWindowHeight;
	mirrorDesc.MiscFlags = ovrTextureMisc_DX_Typeless;
	ovrResult result;
	result = ovr_CreateMirrorTextureDX(this->Session, this->pD3D11Device, &mirrorDesc, &(this->MirrorTexture));

	return result;
}

ovrResult OculusRiftDevice::CreateEyeRenderTexture(ovrSizei EyeTexSize[ovrEye_Count])
{
	ovrResult result;

	if( EyeTexSize[ovrEye_Left].w <= 0 || EyeTexSize[ovrEye_Left].h <= 0 ||
		EyeTexSize[ovrEye_Right].w <= 0 || EyeTexSize[ovrEye_Right].h <= 0
	)
		return ovrError_InvalidParameter;


	int eye;
	int i;

    for (eye = 0; eye < 2; eye++)
    {
		ovrTextureSwapChainDesc dsDesc = {};
        dsDesc.Type				= ovrTexture_2D;
		dsDesc.ArraySize        = 1;
		dsDesc.Format           = OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
		dsDesc.Width            = EyeTexSize[eye].w;
		dsDesc.Height           = EyeTexSize[eye].h;
		dsDesc.MipLevels        = 1;
		dsDesc.SampleCount		= 1;
		dsDesc.MiscFlags        = ovrTextureMisc_DX_Typeless;
		dsDesc.BindFlags        = ovrTextureBind_DX_RenderTarget;
        dsDesc.StaticImage		= ovrFalse;
		result = ovr_CreateTextureSwapChainDX(this->Session, this->pD3D11Device, &dsDesc, &(this->EyeTextureSwapChain[eye]));
		if( !OVR_SUCCESS(result) )
			return result;

		int nTexCount = 0;
		ovr_GetTextureSwapChainLength(this->Session, this->EyeTextureSwapChain[eye], &nTexCount);

		this->pEyeTexture[eye] = (ID3D11Texture2D **)HeapAlloc(this->hHeap, 0, sizeof(ID3D11Texture2D *) * nTexCount);
		if( !this->pEyeTexture[eye] ) {
			return ovrError_MemoryAllocationFailure;
		}

		this->pD3D11RTVSet[eye] = (ID3D11RenderTargetView **)HeapAlloc(this->hHeap, 0, sizeof(ID3D11RenderTargetView *) * nTexCount);
		if( !this->pD3D11RTVSet[eye] ) {
			return ovrError_MemoryAllocationFailure;
		}

		HRESULT hr;

		D3D11_TEXTURE2D_DESC texdesc = {};
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		
		for (i = 0; i < nTexCount; ++i)
        {
			ovr_GetTextureSwapChainDesc(this->Session, this->EyeTextureSwapChain[eye], &dsDesc);
			ovr_GetTextureSwapChainBufferDX(this->Session, this->EyeTextureSwapChain[eye], i, IID_ID3D11Texture2D, (void **)&(this->pEyeTexture[eye][i]));
			hr = this->pD3D11Device->CreateRenderTargetView(this->pEyeTexture[eye][i], &rtvDesc, &(this->pD3D11RTVSet[eye][i]));
			if( FAILED(hr) )
				return ovrError_InvalidParameter;
        }

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = EyeTexSize[eye].w;
		desc.Height = EyeTexSize[eye].h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		this->pD3D11Device->CreateTexture2D(&desc, NULL, &(this->TexDepth[eye]));
		this->pD3D11Device->CreateDepthStencilView(this->TexDepth[eye], NULL, &(this->pD3D11DSV[eye]));

		this->eyeRenderViewport[eye].Pos.x = 0;
		this->eyeRenderViewport[eye].Pos.y = 0;
        this->eyeRenderViewport[eye].Size = EyeTexSize[eye];

		this->eyeRenderDesc[eye] = ovr_GetRenderDesc(this->Session, (ovrEyeType)eye, this->HmdDesc.DefaultEyeFov[eye]);
   }

	return ovrSuccess;
}

ovrResult OculusRiftDevice::BeginFrame(ovrPosef *ppEyeRenderPose)
{
	double dFrameTiming = ovr_GetPredictedDisplayTime(this->Session, 0);
	ovrTrackingState TrackingState = ovr_GetTrackingState(this->Session, dFrameTiming, ovrTrue);
	ovr_CalcEyePoses(TrackingState.HeadPose.ThePose, this->HmdToEyeOffset, this->EyeRenderPose);

	if( ppEyeRenderPose ) {
		ppEyeRenderPose[ovrEye_Left] = this->EyeRenderPose[ovrEye_Left];
		ppEyeRenderPose[ovrEye_Right] = this->EyeRenderPose[ovrEye_Right];
	}

	return ovrSuccess;
}

ovrResult OculusRiftDevice::BeginEyeRender(ovrEyeType eye, bool bClearRTAndDSV, const float fRTClearColor[4])
{
	int nTexIndex = 0;

	ovr_GetTextureSwapChainCurrentIndex(this->Session, this->EyeTextureSwapChain[eye], &nTexIndex);

	if( bClearRTAndDSV ) {
		this->pD3D11Context->OMSetRenderTargets(1, &(this->pD3D11RTVSet[eye][nTexIndex]), this->pD3D11DSV[eye]);
		this->pD3D11Context->ClearRenderTargetView((this->pD3D11RTVSet[eye][nTexIndex]), fRTClearColor);
		this->pD3D11Context->ClearDepthStencilView(this->pD3D11DSV[eye], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}

	ovrRecti vp(this->eyeRenderViewport[eye]);
	D3D11_VIEWPORT D3Dvp;
	D3Dvp.Width = (float)vp.Size.w;		D3Dvp.Height = (float)vp.Size.h;
	D3Dvp.MinDepth = 0;					D3Dvp.MaxDepth = 1;
	D3Dvp.TopLeftX = (float)vp.Pos.x;	D3Dvp.TopLeftY = (float)vp.Pos.y;
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
		ovr_CommitTextureSwapChain(this->Session, this->EyeTextureSwapChain[eye]);
		ld.ColorTexture[eye] = this->EyeTextureSwapChain[eye];
		ld.Viewport[eye]     = this->eyeRenderViewport[eye];
		ld.Fov[eye]          = this->HmdDesc.DefaultEyeFov[eye];
		ld.RenderPose[eye]   = this->EyeRenderPose[eye];
	}

	ovrLayerHeader* layers = &ld.Header;

	result = ovr_SubmitFrame(this->Session, 0, nullptr, &layers, 1);


	HRESULT hr;
	const float white[4] = { 255, 255, 255, 255 };
	if( this->hWndMirrorWindow && this->pMirrorSwapChain && this->MirrorTexture ) {

		this->pD3D11Context->OMSetRenderTargets(1, &this->pMirrorRTV, NULL);
		this->pD3D11Context->ClearRenderTargetView(this->pMirrorRTV, white);

		ID3D11Texture2D *pTexSrc = nullptr;
		ovr_GetMirrorTextureBufferDX(this->Session, this->MirrorTexture, IID_ID3D11Texture2D, (void **)&pTexSrc);
		this->pD3D11Context->CopyResource(this->pMirrorBackBuffer, pTexSrc);
		pTexSrc->Release();
		hr = this->pMirrorSwapChain->Present(0, 0);

	}

	return result;
}

XMMATRIX * OculusRiftDevice::GetProjectionMatrix(XMMATRIX xmProjection[ovrEye_Count])
{
	/* DX9版と同じく、こっちも XMMatrixPerspectiveOffCenterLH() で綺麗にできる？ */
	/* ovrMatrix4f_Projection に置換 */

	XMMATRIX matProj[ovrEye_Count];

	/* Setup projection matrix */
	float projXScale;
	float projXOffset;
	float projYScale;
	float projYOffset;

	if( !xmProjection )
		return nullptr;

	int i;
	/* zNear < 1.0f だとおかしくなる */
	for(i = 0; i < ovrEye_Count; i++ ) {
		matProj[i] = XMMatrixPerspectiveOffCenterLH(
			-this->eyeRenderDesc[i].Fov.LeftTan,
			this->eyeRenderDesc[i].Fov.RightTan,
			-this->eyeRenderDesc[i].Fov.DownTan,
			this->eyeRenderDesc[i].Fov.UpTan,
			zNear, zFar
		);
		matProj[i] = XMMatrixTranspose(matProj[i]);
		xmProjection[i] = matProj[i];
	}

#if 1	/* DX11 Transposeいらない */
	ovrMatrix4f ovrMat;
	for(i = 0; i < ovrEye_Count; i++ ) {
		ovrMat = ovrMatrix4f_Projection(this->eyeRenderDesc[i].Fov, zNear, zFar, ovrProjection_LeftHanded);
		xmProjection[i] = XMMATRIX(&ovrMat.M[0][0]);
	}
#endif

#if 0	/* Oculus SDK OVR_StereoProjection.cpp CreateProjection() を参考に作成 */
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

		xmProjection[i] = matProj[i];
	}
#endif

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
	int i;

	if( !matProjection )
		return nullptr;

	ovrMatrix4f ovrMat;
	for(i = 0; i < ovrEye_Count; i++ ) {
		ovrMat = ovrMatrix4f_Projection(this->eyeRenderDesc[i].Fov, zNear, zFar, ovrProjection_LeftHanded);
		matProjection[i] = D3DXMATRIX(&ovrMat.M[0][0]);
		D3DXMatrixTranspose(&matProjection[i], &matProjection[i]);
	}

	return matProjection;
}

D3DXMATRIX * OculusRiftDevice::GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count], float fZNear, float fZFar)
{
	int i;

	if( !matProjection )
		return nullptr;

	ovrMatrix4f ovrMat;
	for(i = 0; i < ovrEye_Count; i++ ) {
		ovrMat = ovrMatrix4f_Projection(this->eyeRenderDesc[i].Fov, fZNear, fZFar, ovrProjection_LeftHanded);
		matProjection[i] = D3DXMATRIX(&ovrMat.M[0][0]);
		D3DXMatrixTranspose(&matProjection[i], &matProjection[i]);
	}

	return matProjection;
}

D3DXMATRIX * OculusRiftDevice::GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count], double CustomFovOffset)
{
	int i;

	double fov_l,fov_r,fov_t,fov_b;
	double aspect;

	if( !matProjection )
		return nullptr;

	ovrFovPort customFov;
	ovrMatrix4f ovrMat;

	for(i = 0; i < ovrEye_Count; i++ ) {
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

		customFov.LeftTan = (float)tan(fov_l);
		customFov.RightTan = (float)tan(fov_r);
		customFov.UpTan = (float)tan(fov_t);
		customFov.DownTan = (float)tan(fov_b);

		ovrMat = ovrMatrix4f_Projection(customFov, zNear, zFar, ovrProjection_LeftHanded);
		matProjection[i] = D3DXMATRIX(&ovrMat.M[0][0]);
		D3DXMatrixTranspose(&matProjection[i], &matProjection[i]);
	}

	return matProjection;
}

D3DXMATRIX * OculusRiftDevice::GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count], double CustomFovOffset, float fZNear, float fZFar)
{
	int i;

	double fov_l,fov_r,fov_t,fov_b;
	double aspect;

	if( !matProjection )
		return nullptr;

	ovrFovPort customFov;
	ovrMatrix4f ovrMat;

	for(i = 0; i < ovrEye_Count; i++ ) {
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

		customFov.LeftTan = (float)tan(fov_l);
		customFov.RightTan = (float)tan(fov_r);
		customFov.UpTan = (float)tan(fov_t);
		customFov.DownTan = (float)tan(fov_b);

		ovrMat = ovrMatrix4f_Projection(customFov, fZNear, fZFar, ovrProjection_LeftHanded);
		matProjection[i] = D3DXMATRIX(&ovrMat.M[0][0]);
		D3DXMatrixTranspose(&matProjection[i], &matProjection[i]);
	}

	return matProjection;
}

D3DXMATRIX * OculusRiftDevice::GetViewMatrixFromLastPose(D3DXMATRIX matEyePositionAndRotation[ovrEye_Count])
{
	D3DXMATRIX matRotate,matTrans;
	D3DXQUATERNION q;
	const float fPosMul = 5.0f; /* 10がちょうどいい？ */

	if( !matEyePositionAndRotation )
		return nullptr;

	q = D3DXQUATERNION(
		this->EyeRenderPose[ovrEye_Left].Orientation.x, this->EyeRenderPose[ovrEye_Left].Orientation.y,
		-this->EyeRenderPose[ovrEye_Left].Orientation.z, this->EyeRenderPose[ovrEye_Left].Orientation.w);
	D3DXMatrixRotationQuaternion(&matRotate, &q);
	D3DXMatrixTranslation(
		&matTrans,
		-this->EyeRenderPose[ovrEye_Left].Position.x * fPosMul,
		-this->EyeRenderPose[ovrEye_Left].Position.y * fPosMul,
		this->EyeRenderPose[ovrEye_Left].Position.z * fPosMul);
	matEyePositionAndRotation[ovrEye_Left] = matTrans * matRotate;

	q = D3DXQUATERNION(
		this->EyeRenderPose[ovrEye_Left].Orientation.x, this->EyeRenderPose[ovrEye_Left].Orientation.y,
		-this->EyeRenderPose[ovrEye_Left].Orientation.z, this->EyeRenderPose[ovrEye_Left].Orientation.w);
	D3DXMatrixRotationQuaternion(&matRotate, &q);
	D3DXMatrixTranslation(
		&matTrans,
		-this->EyeRenderPose[ovrEye_Right].Position.x * fPosMul,
		-this->EyeRenderPose[ovrEye_Right].Position.y * fPosMul,
		this->EyeRenderPose[ovrEye_Right].Position.z * fPosMul);
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
	ovrDetectResult result;
	result = ovr_Detect(nTimeoutToDetectRiftInMiliseconds);

	if( ovrFalse == result.IsOculusServiceRunning || ovrFalse == result.IsOculusHMDConnected )
		return 0;

	return 1;
}

ovrResult OculusRift::CreateDevice(int nDeviceIndex, OculusRiftDevice **ppDevice)
{
	ovrResult result;
	ovrSession s;
	ovrGraphicsLuid luid;
	OculusRiftDevice *pDeviceReturn;

	/* SDK0.8以降 Device=0限定 */
	if( nDeviceIndex != 0 )
		return ovrError_InvalidParameter;

	if( !bLibOVRInitialized || nDeviceIndex < 0 || nDeviceIndex >= OculusRift_MaxDevices || !ppDevice )
		return ovrError_InvalidParameter;

	if( this->pDevices[nDeviceIndex] )
		return ovrError_InvalidParameter;

	result = ovr_Create(&s, &luid);
	if( ovrSuccess == result ) {
		try {
			pDeviceReturn = new OculusRiftDevice(nDeviceIndex, s, luid);
		}
		catch(bad_alloc) {
			ovr_Destroy(s);
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

