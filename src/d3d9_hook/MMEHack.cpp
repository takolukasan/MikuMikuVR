#include "stdafx.h"

#define MME_DLL_NAME		TEXT("d3d9.dll\0")
#define MMEFFECT_DLL_NAME	TEXT("MMEffect.dll\0")
#define MMHACK_DLL_NAME		TEXT("MMHack.dll\0")
#define FILENAME_LENGTH		(1024)


/* Direct3D9 実装インターフェースポインタ */
tDirect3DCreate9 g_orgMMEDirect3DCreate9;
tD3DXCreateTexture g_orgMMEffectD3DXCreateTexture;


/* D3DX関数オリジナルアドレスのポインタ */
tD3DXCreateTexture g_orgMMED3DXCreateTexture;
tD3DXCreateTextureFromFileExA g_orgMMEffectD3DXCreateTextureFromFileExA;
tD3DXCreateTextureFromFileExA g_orgMMED3DXCreateTextureFromFileExA;
tD3DXCreateTextureFromFileExW g_orgMMED3DXCreateTextureFromFileExW;
tD3DXCreateTextureFromFileInMemoryEx g_orgMMED3DXCreateTextureFromFileInMemoryEx;
tD3DXCreateEffectFromResourceA g_orgMMED3DXCreateEffectFromResourceA;


/* Direct3D9 実装インターフェースポインタ */
CHookIDirect3D9MME *pMMEHookDirect3D9;
CHookIDirect3DDevice9MME *pMMEHookDirect3DDevice9;


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


static HRESULT MMEHack_GarbageCollection();

static HRESULT MMEHack_GarbageCollection()
{

	if( g_pMMEHookMirrorRT ) {
		if( g_pMMEHookMirrorRT->GetRefCnt() == 1 ) {
			delete g_pMMEHookMirrorRT;
			g_pMMEHookMirrorRT = NULL;
			pHookDirect3DDevice9->TriggerWindowResize();
		}
	}

	int i;
	std::vector<CHookID3DXEffectMMEOBJRenderer *>::iterator iterRender;

	iterRender = g_vecMirrorRenderer.begin();
	for( i = 0; i < (int)g_vecMirrorRenderer.size(); i++ ) {
		if( g_vecMirrorRenderer[i]->GetRefCnt() == 1 ) {
			delete g_vecMirrorRenderer[i];
			g_vecMirrorRenderer.erase(iterRender + i);
			i--;
			iterRender = g_vecMirrorRenderer.begin();
		}
	}

#ifdef OVR_ENABLE
	std::vector<CHookID3DXEffectOVRRenderer *>::iterator iterEye;

	iterEye = g_vecOVREyeRT[OVR_EYE_LEFT].begin();
	for( i = 0; i < (int)g_vecOVREyeRT[OVR_EYE_LEFT].size(); i++ ) {
		if( g_vecOVREyeRT[OVR_EYE_LEFT][i]->GetRefCnt() == 1 ) {
			delete g_vecOVREyeRT[OVR_EYE_LEFT][i];
			g_vecOVREyeRT[OVR_EYE_LEFT].erase(iterEye + i);
			i--;
			iterEye = g_vecOVREyeRT[OVR_EYE_LEFT].begin();
		}
	}

	iterEye = g_vecOVREyeRT[OVR_EYE_RIGHT].begin();
	for( i = 0; i < (int)g_vecOVREyeRT[OVR_EYE_RIGHT].size(); i++ ) {
		if( g_vecOVREyeRT[OVR_EYE_RIGHT][i]->GetRefCnt() == 1 ) {
			delete g_vecOVREyeRT[OVR_EYE_RIGHT][i];
			g_vecOVREyeRT[OVR_EYE_RIGHT].erase(iterEye + i);
			i--;
			iterEye = g_vecOVREyeRT[OVR_EYE_RIGHT].begin();
		}
	}
#endif

	return S_OK;
}


