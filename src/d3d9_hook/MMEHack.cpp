#include "stdafx.h"

#define MME_DLL_NAME		D3D9_DLL_NAME
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




static HRESULT MMEHack_GarbageCollection();

static HRESULT MMEHack_GarbageCollection()
{
	int i;

	if( g_pMMEHookMirrorRT ) {
		if( g_pMMEHookMirrorRT->GetRefCnt() == 1 ) {
			delete g_pMMEHookMirrorRT;
			g_pMMEHookMirrorRT = NULL;
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
	if( DynDirect3DCreate9Ex ) {
		DynDirect3DCreate9Ex(D3D_SDK_VERSION, &pD3DEx);
	}
	else {
		Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3DEx);
	}
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
		if( FAILED(pDevice->GetSwapChain(0, &this->pSwapChainMME) ) ) {
			this->pSwapChainMME = NULL;
		}
	}
}

CHookIDirect3DDevice9MME::~CHookIDirect3DDevice9MME()
{
	/* pOriginal->Release() は CHookIDirect3DDevice9::~CHookIDirect3DDevice9 で実施 */

	RELEASE(this->pSwapChainMME);
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

static HRESULT STDMETHODCALLTYPE IDirect3DVertexBuffer9MME_Lock(IDirect3DVertexBuffer9 *pthis, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
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
	// でもダミーバッファ返すだけだとなぜか頂点バッファがこわれるので遅延ロックは止める。
	if( pHook->BufferFirstWritten && pHook->pBuffer ) {
		*ppbData = (void *)(pHook->pBuffer + OffsetToLock);
#if 0 /* 遅延LOCK */
		if( SizeToLock == 0 && OffsetToLock == 0 ) {
			memcpy(*ppbData, pHook->pBuffer, pHook->Length);
		}
		else {
			memcpy(*ppbData, pHook->pBuffer + OffsetToLock, SizeToLock);
		}
#else
		return pLock(pthis, OffsetToLock, SizeToLock, (void **)&(pHook->_pLockedBuffer), D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE);	/* とりあえずここのオプションが大事 */
#endif
		return S_OK;
	}

	// return pLock(pthis, OffsetToLock, SizeToLock, ppbData, Flags);
	return pLock(pthis, OffsetToLock, SizeToLock, ppbData, Flags | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE);
}

static HRESULT STDMETHODCALLTYPE IDirect3DVertexBuffer9MME_Unlock(IDirect3DVertexBuffer9 *pthis)
{
	IDirect3DVertexBuffer9_PrivateData *pHook = HookIDirect3DVertexBuffer9_GetPrivateData(pthis);

	DWORD dwLock = HookIDirect3DVertexBuffer9_GetOffset_Lock(pthis);
	tIDirect3DVertexBuffer9_Lock pLock = (tIDirect3DVertexBuffer9_Lock)HookIDirect3DVertexBuffer9_GetMethod(pthis, dwLock);

	DWORD dwUnlock = HookIDirect3DVertexBuffer9_GetOffset_Unlock(pthis);
	tIDirect3DVertexBuffer9_Unlock pUnlock = (tIDirect3DVertexBuffer9_Unlock)HookIDirect3DVertexBuffer9_GetMethod(pthis, dwUnlock);

	if( pHook->BufferFirstWritten && pHook->pBuffer ) {
		BYTE *pBuffer = NULL;
#if 0 /* 遅延LOCK */
		HRESULT hr;
		if( FAILED(hr = pLock(pthis, pHook->SizeToLock, pHook->OffsetToLock, (void **)&pBuffer, pHook->Flags | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) ) {
			return hr;
		}
#else
		pBuffer = pHook->_pLockedBuffer;
#endif

		if( pHook->SizeToLock == 0 && pHook->OffsetToLock == 0 ) {
			memcpy(pBuffer, pHook->pBuffer, pHook->Length);
		}
		else {
			memcpy(pBuffer, pHook->pBuffer + pHook->OffsetToLock, pHook->SizeToLock);
		}
	}
	else if( pHook->pBuffer ) {
		BYTE *pBuffer;
		/* 最初に書き込まれたバッファの中身をバッファにもらっておく。D3DLOCK_READONLY でOK */
		if( SUCCEEDED(pLock(pthis, 0, 0, (void **)&pBuffer, D3DLOCK_READONLY)) ) {
			memcpy(pHook->pBuffer, pBuffer, pHook->Length);
			pUnlock(pthis);
		}
		pHook->BufferFirstWritten = TRUE;
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

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::BeginScene()
{
	HRESULT hr;
	hr = this->pOriginal->BeginScene();

	if( g_pRift ) {
		/* ここでいい？？ */
		D3DXMATRIX matHeadMove,matRotateYAxis,matHeadMovRot;
		D3DXMatrixTranslation(&matHeadMove, (float)g_dMovingPosX, (float)g_dMovingPosY, (float)g_dMovingPosZ);
		D3DXMatrixRotationY(&matRotateYAxis, (float)g_dRotationY);
		matHeadMovRot = matHeadMove * matRotateYAxis;

		D3DXMATRIX matView[OVR_EYE_NUM];
		g_pRift->GetViewMatrixFromLastPose(&matView[0]);

		this->matEyeView[OVR_EYE_LEFT] = matHeadMovRot * matView[OVR_EYE_LEFT];
		this->matEyeView[OVR_EYE_RIGHT] = matHeadMovRot * matView[OVR_EYE_RIGHT];

		D3DXMATRIX matProj[ovrEye_Count];
		if( g_dFovZoom < 1.0 ) {
			g_pRift->GetProjectionMatrix(matProj, g_dFovZoom);
		}
		else {
			matProj[OVR_EYE_LEFT] = g_matOVREyeProj[OVR_EYE_LEFT];
			matProj[OVR_EYE_RIGHT] = g_matOVREyeProj[OVR_EYE_RIGHT];
		}
		this->matProjection[OVR_EYE_LEFT] = matProj[OVR_EYE_LEFT];
		this->matProjection[OVR_EYE_RIGHT] = matProj[OVR_EYE_RIGHT];
	}
	return hr;
}

HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9MME::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	HRESULT hr = S_OK;

	if( g_pMMEHookMirrorRT ) {
		RECT rectSrc = { 0, 0, 0, 0 };

#ifdef OVR_ENABLE
		int i;
		RECT rectDst = { 0, 0, 0, 0 };

		RECT rectDstMir[2];
		if( bOVREyeTexMirror ) {
			rectDstMir[0].left = 0;
			rectDstMir[0].top = 0;
			rectDstMir[0].right = g_MirBackBufferDesc.Width / 2;
			rectDstMir[0].bottom = g_MirBackBufferDesc.Height;

			rectDstMir[1].left = g_MirBackBufferDesc.Width / 2;
			rectDstMir[1].top = 0;
			rectDstMir[1].right = g_MirBackBufferDesc.Width;
			rectDstMir[1].bottom = g_MirBackBufferDesc.Height;
		}

		D3DSURFACE_DESC *pTexDesc;

		for( i = 0; i < OVR_EYE_NUM; i++) {
			pTexDesc = g_pMMEHookMirrorRT->GetRTEyeTexDesc(i);
			rectSrc.right = pTexDesc->Width;
			rectSrc.bottom = pTexDesc->Height;
			rectDst.right = g_EyeRenderViewport[i].Size.w;
			rectDst.bottom = g_EyeRenderViewport[i].Size.h;

			/* D3D11送信用テクスチャへコピー */
			hr = this->pOriginal->StretchRect(g_pMMEHookMirrorRT->GetEyeSurface(i), &rectSrc, g_pEyeSurf[i], &rectDst, D3DTEXF_NONE);

			if( bOVREyeTexMirror ) {
				/* 左右の目のテクスチャをミラー表示用のバックバッファにコピーする */
				this->pOriginal->StretchRect(g_pMMEHookMirrorRT->GetEyeSurface(i), &rectSrc, g_pMirBackBuffer, &rectDstMir[i], D3DTEXF_NONE);
			}
		}
#endif

#ifdef OVR_ENABLE
		/* テクスチャ準備完了 */
		ReleaseSemaphore(g_hSemaphoreOVRRenderSync, 1, NULL);
#endif

	}
	else {
	}


	if( g_pMirSwapChain && g_hWnd && bOVREyeTexMirror ) {
		g_pMirSwapChain->Present(NULL, NULL, g_hWnd, NULL, D3DPRESENT_DONOTWAIT);
	}

	if( this->pSwapChainMME ) {
		hr = this->pSwapChainMME->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, D3DPRESENT_DONOTWAIT);
	}
	else {
		hr = this->pOriginal->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	/* ここがおそらく一番ヒマなタイミングになるはず？ */

	/* 誰からも参照されなくなったっぽいものは捨てる */
	MMEHack_GarbageCollection();

	return hr;
}

