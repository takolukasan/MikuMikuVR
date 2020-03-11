// d3d9_hook.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

// スレッドと同期用オブジェクト
HANDLE g_hMainProcThread;
DWORD g_dwMainProcThreadId;
HANDLE g_hMutexD3DCreateBlock;
HANDLE g_hSemaphoreMMDInitBlock;
HANDLE g_hSemaphoreMMDShutdownBlock;
#ifdef OVR_ENABLE
HANDLE g_hSemaphoreOVRRenderSync;
HANDLE g_hSemaphoreMMDRenderSync;
#endif



VIEW_POINTS g_ViewPoint = VIEW_POINT_ORIGINAL;


/* Direct3D9 実装インターフェースポインタ */
CHookIDirect3D9MMD *pHookDirect3D9;
CHookIDirect3DDevice9MMD *pHookDirect3DDevice9;
CHookID3DXEffectMMD *pHookEffect;

/* Direct3D9 インターフェース */
IDirect3DSurface9 *g_pPrimaryBackBuffer;
IDirect3DSurface9 *g_pPrimaryDepthStencil;
IDirect3DSwapChain9 *g_pMirSwapChain;
IDirect3DSurface9 *g_pMirBackBuffer;
IDirect3DSurface9 *g_pMirDepthStencil;

#ifdef OVR_ENABLE
IDirect3DTexture9 *g_pEyeTex[OVR_EYE_NUM];
IDirect3DSurface9 *g_pEyeSurf[OVR_EYE_NUM];
IDirect3DSurface9 *g_pEyeDepth[OVR_EYE_NUM];
#endif

D3DVIEWPORT9 g_vpMirror;


D3DXMATRIX g_matRotateViewPoints[VIEW_POINT_SELECT_MAX];	/* 使用するインデックスはg_ViewPoint に同期 */


/* Direct3D共有リソース用ハンドル */
#ifdef D3D9EX_ENABLE
#ifdef OVR_ENABLE
HANDLE g_hEyeTexShareHandle[OVR_EYE_NUM];
#endif
#endif

#define RT_Swap() if( g_pMirBackBuffer && g_pMirDepthStencil) {		\
	D3DVIEWPORT9 vp;												\
	IDirect3DSurface9 *pSurf,*pDepth;								\
	this->pOriginal->GetRenderTarget(0, &pSurf);					\
	this->pOriginal->GetViewport(&vp);								\
	this->pOriginal->GetDepthStencilSurface(&pDepth);				\
	this->pOriginal->SetRenderTarget(0, g_pMirBackBuffer);			\
	this->pOriginal->SetViewport(&vp);								\
	this->pOriginal->SetDepthStencilSurface(g_pMirDepthStencil);	

#define RT_Restore()												\
	this->pOriginal->SetRenderTarget(0, pSurf);						\
	this->pOriginal->SetViewport(&vp);								\
	this->pOriginal->SetDepthStencilSurface(pDepth);				\
	pSurf->Release();												\
	pDepth->Release();												\
}


static void CleanupD3DHookResources();


#if 0
D3DXMATRIX * GetViewPointMatrix(D3DXMATRIX *matOut, const VIEW_POINTS vPoint, const D3DXMATRIX *matOld)
{
	switch(vPoint) {
		case VIEW_POINT_LEFT:
		case VIEW_POINT_RIGHT:
		case VIEW_POINT_TOP:
		case VIEW_POINT_BOTTOM:
		case VIEW_POINT_OPPOSITE:
		case VIEW_POINT_ORIGINAL:
			D3DXMatrixMultiply(matOut, matOld, &g_matRotateViewPoints[vPoint]);
			break;

		case VIEW_POINT_FIX:
			/* 行列変更しない */
			break;

		default:
			/* バカよけ */
		break;
	}

	return matOut;
}
#endif


