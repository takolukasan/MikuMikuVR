#include "stdafx.h"

#ifdef OVR_ENABLE

using namespace OVR;
int g_nHMDDetected;
ovrHmd g_HMD;
Sizei g_OvrRenderSize;

int g_nOVRFPS;


D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;



ID3D11Texture2D *g_pOVREyeTex[OVR_EYE_NUM];
ID3D11ShaderResourceView *g_pEyeSRView[OVR_EYE_NUM];

ovrEyeRenderDesc g_EyeRenderDesc[OVR_EYE_NUM];
ovrRecti g_EyeRenderViewport[OVR_EYE_NUM];
ovrD3D11Texture g_OvrEyeTex[2];

D3DXMATRIX g_matOVREyeProj[OVR_EYE_NUM];


HANDLE hOVREyeViewThread = NULL;
BOOL bOVREyeViewContinue = TRUE;

static DWORD WINAPI OVRDistortion_EyeViewCaluculation(LPVOID lpParameter);


HRESULT OVRManager_Create()
{
	ovr_Initialize();

	g_nHMDDetected = ovrHmd_Detect();
	if( g_nHMDDetected >= 1 ) {
		// Change index 0...g_nHMDDetected-1 for render device
		g_HMD = ovrHmd_Create(0);
	}
	else {
		g_HMD = NULL;
	}

	if( !g_HMD ) {
		g_HMD = ovrHmd_CreateDebug(ovrHmd_DK2);
	}

	if( !g_HMD ) {
		return E_FAIL;
	}


	Sizei Tex0Size,Tex1Size;
	Tex0Size = ovrHmd_GetFovTextureSize(g_HMD, ovrEye_Left, g_HMD->DefaultEyeFov[0], 1.0f);
	Tex1Size = ovrHmd_GetFovTextureSize(g_HMD, ovrEye_Right, g_HMD->DefaultEyeFov[1], 1.0f);
	g_OvrRenderSize.w = Tex0Size.w + Tex1Size.w;
	g_OvrRenderSize.h = max(Tex0Size.h, Tex1Size.h);


	g_EyeRenderViewport[0].Pos  = Vector2i(0,0);
	g_EyeRenderViewport[0].Size = Sizei(g_OvrRenderSize.w / 2, g_OvrRenderSize.h);
	g_EyeRenderViewport[1].Pos  = Vector2i(0,0); // 1枚テクスチャの場合 Vector2i((g_OvrRenderSize.w + 1) / 2, 0);
	g_EyeRenderViewport[1].Size = g_EyeRenderViewport[0].Size;


	DWORD dwThreadId;
	hOVREyeViewThread = CreateThread(NULL, 0, OVRDistortion_EyeViewCaluculation, NULL, CREATE_SUSPENDED, &dwThreadId);

	return S_OK;
}