BOOL WINAPI MMEHack()
{
	DWORD dwLength;
	static const TCHAR tMMEDLLName[] = MME_DLL_NAME;
	TCHAR tcMyProcessParentName[FILENAME_LENGTH + 1];
	static const TCHAR tcYen[] = TEXT("\\");
	size_t len1,len2;
	int nCompResult;
	HMODULE hModMMEffect = NULL;
	HMODULE hModMME = NULL;
	HMODULE hModMMHack = NULL;

	dwLength = GetModuleFileName(NULL, tcMyProcessParentName, sizeof(tcMyProcessParentName));
	if( dwLength >= 0 ) {
		if( SUCCEEDED(StringCchLength(tMMEDLLName, FILENAME_LENGTH, &len1))
		 && SUCCEEDED(StringCchLength(tcMyProcessParentName, FILENAME_LENGTH, &len2)) ) {
			while( len2-- ) {
				nCompResult = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, tcYen, 1, tcMyProcessParentName + (len2), (int)1);
				if( nCompResult == CSTR_EQUAL )
					break;
			}
			StringCchCopyN(tcMyProcessParentName + len2 + 1, FILENAME_LENGTH - (len2 + 1), tMMEDLLName, len1);
			hModMME = GetModuleHandle(tcMyProcessParentName);
		}
	}

	hModMMEffect = GetModuleHandle(MMEFFECT_DLL_NAME);
	hModMMHack = GetModuleHandle(MMHACK_DLL_NAME);

	if( !hModMME || !hModMMHack )
		return FALSE;

	if( !HookFunctionOfModule(hModMMEffect, "D3DXCreateTexture", MMEffectHack_D3DXCreateTexture, (void **)&g_orgMMEffectD3DXCreateTexture) )
		return FALSE;
	if( !HookFunctionOfModule(hModMMEffect, "D3DXCreateTextureFromFileExA", MMEffectHack_D3DXCreateTextureFromFileExA, (void **)&g_orgMMEffectD3DXCreateTextureFromFileExA) )
		return FALSE;
#ifndef X64
	if( !HookFunctionOfModule(hModMMEffect, "D3DXCreateEffectFromFileA", MMEffectHack_D3DXCreateEffectFromFileA, (void **)&g_orgMMEffectD3DXCreateEffectFromFileA) )
		return FALSE;
#else
	if( !HookFunctionOfModule(hModMMEffect, "D3DXCreateEffectFromFileW", MMEffectHack_D3DXCreateEffectFromFileW, (void **)&g_orgMMEffectD3DXCreateEffectFromFileW) )
		return FALSE;
#endif

	if( !HookFunctionOfModule(hModMMHack, "Direct3DCreate9", MMEHack_Direct3DCreate9, (void **)&g_orgMMEDirect3DCreate9) )
		return FALSE;
	if( !HookFunctionOfModule(hModMMHack, "D3DXCreateEffectFromResourceA", MMEHack_D3DXCreateEffectFromResourceA, (void **)&g_orgMMED3DXCreateEffectFromResourceA) )
		return FALSE;
	if( !HookFunctionOfModule(hModMMHack, "D3DXCreateTexture", MMEHack_D3DXCreateTexture, (void **)&g_orgMMED3DXCreateTexture) )
		return FALSE;
	if( !HookFunctionOfModule(hModMMHack, "D3DXCreateTextureFromFileExA", MMEHack_D3DXCreateTextureFromFileExA, (void **)&g_orgMMED3DXCreateTextureFromFileExA) )
		return FALSE;
	if( !HookFunctionOfModule(hModMMHack, "D3DXCreateTextureFromFileExW", MMEHack_D3DXCreateTextureFromFileExW, (void **)&g_orgMMED3DXCreateTextureFromFileExW) )
		return FALSE;
	if( !HookFunctionOfModule(hModMMHack, "D3DXCreateTextureFromFileInMemoryEx", MMEHack_D3DXCreateTextureFromFileInMemoryEx, (void **)&g_orgD3DXCreateTextureFromFileInMemoryEx) )
		return FALSE;

	return TRUE;
}

IDirect3D9 * WINAPI MMEHack_Direct3DCreate9(UINT SDKVersion)
{
	IDirect3D9 *pD3D = NULL,*pD3DHooked = NULL;

#ifndef D3D9EX_ENABLE
	pD3D = g_orgMMEDirect3DCreate9(SDKVersion);
#else
	/* さすがにコレはだめ？→いける！ */
	IDirect3D9Ex *pD3DEx = NULL;
	Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3DEx);
	if( pD3DEx ) {
		pD3DEx->QueryInterface(IID_IDirect3D9, (void **)&pD3D);
		pD3DEx->Release();
	}
	else {
		pD3D = NULL;
	}