DWORD WINAPI MainProc(LPVOID lpParameter)
{
#ifdef OVR_ENABLE
	BOOL bOVRInitialized = FALSE;
#endif

	/* DllMain() でのインジェクション成功判定が必要 */
	if( !g_bMMDHacked ) {
		ExitThread(0xffffffff);
	}

	g_hMutexD3DCreateBlock = CreateMutex(NULL, TRUE, NULL);
	g_hSemaphoreMMDInitBlock = CreateSemaphore(NULL, 0, 1, NULL);
	g_hSemaphoreMMDShutdownBlock = CreateSemaphore(NULL, 0, 1, NULL);

#ifdef OVR_ENABLE
	g_hSemaphoreOVRRenderSync = CreateSemaphore(NULL, 0, 1, NULL);
	g_hSemaphoreMMDRenderSync = CreateSemaphore(NULL, 0, 1, NULL);
#endif

	if( FAILED(InitWindow(g_hModMyLibrary, SW_SHOW)) ) {
		ExitThread(0xffffffff);
	}

	// Wait for call Direct3DCreate9() @ MMD
	SuspendThread(g_hMainProcThread);
#ifndef _DEBUG
	SetThreadPriority(g_hMainProcThread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif
	CloseHandle(g_hMainProcThread);
	g_hMainProcThread = NULL;

	// これが終わるまでIDirect3D9::CreateDevice()、というよりOVR用バックバッファ作成をブロックさせる必要がある
#ifdef OVR_ENABLE
	if(SUCCEEDED(OVRManager_Create()) ) {
		bOVRInitialized = TRUE;
	}
	else {
		bOVRInitialized = FALSE;
	}
#endif
	ReleaseMutex(g_hMutexD3DCreateBlock);

#ifdef OVR_ENABLE
	if( SUCCEEDED(OVRDistortion_D3D11Init()) ) {
		bOVRInitialized = TRUE;
	}
	else {
		bOVRInitialized = FALSE;
	}
#endif

	// 逆にここはMMD側のDirect3D初期化が終わるまでブロック
	WaitForSingleObject(g_hSemaphoreMMDInitBlock, INFINITE);
#ifdef OVR_ENABLE
	if( bOVRInitialized ) {
		if( SUCCEEDED(OVRDistortion_Create()) ) {
			bOVRInitialized = TRUE;
		}
		else {
			bOVRInitialized = FALSE;
		}
	}
#endif

	SetupWindowState();

    ShowWindow( g_hWnd, SW_SHOW );

#ifdef OVR_ENABLE
    ShowWindow( g_hWndDistortion, SW_SHOW );
#endif

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
#ifdef OVR_ENABLE
			ReleaseSemaphore(g_hSemaphoreMMDRenderSync, 1, NULL);
			if( bOVRInitialized && g_pMMEHookMirrorRT && WAIT_OBJECT_0 == WaitForSingleObject(g_hSemaphoreOVRRenderSync, 30) ) {	/* MMD側描画完了 */
				OVRDistortion_Render();
			}
			static int nFPSold = 0;
			if( nFPSold != g_nOVRFPS ) {
				TCHAR tcFPSBuffer[MAX_PATH];
				swprintf_s(tcFPSBuffer, MAX_PATH, L"FPS: %d\n", g_nOVRFPS);
				SetOVRWindowTitleSuffix(tcFPSBuffer);
				nFPSold = g_nOVRFPS;
			}
#else
			Sleep(0);
#endif
        }
    }

#ifdef OVR_ENABLE
	OVRDistortion_Cleanup();
	OVRManager_Cleanup();
#endif
	CleanupD3DHookResources();

#ifdef OVR_ENABLE
	CloseHandle(g_hSemaphoreOVRRenderSync);
	CloseHandle(g_hSemaphoreMMDRenderSync);
#endif

	CloseHandle(g_hSemaphoreMMDInitBlock);
	CloseHandle(g_hMutexD3DCreateBlock);

	ReleaseSemaphore(g_hSemaphoreMMDShutdownBlock, 1, NULL);

	Sleep(100);

	CloseHandle(g_hSemaphoreMMDShutdownBlock);

	/* 運がよければ開放される・・・ */
	if( pHookDirect3D9 && pHookDirect3D9->GetRefCnt() == 1 ) {
		delete pHookDirect3D9;
		pHookDirect3D9 = NULL;
	}

	return 0;
}

static void CleanupD3DHookResources()
{
	RELEASE(g_pPrimaryBackBuffer);
	RELEASE(g_pPrimaryDepthStencil);
	RELEASE(g_pMirSwapChain);
	RELEASE(g_pMirBackBuffer);
	RELEASE(g_pMirDepthStencil);
}

CHookIDirect3D9MMD::CHookIDirect3D9MMD(::IDirect3D9 *pD3D)
{
	if( pD3D ) {
		this->pOriginal = pD3D;
		this->AddRef();
	}
}

