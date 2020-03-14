#include "stdafx.h"

#ifdef OVR_ENABLE

using namespace OVR;

int g_nOVRFPS;

/* Direct3D11 実装インターフェースポインタ */
static ID3D11Device *g_pd3dDevice = NULL;
static ID3D11DeviceContext *g_pImmediateContext = NULL;
static ID3D11Texture2D *g_pOVREyeTex[OVR_EYE_NUM];
static ID3D11ShaderResourceView *g_pEyeSRView[OVR_EYE_NUM];

ovrRecti g_EyeRenderViewport[OVR_EYE_NUM];

D3DXMATRIX g_matOVREyeProj[OVR_EYE_NUM];


OculusRift *g_pOculusRift;
OculusRiftDevice *g_pRift;

ovrPosef g_RiftLastPose[OVR_EYE_NUM];	/* ヘッドトラッキング位置 */


HRESULT OVRManager_Create()
{
	ovrResult or;

	g_pOculusRift = new OculusRift();
	if( !g_pOculusRift ) {
		return E_FAIL;
	}

	or = g_pOculusRift->Initialize();
	if( !OVR_SUCCESS(or) ) {
		return E_FAIL;
	}

	or = g_pOculusRift->CreateDevice(0, &g_pRift);
	if( !OVR_SUCCESS(or) || !g_pRift ) {
		return E_FAIL;
	}

	ovrSizei TexSize0,TexSize1;
	ovrSizei OvrRenderSize;

	g_pRift->GetEyeRenderTextureSize(&TexSize0, &TexSize1);

	OvrRenderSize.w = TexSize0.w + TexSize1.w;
	OvrRenderSize.h = max(TexSize0.h, TexSize1.h);

	// TODO ビューポートはライブラリ側から取得する構成にしたい
	g_EyeRenderViewport[0].Pos  = Vector2i(0,0);
	g_EyeRenderViewport[0].Size = Sizei(OvrRenderSize.w / 2, OvrRenderSize.h);
	g_EyeRenderViewport[1].Pos  = Vector2i(0,0); // 1枚テクスチャの場合 Vector2i((g_OvrRenderSize.w + 1) / 2, 0);
	g_EyeRenderViewport[1].Size = g_EyeRenderViewport[0].Size;

	return S_OK;
}

HRESULT OVRManager_Cleanup()
{
	if( g_pOculusRift ) {
		g_pOculusRift->DestoryDevice(g_pRift);
		g_pOculusRift->UnInitialize();
		delete g_pOculusRift;
		g_pOculusRift = nullptr;
	}
	else {
		E_FAIL;
	}

	return S_OK;
}

HRESULT OVRDistortion_D3D11Init()
{

    HRESULT hr = S_OK;
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;


    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    // sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;			// Oculus DK2 = 75Hz
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    sd.OutputWindow = g_hWnd;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// Todo: デバイスのサポートを調べること
	// ID3D11Device::CheckMultisampleQualityLevels
	sd.SampleDesc.Count = 1;	// 8
    //sd.SampleDesc.Quality = 4;
    sd.Windowed = TRUE;

	IDXGIAdapter *OVROutputAdapter = NULL;

#if 0 /* dxgi.lib が必要 */
	int i = 0;
	IDXGIFactory *pFactory;
	IDXGIAdapter *pAdapter,*pAdapterNew;
	DXGI_ADAPTER_DESC adpDesc;

	CreateDXGIFactory(IID_IDXGIFactory, (void **)&pFactory);

	hr = pFactory->EnumAdapters(i++, &pAdapter);
	pAdapterNew = pAdapter;
	do {
		if( SUCCEEDED(hr) && pAdapterNew ) {
			pAdapterNew->GetDesc(&adpDesc);
			OVROutputAdapter = pAdapterNew;
			break;
		}
		pAdapter = pAdapterNew;
		hr = pFactory->EnumAdapters(i, &pAdapterNew);
		if( SUCCEEDED(hr) ) {
			RELEASE(pAdapter);
		}
		i++;
	} while( SUCCEEDED( hr ) );

	RELEASE(pFactory);
#endif

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
		if( driverTypeIndex == 0 && OVROutputAdapter ) {
	        driverType = D3D_DRIVER_TYPE_UNKNOWN;
		}
		else {
	        driverType = driverTypes[driverTypeIndex];
		}
		hr = D3D11CreateDevice( OVROutputAdapter, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
											D3D11_SDK_VERSION, &g_pd3dDevice, &featureLevel, &g_pImmediateContext );

		// ミラー表示はライブラリ側でやってくれるのでSwapChainは不要
		// hr = pFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
        if( SUCCEEDED( hr ) )
            break;
    }

	RELEASE(OVROutputAdapter);

    if( FAILED( hr ) )
        return hr;

	if( g_pRift ) {
		ovrResult or;

		or = g_pRift->InitializeDirectX(g_pd3dDevice, g_pImmediateContext);
		if( !OVR_SUCCESS(or) ) {
			return E_FAIL;
		}

		or = g_pRift->CreateEyeRenderTexture();
		if( !OVR_SUCCESS(or) ) {
			return E_FAIL;
		}

		or = g_pRift->SetupMirrorWindow(&g_hWnd);	/* 今あるやつを使う */
	}
	else {
		return E_FAIL;
	}

	try {
		g_pProfiler = new RenderProfiler(g_pd3dDevice, g_pImmediateContext);
	}
	catch(bad_alloc) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT OVRDistortion_Create()
{
    HRESULT hr = S_OK;


#ifdef D3D9EX_ENABLE
	int i;

	if( !g_pd3dDevice || !g_pRift ) {
		return E_INVALIDARG;
	}

	for(i = 0; i < OVR_EYE_NUM; i++) {
		/* ここでDirectX9->DirectX11のリソース共有を接続する */
		hr = g_pd3dDevice->OpenSharedResource(g_hEyeTexShareHandle[i], IID_ID3D11Texture2D, (void **)&g_pOVREyeTex[i]);
		if(FAILED(hr)) {
			break;
		}
		hr = g_pd3dDevice->CreateShaderResourceView(g_pOVREyeTex[i], NULL, &g_pEyeSRView[i]);
		if(FAILED(hr)) {
			break;
		}
	}
	if(FAILED(hr)) {
		for( i = 0; i < OVR_EYE_NUM; i++ ) {
			RELEASE(g_pOVREyeTex[i]);
			RELEASE(g_pEyeSRView[i]);
		}
		return hr;
	}
#endif

	if( g_pRift ) {
		g_pRift->GetProjectionMatrix(&g_matOVREyeProj[0]);
	}
	else {
		return E_FAIL;
	}

	return S_OK;
}