#endif
	if( !pD3D ) {
		return NULL;
	}

	pMMEHookDirect3D9 = new CHookIDirect3D9MME(pD3D);
	if( FAILED(pMMEHookDirect3D9->QueryInterface(IID_IDirect3D9, (void **)&pD3DHooked)) ) {
		pD3DHooked = pD3D;
		delete pMMEHookDirect3D9;
		pMMEHookDirect3D9 = NULL;

		g_bMMEHacked = FALSE;
	}
	else
	{
		pD3D->Release();

		g_bMMEHacked = TRUE;
	}

	return pD3DHooked;
}

HRESULT WINAPI MMEffectHack_D3DXCreateTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return g_orgMMEffectD3DXCreateTexture(pDevice, Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
}

HRESULT WINAPI MMEffectHack_D3DXCreateTextureFromFileExA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return g_orgMMEffectD3DXCreateTextureFromFileExA(
		pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool,
		Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
}


HRESULT WINAPI MMEHack_D3DXCreateTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return g_orgMMED3DXCreateTexture(pDevice, Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
}

HRESULT WINAPI
    MMEHack_D3DXCreateTextureFromFileExA(
        LPDIRECT3DDEVICE9         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return g_orgMMED3DXCreateTextureFromFileExA(
		pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool,
		Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
}

HRESULT WINAPI MMEHack_D3DXCreateTextureFromFileExW(
        LPDIRECT3DDEVICE9         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_MANAGED ) { // D3DUSAGE_DYNAMIC
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return g_orgMMED3DXCreateTextureFromFileExW(
		pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool,
		Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
}

HRESULT WINAPI MMEHack_D3DXCreateTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE9         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE9*       ppTexture)
{
#ifdef D3D9EX_ENABLE
	if( Pool == D3DPOOL_MANAGED ) {
		Pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
#endif
	return g_orgMMED3DXCreateTextureFromFileInMemoryEx(
		pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels,
		Usage, Format, Pool, Filter, MipFilter, ColorKey,
		pSrcInfo, pPalette, ppTexture);
}

HRESULT WINAPI MMEHack_D3DXCreateEffectFromResourceA(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors)
{
	HRESULT hr;
	hr = g_orgMMED3DXCreateEffectFromResourceA(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
	return hr;
}


CHookIDirect3D9MME::CHookIDirect3D9MME(::IDirect3D9 *pD3D)
{
	if( pD3D ) {
		this->pOriginal = pD3D;
		this->AddRef();
	}
}

CHookIDirect3D9MME::~CHookIDirect3D9MME()	
{
	/* pOriginal->Release() は CHookIDirect3D9::~CHookIDirect3D9 で実施 */
}

// CHookIDirect3D9MMD::IUnknown
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9MME::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_IDirect3D9 ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

ULONG STDMETHODCALLTYPE CHookIDirect3D9MME::Release()
{
#if 0
	/* ここまでで全部開放されてたらクリーンできる */
	if( pHookEffect && pHookEffect->GetRefCnt() == 1 ) {
		delete (pHookEffect);
		pHookEffect = NULL;
	}

	if( pHookDirect3DDevice9 && pHookDirect3DDevice9->GetRefCnt() == 1 ) {
		delete (pHookDirect3DDevice9);
		pHookDirect3DDevice9 = NULL;
	}
#endif
	/* 親クラスのRelease()を呼ばないと再帰して死ぬ */
	return this->CHookIDirect3D9::Release();
}

// IDirect3D9
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9MME::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
	IDirect3DDevice9 *p = NULL;
	HRESULT hr = S_OK;

#if 0
	// pPresentationParameters->MultiSampleType = D3DMULTISAMPLE_NONE;
	// pPresentationParameters->MultiSampleQuality = 0; 

	// pPresentationParameters->MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
	// pPresentationParameters->MultiSampleQuality = 3; 
	// pPresentationParameters->Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	BehaviorFlags |= D3DCREATE_MULTITHREADED;
#endif

	hr = this->pOriginal->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &p);

	if( SUCCEEDED(hr) ) {
		pMMEHookDirect3DDevice9 = new CHookIDirect3DDevice9MME(p);
		// 面倒でもQueryInterfaceを呼ぶべき！
		if( FAILED(pMMEHookDirect3DDevice9->QueryInterface(IID_IDirect3DDevice9, (void **)ppReturnedDeviceInterface) ) ) {
			*ppReturnedDeviceInterface = p;
			delete pMMEHookDirect3DDevice9;
			pMMEHookDirect3DDevice9 = NULL;

			g_bMMEHacked = FALSE;
		}
		else {
			pMMEHookDirect3DDevice9->Release(); /* コレをやるべきなのか？ */

			g_bMMEHacked = TRUE;
		}
	}

	return hr;
}

CHookIDirect3DDevice9MME::CHookIDirect3DDevice9MME(::IDirect3DDevice9 *pDevice)
{
	if(pDevice) {
		this->pOriginal = pDevice;
		this->AddRef();
	}
}

CHookIDirect3DDevice9MME::~CHookIDirect3DDevice9MME()
{
	/* pOriginal->Release() は CHookIDirect3DDevice9::~CHookIDirect3DDevice9 で実施 */
}

// CHookIDirect3DDevice9MME::IUnknown
HRESULT	CHookIDirect3DDevice9MME::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_IDirect3DDevice9 ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

#if 0
typedef HRESULT (WINAPI * tIDirect3DTexture9_LockRect)(IDirect3DTexture9 *pthis, UINT Level, D3DLOCKED_RECT * pLockedRect, CONST RECT * pRect, DWORD Flags);
HRESULT STDMETHODCALLTYPE IDirect3DTexture9MME_LockRect(IDirect3DTexture9 *pthis, UINT Level, D3DLOCKED_RECT * pLockedRect, CONST RECT * pRect, DWORD Flags)
{
	tIDirect3DTexture9_LockRect pOrg = (tIDirect3DTexture9_LockRect)IDirect3DTexture9_GetMethod(pthis, 0x4c);
	return pOrg(pthis, Level, pLockedRect, pRect, Flags);
	// return pOrg(pthis, Level, pLockedRect, pRect, Flags | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE);
}
#endif

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	HRESULT hr = S_OK;

	if( Usage & D3DUSAGE_DYNAMIC ) {
#if 1
		return hr = this->pOriginal->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
#else
		IDirect3DTexture9 *pTex = NULL;
		hr = this->pOriginal->CreateTexture(Width, Height, Levels, Usage, Format, Pool, &pTex, pSharedHandle);

		/* メモリリークバグあり */
		// pTex->LockRect(0, 0, NULL, 0);	// 0x4c

		HookIDirect3DTexture9_Vtbl(pTex);
		HookIDirect3DTexture9_Method(pTex, 0x4c, (uintptr_t)IDirect3DTexture9MME_LockRect);

		*ppTexture = pTex;
#endif
		return hr;
	}
	return this->pOriginal->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE IDirect3DVertexBuffer9MME_Lock(IDirect3DVertexBuffer9 *pthis, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
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

HRESULT STDMETHODCALLTYPE IDirect3DVertexBuffer9MME_Unlock(IDirect3DVertexBuffer9 *pthis)
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

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	HRESULT hr = S_OK;

	if( Usage & D3DUSAGE_DYNAMIC ) {
#if 0
		return hr = this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
#else
		HRESULT hr;
		IDirect3DVertexBuffer9 *pVB = NULL;
		hr = this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, &pVB, pSharedHandle);

		DWORD dwLock = HookIDirect3DVertexBuffer9_GetOffset_Lock(pVB);
		DWORD dwUnlock = HookIDirect3DVertexBuffer9_GetOffset_Unlock(pVB);

		HookIDirect3DVertexBuffer9_Hookvtable(pVB, Length);
		HookIDirect3DVertexBuffer9_HookMethod(pVB, dwLock, (uintptr_t)IDirect3DVertexBuffer9MME_Lock);
		HookIDirect3DVertexBuffer9_HookMethod(pVB, dwUnlock, (uintptr_t)IDirect3DVertexBuffer9MME_Unlock);

		*ppVertexBuffer = pVB;
#endif
		return hr;
	}
	return this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

#if 0
typedef HRESULT (WINAPI * tIDirect3DIndexBuffer9_Lock)(IDirect3DIndexBuffer9 *pthis, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
HRESULT STDMETHODCALLTYPE IDirect3DIndexBuffer9MME_Lock(IDirect3DIndexBuffer9 *pthis, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	tIDirect3DIndexBuffer9_Lock pOrg = (tIDirect3DIndexBuffer9_Lock)IDirect3DIndexBuffer9_GetMethod(pthis, 0x2c);
	return pOrg(pthis, OffsetToLock, SizeToLock, ppbData, Flags | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE);
}
#endif

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
	HRESULT hr = S_OK;
#ifdef D3D9EX_ENABLE
	if( Usage & D3DUSAGE_DYNAMIC ) {
#if 1
		return hr = this->pOriginal->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
#else
		IDirect3DIndexBuffer9 *pIB = NULL;
		hr = this->pOriginal->CreateIndexBuffer(Length, Usage, Format, Pool, &pIB, pSharedHandle);

		/* メモリリークバグあり */
		// pIB->Lock(0, 0, NULL, 0);	// 0x2c
		// pIB->Unlock();				// 0x30

		HookIDirect3DIndexBuffer9_Vtbl(pIB);
		HookIDirect3DIndexBuffer9_Method(pIB, 0x2c, (uintptr_t)IDirect3DIndexBuffer9MME_Lock);

		*ppIndexBuffer = pIB;
#endif
		return hr;
	}
#endif
	return this->pOriginal->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT STDMETHODCALLTYPE CHookIDirect3DDevice9MME::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
	if( pRenderTarget != g_pPrimaryBackBuffer ) {
		this->bShadowTarget = TRUE;
	}
	else {
		this->bShadowTarget = FALSE;
	}
	return this->pOriginal->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::BeginScene()
{
	HRESULT hr = S_OK;
#if 0
	RT_Swap();
	hr = this->pOriginal->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0);
	RT_Restore();
#endif
	return this->pOriginal->BeginScene();
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	HRESULT hr = S_OK;

	// if( (g_MirrorRenderObj & MIRROR_RENDER_MODE_MMEHACK) ) {

		RECT rect;
		RECT rectMirrorRT;

		rect.left = 0;
		rect.top = 0;

		if( g_pMMEHookMirrorRT ) {
			D3DSURFACE_DESC *pTexDesc = g_pMMEHookMirrorRT->GetRTTexDesc();

			/* Clear & StretchRect */
			RT_Swap();
#if 0
			hr = this->pOriginal->Clear(0, NULL, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,255,255,255), 1.0f, 0);
#endif

			D3DVIEWPORT9 vp_mirror;
			vp_mirror = g_vpMirror;
			vp_mirror.X = 0;
			vp_mirror.Y = 0;
			vp_mirror.Width = pTexDesc->Width;
			vp_mirror.Height = pTexDesc->Height;
			this->pOriginal->SetViewport(&vp_mirror);

			RECT rectSrc;
			rectSrc.left = 0;
			rectSrc.top = 0;
			rectSrc.right = pTexDesc->Width;
			rectSrc.bottom = pTexDesc->Height;
#if 0
			hr = this->pOriginal->StretchRect(g_pMMEHookMirrorRT->GetRTSurface(), &rectSrc, g_pMirBackBuffer, &rectSrc, D3DTEXF_LINEAR);
#endif

#ifdef OVR_ENABLE
			int i;
			RECT rectDst = rectSrc;

			D3DSURFACE_DESC MirBackBufferDesc;
			RECT rectDstMir[2];
			if( bOVREyeTexMirror ) {
				g_pMirBackBuffer->GetDesc(&MirBackBufferDesc);
				rectDstMir[0].left = 0;
				rectDstMir[0].top = 0;
				rectDstMir[0].right = MirBackBufferDesc.Width / 2;
				rectDstMir[0].bottom = MirBackBufferDesc.Height;

				rectDstMir[1].left = MirBackBufferDesc.Width / 2;
				rectDstMir[1].top = 0;
				rectDstMir[1].right = MirBackBufferDesc.Width;
				rectDstMir[1].bottom = MirBackBufferDesc.Height;
			}

			vp_mirror = g_vpMirror;
			for( i = 0; i < OVR_EYE_NUM; i++) {
				pTexDesc = g_pMMEHookMirrorRT->GetRTEyeTexDesc(i);
				vp_mirror.X = g_EyeRenderViewport[i].Pos.x;
				vp_mirror.Y = g_EyeRenderViewport[i].Pos.y;
				vp_mirror.Width = g_EyeRenderViewport[i].Size.w;
				vp_mirror.Height = g_EyeRenderViewport[i].Size.h;
				this->pOriginal->SetViewport(&vp_mirror);
				rectSrc.right = pTexDesc->Width;
				rectSrc.bottom = pTexDesc->Height;
				rectDst.right = g_EyeRenderViewport[i].Size.w;
				rectDst.bottom = g_EyeRenderViewport[i].Size.h;
				hr = this->pOriginal->StretchRect(g_pMMEHookMirrorRT->GetEyeSurface(i), &rectSrc, g_pEyeSurf[i], &rectDst, D3DTEXF_NONE);
				if( bOVREyeTexMirror ) {
					this->pOriginal->StretchRect(g_pMMEHookMirrorRT->GetEyeSurface(i), &rectSrc, g_pMirBackBuffer, &rectDstMir[i], D3DTEXF_NONE);
				}
			}
#endif

#ifdef OVR_ENABLE
			ReleaseSemaphore(g_hSemaphoreOVRRenderSync, 1, NULL);
#endif

			RT_Restore();

			rect.right = pTexDesc->Width;
			rect.bottom = pTexDesc->Height;

			rectMirrorRT.left = 0;
			rectMirrorRT.top = 0;
			rectMirrorRT.right = pTexDesc->Width;
			rectMirrorRT.bottom = pTexDesc->Height;
		}
		else {
			rect.right = pSourceRect->right - pSourceRect->left;
			rect.bottom = pSourceRect->bottom - pSourceRect->top;
			rectMirrorRT = *pSourceRect;
		}

#if 0
		float fAspect = (float)rect.bottom / (float)rect.right;

		/* 横が長い(上下が切れる) */
		if( (LONG)(fAspect * g_ClientRect.right) >= g_ClientRect.bottom ) {
			rect.right = g_ClientRect.right;
			rect.bottom = (LONG)((float)g_ClientRect.right * fAspect + 0.5);
		}
		else { /* 縦が長い(左右が切れる) */
			rect.right = (LONG)((float)g_ClientRect.bottom / fAspect);
			rect.bottom = g_ClientRect.bottom;
		}
#endif

		if( g_pMirSwapChain && g_hWnd ) {
			// g_pMirSwapChain->Present(pSourceRect, pDestRect, g_hWnd, NULL, 0);
			// g_pMirSwapChain->Present(&rectMirrorRT, &rect, g_hWnd, NULL, D3DPRESENT_DONOTWAIT);
			g_pMirSwapChain->Present(NULL, NULL, g_hWnd, NULL, D3DPRESENT_DONOTWAIT);
		}
	// }

	hr = this->pOriginal->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	/* ここがおそらく一番ヒマなタイミングになるはず？ */

	/* 誰からも参照されなくなったっぽいものは捨てる */
	MMEHack_GarbageCollection();

	return hr;
}