CHookIDirect3D9MMD::~CHookIDirect3D9MMD()	
{
	/* pOriginal->Release() は CHookIDirect3D9::~CHookIDirect3D9 で実施 */
}

// CHookIDirect3D9MMD::IUnknown
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9MMD::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_IDirect3D9 ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

ULONG STDMETHODCALLTYPE CHookIDirect3D9MMD::Release()
{
	/* ここまでで全部開放されてたらクリーンできる */
	if( pHookEffect && pHookEffect->GetRefCnt() == 1 ) {
		delete (pHookEffect);
		pHookEffect = NULL;
	}

	if( pHookDirect3DDevice9 && pHookDirect3DDevice9->GetRefCnt() == 1 ) {
		delete (pHookDirect3DDevice9);
		pHookDirect3DDevice9 = NULL;
	}

	/* 親クラスのRelease()を呼ばないと再帰して死ぬ */
	return this->CHookIDirect3D9::Release();
}

// IDirect3D9
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9MMD::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
	IDirect3DDevice9 *p = NULL;
	HRESULT hr = S_OK;

	HackMMDWindow(hFocusWindow);

	// pPresentationParameters->MultiSampleType = D3DMULTISAMPLE_NONE;
	// pPresentationParameters->MultiSampleQuality = 0; 

	// 付けておいたほうが安全？
	BehaviorFlags |= D3DCREATE_MULTITHREADED;

#ifdef _DEBUG
	// Adapter = 1;
#endif
	hr = this->pOriginal->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &p);

	if( SUCCEEDED(hr) ) {

#ifdef D3D9EX_ENABLE
		IDirect3DDevice9Ex *pDevice9Ex;
		int nPriority = 0;
		if( SUCCEEDED( p->QueryInterface(IID_IDirect3DDevice9Ex, (void **)&pDevice9Ex) ) ) {
			pDevice9Ex->GetGPUThreadPriority(&nPriority);
			pDevice9Ex->SetGPUThreadPriority(nPriority + 1);
			pDevice9Ex->SetMaximumFrameLatency(1);
			pDevice9Ex->Release();
		}
#endif

		pHookDirect3DDevice9 = new CHookIDirect3DDevice9MMD(p);
		// 面倒でもQueryInterfaceを呼ぶべき！
		if( FAILED(pHookDirect3DDevice9->QueryInterface(IID_IDirect3DDevice9, (void **)ppReturnedDeviceInterface) ) ) {
			*ppReturnedDeviceInterface = p;
			delete pHookDirect3DDevice9;
			pHookDirect3DDevice9 = NULL;
		}
		else {
			p->Release(); /* コレをやるべきなのか？ */
		}

		HRESULT hr1;
		hr1 = p->GetRenderTarget(0, &g_pPrimaryBackBuffer);
		hr1 = p->GetDepthStencilSurface(&g_pPrimaryDepthStencil);

		D3DPRESENT_PARAMETERS pr = *pPresentationParameters;
		pr.hDeviceWindow = g_hWnd;
		// pr.BackBufferCount = 1;

		// Wait for OVR initialize...
		WaitForSingleObject(g_hMutexD3DCreateBlock, INFINITE);

		D3DSURFACE_DESC SurfDesc[2];
		ZeroMemory(SurfDesc, sizeof(SurfDesc));
		hr1 = g_pPrimaryBackBuffer->GetDesc(&SurfDesc[0]);
		hr1 = g_pPrimaryDepthStencil->GetDesc(&SurfDesc[1]);

#ifdef MIRROR_RENDER
		if( SUCCEEDED(p->CreateAdditionalSwapChain(&pr, &g_pMirSwapChain) ) ) {
			hr1 = g_pMirSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &g_pMirBackBuffer);

			hr1 = p->CreateDepthStencilSurface(
				SurfDesc[1].Width, SurfDesc[1].Height, SurfDesc[1].Format, SurfDesc[1].MultiSampleType, SurfDesc[1].MultiSampleQuality,
				FALSE, &g_pMirDepthStencil, NULL);
		}
