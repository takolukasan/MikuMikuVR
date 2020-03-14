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



/* Direct3D9 実装インターフェースポインタ */
CHookIDirect3D9MMD *pHookDirect3D9;
CHookIDirect3DDevice9MMD *pHookDirect3DDevice9;
CHookID3DXEffectMMD *pHookEffect;

/* Direct3D9 インターフェース */
IDirect3DSurface9 *g_pPrimaryBackBuffer;
IDirect3DSurface9 *g_pPrimaryDepthStencil;

#ifdef OVR_ENABLE
IDirect3DTexture9 *g_pEyeTex[OVR_EYE_NUM];
IDirect3DSurface9 *g_pEyeSurf[OVR_EYE_NUM];
#endif


/* Direct3D共有リソース用ハンドル */
#ifdef D3D9EX_ENABLE
#ifdef OVR_ENABLE
HANDLE g_hEyeTexShareHandle[OVR_EYE_NUM];
#endif
#endif


static void CleanupD3DHookResources();



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
	if( bOVRInitialized && SUCCEEDED(OVRDistortion_D3D11Init()) ) {
		bOVRInitialized = TRUE;
	}
	else {
		bOVRInitialized = FALSE;
	}
#endif

	// 逆にここはMMD側のDirect3D初期化が終わるまでブロック
	WaitForSingleObject(g_hSemaphoreMMDInitBlock, INFINITE);
#ifdef OVR_ENABLE
	if( bOVRInitialized && SUCCEEDED(OVRDistortion_Create()) ) {
		bOVRInitialized = TRUE;
	}
	else {
		bOVRInitialized = FALSE;
	}
#endif

	SetupWindowState();

    ShowWindow( g_hWnd, SW_SHOW );

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
			if( bOVRInitialized && g_pMMEHookMirrorRT ) {
				static BOOL bMMDRenderTrigger = TRUE;

				if( bMMDRenderTrigger ) {
					bMMDRenderTrigger = FALSE;
					OVRDistortion_Render1();
				}

				if( WAIT_OBJECT_0 == WaitForSingleObject(g_hSemaphoreOVRRenderSync, 0) ) {	/* MMD側描画完了 */
					bMMDRenderTrigger = TRUE;

					OVRDistortion_Render2();

					if( g_pProfiler ) {
						g_pProfiler->LeaveCheckPoint();
						g_pProfiler->EndFrame();
						g_pProfiler->BeginFrame();
						g_pProfiler->EnterCheckPoint(TEXT("g_hSemaphoreMMDRenderSync"));
					}
					/* リソース(テクスチャ)使用終了 & VSync完 */
					ReleaseSemaphore(g_hSemaphoreMMDRenderSync, 1, NULL);

				}
				/* FPS表示はヒマな時にやろうよ？ */
				if( g_nOVRFPS >= 0 ) {
					TCHAR tcFPSBuffer[MAX_PATH];
					swprintf_s(tcFPSBuffer, MAX_PATH, L"FPS: %d\n", g_nOVRFPS);
					// SetOVRWindowTitleSuffix(tcFPSBuffer);
					g_nOVRFPS = -1;
				}
#ifdef _DEBUG
				/* 続けて暇な時処理 */
				HMODULE hModMMotion;
				typedef void (__stdcall * tSetHeadPoint)(D3DXVECTOR3 *);
				tSetHeadPoint pFunc;
				D3DXVECTOR3 vecHead;

				hModMMotion = GetModuleHandle(TEXT("DxOpenNI.DLL"));
				if( hModMMotion ) {
					pFunc = (tSetHeadPoint)GetProcAddress(hModMMotion, "_MMMotionControl_SetHeadPoint@4");
					if( pFunc ) {
						vecHead.x = g_RiftLastPose[OVR_EYE_LEFT].Position.x * -5;
						vecHead.y = g_RiftLastPose[OVR_EYE_LEFT].Position.y * 1;
						vecHead.z = g_RiftLastPose[OVR_EYE_LEFT].Position.z * 5;
						pFunc(&vecHead);
					}
				}
#endif
			}
			else {
				if( bOVRInitialized ) {
					OVRDistortion_Render1();
					OVRDistortion_Render2();
				}
				if( g_pProfiler ) {
					g_pProfiler->EndFrame();
					g_pProfiler->BeginFrame();
					g_pProfiler->EnterCheckPoint(TEXT("g_hSemaphoreMMDRenderSync"));
				}

				/* 初期化未完了時は強制許可しないとMMDが止まる */
				ReleaseSemaphore(g_hSemaphoreMMDRenderSync, 1, NULL);

				Sleep(1);
			}
#else
			Sleep(1);
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

	/* 運がよければ開放される・・・ */
	if( pHookDirect3D9 && pHookDirect3D9->GetRefCnt() == 1 ) {
		delete pHookDirect3D9;
		pHookDirect3D9 = NULL;
	}

	CloseHandle(g_hSemaphoreMMDShutdownBlock);

	return 0;
}