HRESULT OVRManager_Cleanup()
{
	if( hOVREyeViewThread ) {
		bOVREyeViewContinue = FALSE;
		ResumeThread(hOVREyeViewThread);
		WaitForSingleObject(hOVREyeViewThread, INFINITE); // 値は暫定
		CloseHandle(hOVREyeViewThread);
	}

	ovrHmd_Destroy(g_HMD);
	ovr_Shutdown();

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

	int width = g_HMD->Resolution.w;
	int height = g_HMD->Resolution.h;


    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;			// Oculus DK2 = 75Hz
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    sd.OutputWindow = g_hWndDistortion;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// Todo: デバイスのサポートを調べること
	// ID3D11Device::CheckMultisampleQualityLevels
	sd.SampleDesc.Count = 1;	// 8
    //sd.SampleDesc.Quality = 4;
    sd.Windowed = TRUE;

	IDXGIAdapter *OVROutputAdapter = NULL;

#if 0
#ifdef _DEBUG	/* dxgi.lib が必要 */
	int i = 0;
	IDXGIFactory *pFactory;
	IDXGIAdapter *pAdapter,*pAdapterNew;
	DXGI_ADAPTER_DESC adpDesc;

	CreateDXGIFactory(IID_IDXGIFactory, (void **)&pFactory);

	hr = pFactory->EnumAdapters(i++, &pAdapter);
	pAdapterNew = pAdapter;
	do {
		if( SUCCEEDED(hr) ) {
			pAdapterNew->GetDesc(&adpDesc);
			OVROutputAdapter = pAdapterNew;
			break;
		}
		pAdapter = pAdapterNew;
		hr = pFactory->EnumAdapters(i, &pAdapterNew);
		if( SUCCEEDED(hr) ) {
			pAdapter->Release();
		}
		if( i == 1 )
			break;
		i++;
	} while( SUCCEEDED( hr ) );

	pFactory->Release();
#endif
#endif

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
		if( driverTypeIndex == 0 && OVROutputAdapter ) {
	        g_driverType = D3D_DRIVER_TYPE_UNKNOWN;
		}
		else {
	        g_driverType = driverTypes[driverTypeIndex];
		}
		// Swapchain を別で作るように変更予定(@GPU変更) → リソース共有できない。
		// D3D11CreateDevice と別々に呼ばないとダメ？
        hr = D3D11CreateDeviceAndSwapChain( OVROutputAdapter, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

        // hr = D3D11CreateDevice( OVROutputAdapter, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
        //                                     D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );

		// hr = pFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );


	return S_OK;
}

HRESULT OVRDistortion_Create()
{
    HRESULT hr = S_OK;
	int width = g_HMD->Resolution.w;
	int height = g_HMD->Resolution.h;


#ifdef D3D9EX_ENABLE
	int i;

	if( !g_pd3dDevice ) {
		return E_INVALIDARG;
	}

	/* Todo 失敗対策がない */
	for(i = 0; i < OVR_EYE_NUM; i++) {
		hr = g_pd3dDevice->OpenSharedResource(g_hEyeTexShareHandle[i], IID_ID3D11Texture2D, (void **)&g_pOVREyeTex[i]);
		hr = g_pd3dDevice->CreateShaderResourceView(g_pOVREyeTex[i], NULL, &g_pEyeSRView[i]);
	}

#endif


	// Attach HMD to Window
	ovrBool ob;
	ob = ovrHmd_AttachToWindow(g_HMD, g_hWndDistortion, NULL, NULL);

	ovrD3D11Config conf;

	ZeroMemory(&conf, sizeof(conf));
	conf.D3D11.Header.API         = ovrRenderAPI_D3D11;
	conf.D3D11.Header.BackBufferSize = Sizei(width, height);
	conf.D3D11.Header.Multisample = 1;	/* これやるときれいになる？ */
	conf.D3D11.pDevice            = g_pd3dDevice;
	conf.D3D11.pDeviceContext     = g_pImmediateContext;
	conf.D3D11.pBackBufferRT      = g_pRenderTargetView;
	conf.D3D11.pSwapChain         = g_pSwapChain;

	ovrFovPort eyeFov[2];
	eyeFov[0] = g_HMD->DefaultEyeFov[0];
	eyeFov[1] = g_HMD->DefaultEyeFov[1];

#ifndef _DEBUG
	ob = ovrHmd_ConfigureRendering(g_HMD, &conf.Config,
		ovrDistortionCap_Vignette | ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive | ovrDistortionCap_HqDistortion,
		eyeFov, g_EyeRenderDesc);
#else
	ob = ovrHmd_ConfigureRendering(g_HMD, &conf.Config,
		ovrDistortionCap_Vignette | ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
		eyeFov, g_EyeRenderDesc);
#endif
	if( !ob ) {
		return E_FAIL;
	}

	ovrHmd_SetEnabledCaps(g_HMD, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction | ovrHmdCap_NoMirrorToWindow);
	// Start the sensor which informs of the Rift's pose and motion
    ovrHmd_ConfigureTracking(g_HMD, ovrTrackingCap_Orientation |
                                    ovrTrackingCap_MagYawCorrection |
                                    ovrTrackingCap_Position, 0);

	ovrHmd_DismissHSWDisplay(g_HMD);


	g_OvrEyeTex[0].D3D11.Header.API = ovrRenderAPI_D3D11;
	g_OvrEyeTex[0].D3D11.Header.TextureSize = g_EyeRenderViewport[0].Size;
	g_OvrEyeTex[0].D3D11.Header.RenderViewport = g_EyeRenderViewport[0];
	g_OvrEyeTex[0].D3D11.pTexture = g_pOVREyeTex[0];
	g_OvrEyeTex[0].D3D11.pSRView = g_pEyeSRView[0];

	g_OvrEyeTex[1] = g_OvrEyeTex[0];
	g_OvrEyeTex[1].D3D11.Header.TextureSize = g_EyeRenderViewport[1].Size;
	g_OvrEyeTex[1].D3D11.Header.RenderViewport = g_EyeRenderViewport[1];
	g_OvrEyeTex[1].D3D11.pTexture = g_pOVREyeTex[1];
	g_OvrEyeTex[1].D3D11.pSRView = g_pEyeSRView[1];


	/* Setup projection matrix */
	float projXScale;
	float projXOffset;
	float projYScale;
	float projYOffset;
	float zNear = 0.2f;
	float zFar = 1000.0f;

	for(i = 0; i < OVR_EYE_NUM; i++ ) {
		projXScale = 2.0f / ( g_EyeRenderDesc[i].Fov.LeftTan + g_EyeRenderDesc[i].Fov.RightTan );
		projXOffset = ( g_EyeRenderDesc[i].Fov.LeftTan - g_EyeRenderDesc[i].Fov.RightTan ) * projXScale * 0.5f;
		projYScale = 2.0f / ( g_EyeRenderDesc[i].Fov.UpTan + g_EyeRenderDesc[i].Fov.DownTan );
		projYOffset = ( g_EyeRenderDesc[i].Fov.UpTan - g_EyeRenderDesc[i].Fov.DownTan ) * projYScale * 0.5f;

		// Produces X result, mapping clip edges to [-w,+w]
		g_matOVREyeProj[i].m[0][0] = projXScale;
		g_matOVREyeProj[i].m[0][1] = 0.0f;
		g_matOVREyeProj[i].m[0][2] = projXOffset;
		g_matOVREyeProj[i].m[0][3] = 0.0f;

		// Produces Y result, mapping clip edges to [-w,+w]
		// Hey - why is that YOffset negated?
		// It's because a projection matrix transforms from world coords with Y=up,
		// whereas this is derived from an NDC scaling, which is Y=down.
		g_matOVREyeProj[i].m[1][0] = 0.0f;
		g_matOVREyeProj[i].m[1][1] = projYScale;
		g_matOVREyeProj[i].m[1][2] = -projYOffset;
		g_matOVREyeProj[i].m[1][3] = 0.0f;

		// Produces Z-buffer result - app needs to fill this in with whatever Z range it wants.
		// We'll just use some defaults for now.
		g_matOVREyeProj[i].m[2][0] = 0.0f;
		g_matOVREyeProj[i].m[2][1] = 0.0f;
		g_matOVREyeProj[i].m[2][2] = -1.0f * zFar / (zNear - zFar);
		g_matOVREyeProj[i].m[2][3] = (zFar * zNear) / (zNear - zFar);

		// Produces W result (= Z in)
		g_matOVREyeProj[i].m[3][0] = 0.0f;
		g_matOVREyeProj[i].m[3][1] = 0.0f;
		g_matOVREyeProj[i].m[3][2] = 1.0f;
		g_matOVREyeProj[i].m[3][3] = 0.0f;

		D3DXMatrixTranspose(&g_matOVREyeProj[i], &g_matOVREyeProj[i]);	// 冗長というか↑加工すれば消せるけどまぁいいや
	}

	ResumeThread(hOVREyeViewThread);
#ifndef _DEBUG
	SetThreadPriority(hOVREyeViewThread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

	return S_OK;
}


D3DXVECTOR3 * Transform(D3DXVECTOR3 *vecOut, D3DXMATRIX *matIn, D3DXVECTOR3 *v)
{
	const float W = 1.0f / (matIn->m[3][0] * v->x + matIn->m[3][1] * v->y + matIn->m[3][2] * v->z + matIn->m[3][3]);

	vecOut->x = (matIn->m[0][0] * v->x + matIn->m[0][1] * v->y + matIn->m[0][2] * v->z + matIn->m[0][3]) * W;
	vecOut->y = (matIn->m[1][0] * v->x + matIn->m[1][1] * v->y + matIn->m[1][2] * v->z + matIn->m[1][3]) * W;
	vecOut->z = (matIn->m[2][0] * v->x + matIn->m[2][1] * v->y + matIn->m[2][2] * v->z + matIn->m[2][3]) * W;

	return vecOut;
}

D3DXMATRIX matEyeView[OVR_EYE_NUM];
BOOL bSync = FALSE;

ovrPosef EyeRenderPose[OVR_EYE_NUM];

HRESULT OVRDistortion_Render1()
{
	// OVR
	ovrHmd_BeginFrame(g_HMD, 0);

	ovrTrackingState hmdState;
	ovrVector3f hmdToEyeViewOffset[OVR_EYE_NUM] = { g_EyeRenderDesc[0].HmdToEyeViewOffset, g_EyeRenderDesc[1].HmdToEyeViewOffset };
	ovrHmd_GetEyePoses(g_HMD, 0, hmdToEyeViewOffset, EyeRenderPose, &hmdState);

#if 0
	D3DXMATRIX matRotate,matTrans,mat,matUp;
	D3DXQUATERNION q;
	D3DXVECTOR3 vecEye,vecAt,vecUp;
	q.x = EyeRenderPose[OVR_EYE_LEFT].Orientation.x;
	q.y = EyeRenderPose[OVR_EYE_LEFT].Orientation.y;
	q.z = -EyeRenderPose[OVR_EYE_LEFT].Orientation.z;
	q.w = EyeRenderPose[OVR_EYE_LEFT].Orientation.w;
	D3DXMatrixRotationQuaternion(&matRotate, &q);
	D3DXMatrixTranslation(&matTrans, -EyeRenderPose[OVR_EYE_LEFT].Position.x * 10, -EyeRenderPose[OVR_EYE_LEFT].Position.y * 10, EyeRenderPose[OVR_EYE_LEFT].Position.z * 10);
	matEyeView[OVR_EYE_LEFT] = matRotate * matTrans;

#if 0
	XMVECTOR XMVecQ = XMVectorSet(
		EyeRenderPose[OVR_EYE_LEFT].Orientation.x, EyeRenderPose[OVR_EYE_LEFT].Orientation.y,
		-EyeRenderPose[OVR_EYE_LEFT].Orientation.z, EyeRenderPose[OVR_EYE_LEFT].Orientation.w);
	XMMATRIX xmmatTrans,xmmat;
	xmmat = XMMatrixRotationQuaternion(XMVecQ);
	xmmatTrans = XMMatrixTranslation(EyeRenderPose[OVR_EYE_LEFT].Position.x, EyeRenderPose[OVR_EYE_LEFT].Position.y, EyeRenderPose[OVR_EYE_LEFT].Position.z);
	xmmat *= xmmatTrans;
	matEyeView[OVR_EYE_LEFT] = D3DXMATRIX(&xmmatTrans.m[0][0]);
#endif
#if 0
	D3DXVECTOR3 vec1 = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 vec2 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 vecBasePos = D3DXVECTOR3(0.0f, 10.0f, -10.0f);
	Transform(&vecUp, &matRotate, &vec1);
	vecEye = vecBasePos + D3DXVECTOR3(EyeRenderPose[OVR_EYE_LEFT].Position.x, EyeRenderPose[OVR_EYE_LEFT].Position.y, EyeRenderPose[OVR_EYE_LEFT].Position.z);
	Transform(&vecAt, &matRotate, &vec2);
	vecAt += vecEye;
	D3DXMatrixLookAtLH(&matEyeView[OVR_EYE_LEFT], &vecEye, &vecAt, &vecUp);
#endif

	q.x = EyeRenderPose[OVR_EYE_RIGHT].Orientation.x;
	q.y = EyeRenderPose[OVR_EYE_RIGHT].Orientation.y;
	q.z = -EyeRenderPose[OVR_EYE_RIGHT].Orientation.z;
	q.w = EyeRenderPose[OVR_EYE_RIGHT].Orientation.w;
	D3DXMatrixRotationQuaternion(&matRotate, &q);
	D3DXMatrixTranslation(&matTrans, -EyeRenderPose[OVR_EYE_RIGHT].Position.x * 10, -EyeRenderPose[OVR_EYE_RIGHT].Position.y * 10, EyeRenderPose[OVR_EYE_RIGHT].Position.z * 10);
	matEyeView[OVR_EYE_RIGHT] = matRotate * matTrans;

#if 0
	Transform(&vecUp, &matRotate, &vec1);
	vecEye = vecBasePos + D3DXVECTOR3(EyeRenderPose[OVR_EYE_RIGHT].Position.x, EyeRenderPose[OVR_EYE_RIGHT].Position.y, EyeRenderPose[OVR_EYE_RIGHT].Position.z);
	Transform(&vecAt, &matRotate, &vec2);
	vecAt += vecEye;
	D3DXMatrixLookAtLH(&matEyeView[OVR_EYE_RIGHT], &vecEye, &vecAt, &vecUp);
#endif

	bSync = TRUE;
#endif

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	return S_OK;
}

HRESULT OVRDistortion_Render2()
{
	static int nFrameCount = 0;
	static DWORD dwFPSStartTime = 0;

	ovrTexture EyeTexture[2];
	ZeroMemory(EyeTexture, sizeof(EyeTexture));
	EyeTexture[0] = g_OvrEyeTex[0].Texture;
	EyeTexture[1] = g_OvrEyeTex[1].Texture;
	ovrHmd_EndFrame(g_HMD, EyeRenderPose, EyeTexture);


	//
    // Present our back buffer to our front buffer
    //
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

static DWORD WINAPI OVRDistortion_EyeViewCaluculation(LPVOID lpParameter)
{
	ovrPosef EyeRenderPose[OVR_EYE_NUM];
	ovrTrackingState hmdState;
	ovrVector3f hmdToEyeViewOffset[OVR_EYE_NUM] = { g_EyeRenderDesc[0].HmdToEyeViewOffset, g_EyeRenderDesc[1].HmdToEyeViewOffset };

	while( bOVREyeViewContinue ) {
#if 1
		ovrHmd_GetEyePoses(g_HMD, 0, hmdToEyeViewOffset, EyeRenderPose, &hmdState);

		D3DXMATRIX matRotate,matTrans,mat,matUp;
		D3DXVECTOR3 vec1 = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVECTOR3 vec2 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		D3DXQUATERNION q;
		D3DXVECTOR3 vecEye,vecAt,vecUp;
		D3DXVECTOR3 vecBasePos = D3DXVECTOR3(0.0f, 10.0f, -10.0f);
		q.x = EyeRenderPose[OVR_EYE_LEFT].Orientation.x;
		q.y = EyeRenderPose[OVR_EYE_LEFT].Orientation.y;
		q.z = -EyeRenderPose[OVR_EYE_LEFT].Orientation.z;
		q.w = EyeRenderPose[OVR_EYE_LEFT].Orientation.w;
		D3DXMatrixRotationQuaternion(&matRotate, &q);
		D3DXMatrixTranslation(&matTrans, -EyeRenderPose[OVR_EYE_LEFT].Position.x * 10, -EyeRenderPose[OVR_EYE_LEFT].Position.y * 10, EyeRenderPose[OVR_EYE_LEFT].Position.z * 10);
		matEyeView[OVR_EYE_LEFT] = matTrans * matRotate;

#if 0
		XMVECTOR XMVecQ = XMVectorSet(
			EyeRenderPose[OVR_EYE_LEFT].Orientation.x, EyeRenderPose[OVR_EYE_LEFT].Orientation.y,
			-EyeRenderPose[OVR_EYE_LEFT].Orientation.z, EyeRenderPose[OVR_EYE_LEFT].Orientation.w);
		XMMATRIX xmRotate,xmTrans,xmmat;
		xmRotate = XMMatrixRotationQuaternion(XMVecQ);
		xmTrans = XMMatrixTranslation(-EyeRenderPose[OVR_EYE_LEFT].Position.x * 10, -EyeRenderPose[OVR_EYE_LEFT].Position.y * 10, EyeRenderPose[OVR_EYE_LEFT].Position.z * 10);
		xmmat = xmTrans * xmRotate;
		matEyeView[OVR_EYE_LEFT] = D3DXMATRIX(&xmmat.m[0][0]);
#endif
#if 0
		Transform(&vecUp, &matRotate, &vec1);
		vecEye = vecBasePos + D3DXVECTOR3(EyeRenderPose[OVR_EYE_LEFT].Position.x, EyeRenderPose[OVR_EYE_LEFT].Position.y, EyeRenderPose[OVR_EYE_LEFT].Position.z);
		Transform(&vecAt, &matRotate, &vec2);
		vecAt += vecEye;
		D3DXMatrixLookAtLH(&matEyeView[OVR_EYE_LEFT], &vecEye, &vecAt, &vecUp);
#endif

		q.x = EyeRenderPose[OVR_EYE_RIGHT].Orientation.x;
		q.y = EyeRenderPose[OVR_EYE_RIGHT].Orientation.y;
		q.z = -EyeRenderPose[OVR_EYE_RIGHT].Orientation.z;
		q.w = EyeRenderPose[OVR_EYE_RIGHT].Orientation.w;
		D3DXMatrixRotationQuaternion(&matRotate, &q);
		D3DXMatrixTranslation(&matTrans, -EyeRenderPose[OVR_EYE_RIGHT].Position.x * 10, -EyeRenderPose[OVR_EYE_RIGHT].Position.y * 10, EyeRenderPose[OVR_EYE_RIGHT].Position.z * 10);
		matEyeView[OVR_EYE_RIGHT] = matTrans * matRotate;

#if 0
		Transform(&vecUp, &matRotate, &vec1);
		vecEye = vecBasePos + D3DXVECTOR3(EyeRenderPose[OVR_EYE_RIGHT].Position.x, EyeRenderPose[OVR_EYE_RIGHT].Position.y, EyeRenderPose[OVR_EYE_RIGHT].Position.z);
		Transform(&vecAt, &matRotate, &vec2);
		vecAt += vecEye;
		D3DXMatrixLookAtLH(&matEyeView[OVR_EYE_RIGHT], &vecEye, &vecAt, &vecUp);
#endif

		bSync = TRUE;

#endif


		Sleep(1);
	}
	
	return 0;
}

HRESULT OVRDistortion_Cleanup()
{
	int i;

    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

	for( i = 0; i < OVR_EYE_NUM; i++ ) {
		RELEASE(g_pOVREyeTex[i]);
		RELEASE(g_pEyeSRView[i]);
	}

    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();

	return S_OK;
}


#endif