#endif
#ifdef OVR_ENABLE
		int i;
		for(i = 0; i < OVR_EYE_NUM; i++)
		{
			// Sizei idealSize = ovrHmd_GetFovTextureSize(g_HMD, (ovrEyeType)i, g_HMD->DefaultEyeFov[i], 1.0f);
			hr1 = p->CreateTexture(g_EyeRenderViewport[i].Size.w, g_EyeRenderViewport[i].Size.h, 1, D3DUSAGE_RENDERTARGET,
				SurfDesc[0].Format, D3DPOOL_DEFAULT, &g_pEyeTex[i], &g_hEyeTexShareHandle[i]);
			hr1 = g_pEyeTex[i]->GetSurfaceLevel(0, &g_pEyeSurf[i]);
			hr1 = p->CreateDepthStencilSurface(g_EyeRenderViewport[i].Size.w, g_EyeRenderViewport[i].Size.h,
				SurfDesc[1].Format, D3DMULTISAMPLE_NONE, 0,
				FALSE, &g_pEyeDepth[i], NULL);
		}
#endif

#if 0
		ID3DXMesh *mesh;
		ID3DXBuffer *buffer;
		D3DXMATERIAL material;
		D3DCOLORVALUE white = { 1.0f, 1.0f, 1.0f, 1.0f };
		D3DCOLORVALUE yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
		D3DCOLORVALUE red = { 1.0f, 0.0f, 0.0f, 1.0f };
		D3DCOLORVALUE green = { 0.0f, 1.0f, 0.0f, 1.0f };
		D3DCOLORVALUE blue = { 0.0f, 0.0f, 1.0f, 1.0f };
		D3DCOLORVALUE black = { 0.0f, 0.0f, 0.0f, 1.0f };
		D3DCOLORVALUE pink = { 1.0f, 0.67f, 0.67f, 1.0f };

		// D3DXCreateCylinder(p, 1.0f, 1.2f, 2.0f, 10, 10, &mesh, &buffer);
		D3DXCreateSphere(p, 0.05f, 10, 10, &mesh, &buffer);


		material.MatD3D.Ambient = white;
		material.MatD3D.Diffuse = white;
		material.MatD3D.Specular = white;
		material.MatD3D.Emissive = white;
		material.MatD3D.Power = 0.0f;
		material.pTextureFilename = NULL;

		hr1 = D3DXSaveMeshToX(TEXT("SphereWhite.x"), mesh, NULL, &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);
		material.MatD3D.Ambient = material.MatD3D.Diffuse = material.MatD3D.Specular = material.MatD3D.Emissive = black;
		hr1 = D3DXSaveMeshToX(TEXT("SphereBlack.x"), mesh, NULL, &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);
		material.MatD3D.Ambient = material.MatD3D.Diffuse = material.MatD3D.Specular = material.MatD3D.Emissive = red;
		hr1 = D3DXSaveMeshToX(TEXT("SphereRed.x"), mesh, (DWORD *)(buffer->GetBufferPointer()), &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);
		material.MatD3D.Ambient = material.MatD3D.Diffuse = material.MatD3D.Specular = material.MatD3D.Emissive = pink;
		hr1 = D3DXSaveMeshToX(TEXT("SpherePink.x"), mesh, (DWORD *)(buffer->GetBufferPointer()), &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);
		material.MatD3D.Ambient = material.MatD3D.Diffuse = material.MatD3D.Specular = material.MatD3D.Emissive = green;
		hr1 = D3DXSaveMeshToX(TEXT("SphereGreen.x"), mesh, (DWORD *)(buffer->GetBufferPointer()), &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);
		material.MatD3D.Ambient = material.MatD3D.Diffuse = material.MatD3D.Specular = material.MatD3D.Emissive = blue;
		hr1 = D3DXSaveMeshToX(TEXT("SphereBlue.x"), mesh, (DWORD *)(buffer->GetBufferPointer()), &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);
		material.MatD3D.Ambient = material.MatD3D.Diffuse = material.MatD3D.Specular = material.MatD3D.Emissive = yellow;
		hr1 = D3DXSaveMeshToX(TEXT("SphereYellow.x"), mesh, (DWORD *)(buffer->GetBufferPointer()), &material, NULL, 1, D3DXF_FILEFORMAT_TEXT | D3DXF_FILESAVE_TOFILE);

#endif

	}

	ReleaseSemaphore(g_hSemaphoreMMDInitBlock, 1, NULL);

	return hr;
}

CHookIDirect3DDevice9MMD::CHookIDirect3DDevice9MMD(::IDirect3DDevice9 *pDevice)
{
	bShadowTarget = FALSE;

	if(pDevice) {
		this->pOriginal = pDevice;
		this->AddRef();
	}
}