HRESULT OVRDistortion_Render1()
{
	if( g_pRift ) {
		/* EyeRenderPoseはライブラリ側の管理に任せるので特にいらない。→やっぱりちょっと欲しい */
		g_pRift->BeginFrame(g_RiftLastPose);
	}
	else {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT OVRDistortion_Render2()
{
	static int nFrameCount = 0;
	static DWORD dwFPSStartTime = 0;
    const float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // red, green, blue, alpha

	ID3D11Texture2D *pTex;

	if( !g_pRift ) {
		return E_FAIL;
	}


	if( g_pProfiler )
		g_pProfiler->EnterCheckPoint(TEXT("OVRDistortion_Render2 / copy eye texture"));

	g_pRift->BeginEyeRender(ovrEye_Left, true, &ClearColor[0]);
	pTex = g_pRift->GetCurrentEyeTex(ovrEye_Left);
	g_pRift->GetD3D11Context()->CopyResource(pTex, g_pOVREyeTex[ovrEye_Left]);

	g_pRift->BeginEyeRender(ovrEye_Right, true, &ClearColor[0]);
	pTex = g_pRift->GetCurrentEyeTex(ovrEye_Right);
	g_pRift->GetD3D11Context()->CopyResource(pTex, g_pOVREyeTex[ovrEye_Right]);

	if( g_pProfiler )
		g_pProfiler->LeaveCheckPoint();

	if( g_pProfiler )
		g_pProfiler->EnterCheckPoint(TEXT("OVRDistortion_Render2 / call g_pRift->EndFrame()"));

	g_pRift->EndFrame();

	if( g_pProfiler )
		g_pProfiler->LeaveCheckPoint();

	// ovrHmd_EndFrame() が勝手にやってくれる
    // g_pSwapChain->Present( 0, 0 );

	DWORD dwTime = timeGetTime();
	DWORD dwTimeDiff = dwTime - dwFPSStartTime;
	if( dwTimeDiff > 1000 ) {
		g_nOVRFPS = nFrameCount;
		nFrameCount = 0;
		dwFPSStartTime = dwTime;
	}
	nFrameCount++;

	return S_OK;
}

HRESULT OVRDistortion_Cleanup()
{
	int i;

    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

	for( i = 0; i < OVR_EYE_NUM; i++ ) {
		RELEASE(g_pOVREyeTex[i]);
		RELEASE(g_pEyeSRView[i]);
	}

	if( g_pProfiler ) {
		delete g_pProfiler;
		g_pProfiler = nullptr;
	}

    RELEASE( g_pImmediateContext );
    RELEASE( g_pd3dDevice );

	return S_OK;
}


#endif