static void CleanupD3DHookResources()
{
	int i;
	for(i = 0; i < OVR_EYE_NUM; i++) {
		RELEASE(g_pEyeTex[i]);
		RELEASE(g_pEyeSurf[i]);
	}

	RELEASE(g_pPrimaryBackBuffer);
	RELEASE(g_pPrimaryDepthStencil);
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

		// Wait for OVR initialize...
		WaitForSingleObject(g_hMutexD3DCreateBlock, INFINITE);

		D3DSURFACE_DESC SurfDesc[2];
		ZeroMemory(SurfDesc, sizeof(SurfDesc));
		hr1 = g_pPrimaryBackBuffer->GetDesc(&SurfDesc[0]);
		hr1 = g_pPrimaryDepthStencil->GetDesc(&SurfDesc[1]);

		D3DPRESENT_PARAMETERS pr = *pPresentationParameters;
		pr.hDeviceWindow = g_hWnd;
		// pr.BackBufferCount = 1;


#ifdef OVR_ENABLE
		if( g_pRift ) { /* Rift 初期化成功？ */
			int i;
			for(i = 0; i < OVR_EYE_NUM; i++)
			{
				hr1 = p->CreateTexture(g_EyeRenderViewport[i].Size.w, g_EyeRenderViewport[i].Size.h, 1, D3DUSAGE_RENDERTARGET,
					SurfDesc[0].Format, D3DPOOL_DEFAULT, &g_pEyeTex[i], &g_hEyeTexShareHandle[i]);
				if( FAILED(hr) || !g_pEyeTex[i] )
					break;
				hr1 = g_pEyeTex[i]->GetSurfaceLevel(0, &g_pEyeSurf[i]);
				if( FAILED(hr) || !g_pEyeSurf[i] )
					break;
			}
		}
#endif

	}

	ReleaseSemaphore(g_hSemaphoreMMDInitBlock, 1, NULL);

	return hr;
}

CHookIDirect3DDevice9MMD::CHookIDirect3DDevice9MMD(::IDirect3DDevice9 *pDevice)
{
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
#ifdef OVR_ENABLE
	WaitForSingleObject(g_hSemaphoreMMDRenderSync, 1000);
#endif

	if( g_pProfiler ) {
		g_pProfiler->LeaveCheckPoint();
		g_pProfiler->EnterCheckPoint(TEXT("CHookIDirect3DDevice9MMD::BeginScene()"));
	}

	return hr;
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::EndScene()
{
	if( g_pProfiler ) {
		g_pProfiler->LeaveCheckPoint();
		g_pProfiler->EnterCheckPoint(TEXT("CHookIDirect3DDevice9MMD::EndScene()"));
	}

	HRESULT hr = this->pOriginal->EndScene();

#if 0
	if( g_pProfiler ) {
		g_pProfiler->LeaveCheckPoint();
		g_pProfiler->EnterCheckPoint(TEXT("CHookIDirect3DDevice9MMD::EndScene() / copy eye texture"));
	}

	if( g_pMMEHookMirrorRT ) {
		RECT rectSrc = { 0, 0, 0, 0 };

#ifdef OVR_ENABLE
		int i;
		RECT rectDst = { 0, 0, 0, 0 };

		D3DSURFACE_DESC *pTexDesc;

		for( i = 0; i < OVR_EYE_NUM; i++) {
			pTexDesc = g_pMMEHookMirrorRT->GetRTEyeTexDesc(i);
			rectSrc.right = pTexDesc->Width;
			rectSrc.bottom = pTexDesc->Height;
			rectDst.right = g_EyeRenderViewport[i].Size.w;
			rectDst.bottom = g_EyeRenderViewport[i].Size.h;

			/* D3D11送信用テクスチャへコピー */
			hr = this->pOriginal->StretchRect(g_pMMEHookMirrorRT->GetEyeSurface(i), &rectSrc, g_pEyeSurf[i], &rectDst, D3DTEXF_NONE);

		}
#endif

#ifdef OVR_ENABLE
		/* テクスチャ準備完了 */
		ReleaseSemaphore(g_hSemaphoreOVRRenderSync, 1, NULL);
#endif

	}

	if( g_pProfiler ) {
		g_pProfiler->LeaveCheckPoint();
		g_pProfiler->EnterCheckPoint(TEXT("CHookIDirect3DDevice9MMD::EndScene() to Present()"));
	}
#endif

	return hr;
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	if( g_pProfiler ) {
		g_pProfiler->LeaveCheckPoint();
		g_pProfiler->EnterCheckPoint(TEXT("CHookIDirect3DDevice9MMD::Present() to CHookIDirect3DDevice9MME::Present()"));
	}
	return this->pOriginal->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

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
	return this->pOriginal->SetViewport(pViewport);
}

HRESULT STDMETHODCALLTYPE CHookIDirect3DDevice9MMD::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
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