CHookIDirect3DDevice9MMD::~CHookIDirect3DDevice9MMD()
{
	/* pOriginal->Release() は CHookIDirect3DDevice9::~CHookIDirect3DDevice9 で実施 */
}

// CHookIDirect3DDevice9MMD::IUnknown
HRESULT	CHookIDirect3DDevice9MMD::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_IDirect3DDevice9 ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

// IDirect3DDevice9
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::BeginScene()
{
	HRESULT hr;
	hr = this->pOriginal->BeginScene();
	WaitForSingleObject(g_hSemaphoreMMDRenderSync, 30);
	return hr;
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_DEFAULT ) {
		// Usage |= D3DUSAGE_DYNAMIC;	// 落ちる
	}
	else if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
	else if( Pool == D3DPOOL_SYSTEMMEM ) {
		//Pool = D3DPOOL_DEFAULT;
		// Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return this->pOriginal->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE IDirect3DVertexBuffer9MMD_Lock(IDirect3DVertexBuffer9 *pthis, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	IDirect3DVertexBuffer9_PrivateData *pHook = HookIDirect3DVertexBuffer9_GetPrivateData(pthis);

	DWORD dwLock = HookIDirect3DVertexBuffer9_GetOffset_Lock(pthis);
	tIDirect3DVertexBuffer9_Lock pLock = (tIDirect3DVertexBuffer9_Lock)HookIDirect3DVertexBuffer9_GetMethod(pthis, dwLock);

	/* Lock()に必要な情報を保存する */
	pHook->OffsetToLock = OffsetToLock;
	pHook->SizeToLock = SizeToLock;
	pHook->Flags = Flags;

	// ダミーバッファを返して、ここに書き込んでもらう。遅延Lockする。
	// こうするとなぜか早くなる
	if( pHook->pBuffer ) {
		*ppbData = (void *)pHook->pBuffer;
		return S_OK;
	}

	// return pLock(pthis, OffsetToLock, SizeToLock, ppbData, Flags);
	return pLock(pthis, OffsetToLock, SizeToLock, ppbData, Flags | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE);
}

HRESULT STDMETHODCALLTYPE IDirect3DVertexBuffer9MMD_Unlock(IDirect3DVertexBuffer9 *pthis)
{
	IDirect3DVertexBuffer9_PrivateData *pHook = HookIDirect3DVertexBuffer9_GetPrivateData(pthis);

	DWORD dwLock = HookIDirect3DVertexBuffer9_GetOffset_Lock(pthis);
	DWORD dwUnlock = HookIDirect3DVertexBuffer9_GetOffset_Unlock(pthis);
	tIDirect3DVertexBuffer9_Lock pLock = (tIDirect3DVertexBuffer9_Lock)HookIDirect3DVertexBuffer9_GetMethod(pthis, dwLock);
	tIDirect3DVertexBuffer9_Unlock pUnlock = (tIDirect3DVertexBuffer9_Unlock)HookIDirect3DVertexBuffer9_GetMethod(pthis, dwUnlock);

	if( pHook->pBuffer ) {
		HRESULT hr;
		BYTE *pBuffer = NULL;
		if( FAILED(hr = pLock(pthis, pHook->SizeToLock, pHook->OffsetToLock, (void **)&pBuffer, pHook->Flags | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) ) {
			return hr;
		}

		if( pHook->SizeToLock == 0 && pHook->OffsetToLock == 0 ) {
			memcpy(pBuffer, pHook->pBuffer, pHook->Length);
		}
		else {
			memcpy(pBuffer, pHook->pBuffer + pHook->OffsetToLock, pHook->SizeToLock);
		}
	}
	return pUnlock(pthis);
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	//return this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
	// 遅くなる
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_DEFAULT ) {
		Usage |= D3DUSAGE_DYNAMIC; //これがないとさらに遅くなる
	}
	else if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC; //これがないとさらに遅くなる

#if 0
		HRESULT hr;
		IDirect3DVertexBuffer9 *pVB = NULL;
		hr = this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, &pVB, pSharedHandle);

		DWORD dwLock = HookIDirect3DVertexBuffer9_GetOffset_Lock(pVB);
		DWORD dwUnlock = HookIDirect3DVertexBuffer9_GetOffset_Unlock(pVB);

		HookIDirect3DVertexBuffer9_Hookvtable(pVB, Length);
		HookIDirect3DVertexBuffer9_HookMethod(pVB, dwLock, (uintptr_t)IDirect3DVertexBuffer9MMD_Lock);
		HookIDirect3DVertexBuffer9_HookMethod(pVB, dwUnlock, (uintptr_t)IDirect3DVertexBuffer9MMD_Unlock);

		*ppVertexBuffer = pVB;

		return hr;