// モデルの描画では使われないっぽい？
// MMEにおいてはマーカーのみ？
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	HRESULT hr = S_OK;

#if 0
#ifdef MIRROR_RENDER
	if( !g_pMMEHookMirrorRT ) {
		if( (g_MirrorRenderObj & MIRROR_RENDER_MODE_MMEHACK) ) {
			if( g_MirrorRenderObj & MIRROR_RENDER_MAKER ) {
				RT_Swap();
				hr = this->pOriginal->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
				RT_Restore();
			}
		}
	}
#endif
#endif

	return this->pOriginal->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

//モデルメイン描画
//グリッド線の描画はType = D3DPT_LINELISTらしい
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT PrimitiveCount)
{
	HRESULT hr = S_OK;

#if 0
#ifdef MIRROR_RENDER
	if( !g_pMMEHookMirrorRT && !this->bShadowTarget ) {

		/* >0:モデル,=0:グリッド,<0:アクセサリ */
		if( (g_MirrorRenderObj & MIRROR_RENDER_MODE_MMEHACK) ) {
			if( ( Type != D3DPT_LINELIST )
			 || ( (g_MirrorRenderObj & MIRROR_RENDER_GRID) && Type == D3DPT_LINELIST ) ) {
				RT_Swap();
				hr = this->pOriginal->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
				RT_Restore();
			}
		}
	}
#endif
#endif

	return this->pOriginal->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
}