#endif
	}
#endif
	return this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_DEFAULT ) {
		// Usage |= D3DUSAGE_DYNAMIC; // VB/テクスチャしか付けなくていい？
	}
	else if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		// Usage |= D3DUSAGE_DYNAMIC; // VB/テクスチャしか付けなくていい？
	}
#endif
	return this->pOriginal->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	static D3DVIEWPORT9 vp_old;
	BOOL bResizeTrigger;

	if( ( vp_old.X != pViewport->X || vp_old.Y != pViewport->Y
		|| vp_old.Width != pViewport->Width || vp_old.Height != pViewport->Height )
	 || this->bResizeFlag )
	{
		bResizeTrigger = TRUE;
	}
	else
	{
		bResizeTrigger = FALSE;
	}

	if( this->bShadowTarget ) {
	}
	else if( bResizeTrigger ) {
		RECT rect;

		vp_old = *pViewport;
		this->bResizeFlag = FALSE;

		g_vpMirror = vp_old;

		if( !g_pMMEHookMirrorRT ) {
			rect.top = 0;
			rect.left = 0;
			rect.right = pViewport->Width;
			rect.bottom = pViewport->Height;
		}
		else {
			D3DSURFACE_DESC *pTexDesc = g_pMMEHookMirrorRT->GetRTTexDesc();
			rect.top = 0;
			rect.left = 0;
			rect.right = pTexDesc->Width;
			rect.bottom = pTexDesc->Height;
			g_vpMirror.Width = pTexDesc->Width;
			g_vpMirror.Height = pTexDesc->Height;
		}

#if 0
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & (~WS_OVERLAPPED), TRUE);

		g_rectLastMMDSize = rect;

		if( g_bMMDSyncResize ) {
			SetWindowPos(g_hWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
#endif
	}

	return this->pOriginal->SetViewport(pViewport);
}

HRESULT STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
#if 0
	/* ここでRTのサイズが2048x2048 or 4096x4096か？で判定できる。たぶんこっちのが↑より確実。 */
	D3DSURFACE_DESC desc;
	pRenderTarget->GetDesc(&desc);
#endif

	if( pRenderTarget != g_pPrimaryBackBuffer ) {
		this->bShadowTarget = TRUE;
	}
	else {
		this->bShadowTarget = FALSE;
	}
	return this->pOriginal->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}


CHookIDirect3DVertexBuffer9MMD::CHookIDirect3DVertexBuffer9MMD(::IDirect3DVertexBuffer9 *pD3DVB)
{
	if( pD3DVB ) {
		this->pOriginal = pD3DVB;
	}
}

CHookIDirect3DVertexBuffer9MMD::~CHookIDirect3DVertexBuffer9MMD()	
{
	/* pOriginal->Release() は CHookIDirect3DVertexBuffer9MMD::~CHookIDirect3DVertexBuffer9MMD で実施 */
}

// CHookIDirect3DVertexBuffer9MMD::IUnknown
HRESULT	STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9MMD::QueryInterface(REFIID riid, void** ppvObj)
{
	if( riid == IID_IDirect3DVertexBuffer9 ) {
		this->AddRef();
		*ppvObj = this;
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObj);
}

HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9MMD::Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	return this->pOriginal->Lock(OffsetToLock, SizeToLock, ppbData, Flags);
}


CHookID3DXEffectMMD::CHookID3DXEffectMMD(::ID3DXEffect *pEffect)
{
	if( pEffect ) {
		this->pOriginal = pEffect;
		this->AddRef();
	}
}

CHookID3DXEffectMMD::~CHookID3DXEffectMMD()
{
	/* pOriginal->Release() は CHookID3DXEffect::~CHookID3DXEffect で実施 */
}

// CHookID3DXEffectMMD::IUnknown
HRESULT	STDMETHODCALLTYPE CHookID3DXEffectMMD::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_ID3DXEffect ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

// カメラ編は影なし？
HRESULT STDMETHODCALLTYPE CHookID3DXEffectMMD::SetMatrix(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix)
{
	return pOriginal->SetMatrix(hParameter, pMatrix);
}

