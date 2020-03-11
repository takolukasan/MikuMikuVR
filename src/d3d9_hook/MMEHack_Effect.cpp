/******************************************************************************/
/* MMDMirrorView                                                              */
/*                                                                            */
/* ファイル名：MMEHack_Effect.cpp                                             */
/* 作成：たこルカさん                                                         */
/* 説明：MMEからD3DX関数コールのフックとエフェクト操作のフック                */
/*                                                                            */
/******************************************************************************/

#include "stdafx.h"

/* Direct3DX9 実装インターフェースポインタ */
// 使用されなくなったときのガベージコレクションは別で実装している。
CHookID3DXEffectMMEMirrorRT *g_pMMEHookMirrorRT;
// Rendererはオブジェクト数分？クローンされるのでたくさん管理できるようにする。
std::vector<CHookID3DXEffectMMEOBJRenderer *> g_vecMirrorRenderer;

#ifdef OVR_ENABLE
// Rendererはオブジェクト数分？クローンされるのでたくさん管理できるようにする。
std::vector<CHookID3DXEffectOVRRenderer *> g_vecOVREyeRT[OVR_EYE_NUM];
#endif

/* D3DX関数オリジナルアドレスのポインタ */
#ifndef X64
tD3DXCreateEffectFromFileA g_orgMMEffectD3DXCreateEffectFromFileA;
#else
tD3DXCreateEffectFromFileW g_orgMMEffectD3DXCreateEffectFromFileW;
#endif


// ファイル名の先頭文字インデックスを取得する
#ifndef X64
static size_t GetFileNameIndexFromFullPathA(LPCSTR lpFullPathName, size_t MaxLength)
{
	int nCompResult;
	const CHAR tcYen[] = "\\";
	size_t len;

	if( SUCCEEDED(StringCchLengthA(lpFullPathName, MaxLength, &len)) ) {
		while( len-- ) {
			nCompResult = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, tcYen, 1, lpFullPathName + (len), (int)1);
			if( nCompResult == CSTR_EQUAL ) {
				return len + 1;
			}
		}
	}

	/* 見つからないということはファイル名のみだろう */
	return 0;
}
#else
static size_t GetFileNameIndexFromFullPathW(LPCWSTR lpFullPathName, size_t MaxLength)
{
	int nCompResult;
	const WCHAR tcYen[] = L"\\";
	size_t len;

	if( SUCCEEDED(StringCchLengthW(lpFullPathName, MaxLength, &len)) ) {
		while( len-- ) {
			nCompResult = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, tcYen, 1, lpFullPathName + (len), (int)1);
			if( nCompResult == CSTR_EQUAL ) {
				return len + 1;
			}
		}
	}

	/* 見つからないということはファイル名のみだろう */
	return 0;
}
#endif

// x86: ANSI x64: Unicodeでインポートしているとかいう、まさに外道
#ifndef X64

HRESULT WINAPI MMEffectHack_D3DXCreateEffectFromFileA(
        LPDIRECT3DDEVICE9               pDevice,
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors)

#else

HRESULT WINAPI MMEffectHack_D3DXCreateEffectFromFileW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors)

#endif
{
	ID3DXEffect *p = NULL;
	HRESULT hr;

#ifndef X64
	hr = g_orgMMEffectD3DXCreateEffectFromFileA(
		pDevice, pSrcFile, pDefines, pInclude,
		Flags, pPool, &p, ppCompilationErrors);
#else
	hr = g_orgMMEffectD3DXCreateEffectFromFileW(
		pDevice, pSrcFile, pDefines, pInclude,
		Flags, pPool, &p, ppCompilationErrors);
#endif

	if( SUCCEEDED(hr) ) {
		size_t index,len1,len2;
		int nCompResult_RT,nCompResult_OBJ;
#ifdef OVR_ENABLE
		int nCompResult_EyeRT[OVR_EYE_NUM];
#endif

#ifndef X64
		/* x86: 読み出したファイルを検証 全部ANSI文字で処理すること。 */
		char *szFilename;

		len1 = strlen(pSrcFile);
		index = GetFileNameIndexFromFullPathA(pSrcFile, len1+1);

		// MMEHACK_EFFECT_RENDERTARGET : RT Buffer
		szFilename = MMEHACK_EFFECT_RENDERTARGET;
		len2 = strlen(szFilename);
		nCompResult_RT = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);

#if 0
		// MMEHACK_EFFECT_OBJRENDERER : Object renderer
		szFilename = MMEHACK_EFFECT_OBJRENDERER;
		len2 = strlen(szFilename);
		nCompResult_OBJ = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);
#endif

#ifdef OVR_ENABLE
		// MMEHACK_EFFECT_OVRRENDERL : OVR Eye renderer Left
		szFilename = MMEHACK_EFFECT_OVRRENDERL;
		len2 = strlen(szFilename);
		nCompResult_EyeRT[OVR_EYE_LEFT] = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);

		// MMEHACK_EFFECT_OVRRENDERR : OVR Eye renderer Left
		szFilename = MMEHACK_EFFECT_OVRRENDERR;
		len2 = strlen(szFilename);
		nCompResult_EyeRT[OVR_EYE_RIGHT] = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);
#endif

#else
		/* x64: 読み出したファイルを検証 全部Unicode文字で処理すること。 */
		wchar_t *szFilename;

		len1 = wcslen(pSrcFile);
		index = GetFileNameIndexFromFullPathW(pSrcFile, len1+1);

		// MMEHACK_EFFECT_RENDERTARGET : RT Buffer
		szFilename = TEXT(MMEHACK_EFFECT_RENDERTARGET);
		len2 = wcslen(szFilename);
		nCompResult_RT = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);

#if 0
		// MMEHACK_EFFECT_OBJRENDERER : Object renderer
		szFilename = TEXT(MMEHACK_EFFECT_OBJRENDERER);
		len2 = wcslen(szFilename);
		nCompResult_OBJ = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);
#endif

#ifdef OVR_ENABLE
		// MMEHACK_EFFECT_OVRRENDERL : OVR Eye renderer Left
		szFilename = TEXT(MMEHACK_EFFECT_OVRRENDERL);
		len2 = wcslen(szFilename);
		nCompResult_EyeRT[OVR_EYE_LEFT] = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);

		// MMEHACK_EFFECT_OVRRENDERR : OVR Eye renderer Left
		szFilename = TEXT(MMEHACK_EFFECT_OVRRENDERR);
		len2 = wcslen(szFilename);
		nCompResult_EyeRT[OVR_EYE_RIGHT] = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pSrcFile + index, (int)(len1 - index), szFilename, (int)len2);
#endif

#endif

		if( CSTR_EQUAL == nCompResult_RT ) {
			if( g_pMMEHookMirrorRT && g_pMMEHookMirrorRT->GetRefCnt() == 1 ) {	/* ここまでにお行儀よくRelease()してくれてるようだが・・・ */
				delete g_pMMEHookMirrorRT;
			}
			g_pMMEHookMirrorRT = new CHookID3DXEffectMMEMirrorRT(p);

			if( !g_pMMEHookMirrorRT ) {
				*ppEffect = p;
			}
			else if( FAILED(g_pMMEHookMirrorRT->QueryInterface(IID_ID3DXEffect, (void **)ppEffect)) ) {
				*ppEffect = p;
				delete g_pMMEHookMirrorRT;
				g_pMMEHookMirrorRT = NULL;
			}
			else {
				p->Release();
			}
		}
#if 0
		else if( CSTR_EQUAL == nCompResult_OBJ ) {
			CHookID3DXEffectMMEOBJRenderer *pRenderer = NULL;

			pRenderer = new CHookID3DXEffectMMEOBJRenderer(p);
			if( !pRenderer ) {
				*ppEffect = p;
			}
			else if( FAILED(pRenderer->QueryInterface(IID_ID3DXEffect, (void **)ppEffect)) ) {
				*ppEffect = p;
				delete pRenderer;
			}
			else {
				p->Release();
				g_vecMirrorRenderer.push_back(pRenderer);
			}
		}
#endif
#ifdef OVR_ENABLE
		else if( CSTR_EQUAL == nCompResult_EyeRT[OVR_EYE_LEFT] ) {
			CHookID3DXEffectOVRRenderer *pRenderer = NULL;

			pRenderer = new CHookID3DXEffectOVRRenderer(p);
			if( !pRenderer ) {
				*ppEffect = p;
			}
			else if( FAILED(pRenderer->QueryInterface(IID_ID3DXEffect, (void **)ppEffect)) ) {
				*ppEffect = p;
				delete pRenderer;
			}
			else {
				p->Release();
				g_vecOVREyeRT[OVR_EYE_LEFT].push_back(pRenderer);
				pRenderer->SetOVREye(OVR_EYE_LEFT);
				pRenderer->SetProjMatrix(&g_matOVREyeProj[OVR_EYE_LEFT]);
			}
		}
		else if( CSTR_EQUAL == nCompResult_EyeRT[OVR_EYE_RIGHT] ) {
			CHookID3DXEffectOVRRenderer *pRenderer = NULL;

			pRenderer = new CHookID3DXEffectOVRRenderer(p);
			if( !pRenderer ) {
				*ppEffect = p;
			}
			else if( FAILED(pRenderer->QueryInterface(IID_ID3DXEffect, (void **)ppEffect)) ) {
				*ppEffect = p;
				delete pRenderer;
			}
			else {
				p->Release();
				g_vecOVREyeRT[OVR_EYE_RIGHT].push_back(pRenderer);
				pRenderer->SetOVREye(OVR_EYE_RIGHT);
				pRenderer->SetProjMatrix(&g_matOVREyeProj[OVR_EYE_RIGHT]);
			}
		}
		else {
			*ppEffect = p;
		}
#endif
	}

	return hr;
}

CHookID3DXEffectMMEMirrorRT::CHookID3DXEffectMMEMirrorRT(::ID3DXEffect *pEffect)
{
	this->hMirrorRT = NULL;
	this->pSurfRT = NULL;
	ZeroMemory(&this->RTTexDesc, sizeof(this->RTTexDesc));

#ifdef OVR_ENABLE
	this->hEyeRT[0] = NULL;
	this->hEyeRT[1] = NULL;
	this->pSurfRTEye[0] = NULL;
	this->pSurfRTEye[1] = NULL;
	ZeroMemory(this->RTEyeTexDesc, sizeof(this->RTEyeTexDesc));
#endif

	if( pEffect ) {
		this->pOriginal = pEffect;
		this->AddRef();

		D3DXHANDLE handle;
		handle = this->pOriginal->GetParameterByName(NULL, MMEHACK_EFFECT_TEXTURENAME);
		if( handle ) {
			this->hMirrorRT = handle;
		}
		else {
			this->hMirrorRT = NULL;
		}

#ifdef OVR_ENABLE
		handle = this->pOriginal->GetParameterByName(NULL, MMEHACK_EFFECT_EYETEXTURENAMEL);
		if( handle ) {
			this->hEyeRT[0] = handle;
		}
		else {
			this->hEyeRT[0] = NULL;
		}

		handle = this->pOriginal->GetParameterByName(NULL, MMEHACK_EFFECT_EYETEXTURENAMER);
		if( handle ) {
			this->hEyeRT[1] = handle;
		}
		else {
			this->hEyeRT[1] = NULL;
		}
#endif

	}
}

CHookID3DXEffectMMEMirrorRT::~CHookID3DXEffectMMEMirrorRT()
{
	/* pOriginal->Release() は CHookID3DXEffect::~CHookID3DXEffect で実施 */
	if( this->pSurfRT ) {
		this->pSurfRT->Release();
		this->pSurfRT = NULL;
	}
#ifdef OVR_ENABLE
	if( this->pSurfRTEye[0] ) {
		this->pSurfRTEye[0]->Release();
		this->pSurfRTEye[0] = NULL;
	}
	if( this->pSurfRTEye[1] ) {
		this->pSurfRTEye[1]->Release();
		this->pSurfRTEye[1] = NULL;
	}
#endif
}

// CHookID3DXEffectMMEMirrorRT::IUnknown
HRESULT	STDMETHODCALLTYPE CHookID3DXEffectMMEMirrorRT::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_ID3DXEffect ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

HRESULT STDMETHODCALLTYPE CHookID3DXEffectMMEMirrorRT::SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture)
{
	if( hParameter == this->hMirrorRT ) {
		HRESULT hr;
		IDirect3DTexture9 *pTextureRT;

		if( this->pSurfRT ) {
			this->pSurfRT->Release();
			this->pSurfRT = NULL;
		}
		ZeroMemory(&(this->RTTexDesc), sizeof(this->RTTexDesc));

		if( pTexture ) {
			if( SUCCEEDED(pTexture->QueryInterface(IID_IDirect3DTexture9, (void **)&pTextureRT)) ) {
				hr = pTextureRT->GetSurfaceLevel(0, &(this->pSurfRT));
				if( SUCCEEDED(hr) && this->pSurfRT ) {
					hr = this->pSurfRT->GetDesc(&(this->RTTexDesc));
				}
				pTextureRT->Release();

				/* バックバッファ解像度変更をDirect3D/Window管理側に通知 */
				pHookDirect3DDevice9->TriggerWindowResize();
			}
		}
	}
#ifdef OVR_ENABLE
	else if( hParameter == this->hEyeRT[0] || hParameter == this->hEyeRT[1] ) {
		int eye;
		HRESULT hr;
		IDirect3DTexture9 *pTextureRT;
		if( hParameter == this->hEyeRT[0] ) {
			eye = 0;
		}
		else {
			eye = 1;
		}
		if( this->pSurfRTEye[eye] ) {
			this->pSurfRTEye[eye]->Release();
			this->pSurfRTEye[eye] = NULL;
		}
		ZeroMemory(&(this->RTEyeTexDesc[eye]), sizeof(this->RTEyeTexDesc[eye]));

		if( pTexture ) {
			if( SUCCEEDED(pTexture->QueryInterface(IID_IDirect3DTexture9, (void **)&pTextureRT)) ) {
				hr = pTextureRT->GetSurfaceLevel(0, &(this->pSurfRTEye[eye]));
				if( SUCCEEDED(hr) && this->pSurfRTEye[eye] ) {
					hr = this->pSurfRTEye[eye]->GetDesc(&(this->RTEyeTexDesc[eye]));
				}
				pTextureRT->Release();
			}
		}
	}
#endif
	return this->pOriginal->SetTexture(hParameter, pTexture);
}

CHookID3DXEffectMMEOBJRenderer::CHookID3DXEffectMMEOBJRenderer(::ID3DXEffect *pEffect)
{
	this->hCameraOffset = NULL;
	this->hFocusOffset = NULL;
	this->ClearCameraPos();
	this->ClearFocusPos();

	if( pEffect ) {
		this->pOriginal = pEffect;
		this->AddRef();

		D3DXHANDLE handle;

		handle = this->pOriginal->GetParameterByName(NULL, MMEHACK_EFFECT_CAMERAOFFSET);
		if( handle ) {
			this->hCameraOffset = handle;
		}
		else {
			this->hCameraOffset = NULL;
		}

		handle = this->pOriginal->GetParameterByName(NULL, MMEHACK_EFFECT_FOCUSOFFSET);
		if( handle ) {
			this->hFocusOffset = handle;
		}
		else {
			this->hFocusOffset = NULL;
		}
	}
}

CHookID3DXEffectMMEOBJRenderer::~CHookID3DXEffectMMEOBJRenderer()
{
	/* pOriginal->Release() は CHookID3DXEffect::~CHookID3DXEffect で実施 */
}

// CHookID3DXEffectMMEOBJRenderer::IUnknown
HRESULT	STDMETHODCALLTYPE CHookID3DXEffectMMEOBJRenderer::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_ID3DXEffect ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

HRESULT STDMETHODCALLTYPE CHookID3DXEffectMMEOBJRenderer::CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect)
{
	HRESULT hr;
	ID3DXEffect *p;
	CHookID3DXEffectMMEOBJRenderer *pRenderer = NULL;

	hr = this->pOriginal->CloneEffect(pDevice, &p);
	if( SUCCEEDED(hr) ) {
		pRenderer = new CHookID3DXEffectMMEOBJRenderer(p);
		if( !pRenderer ) {
			*ppEffect = p;
		}
		else if( FAILED(pRenderer->QueryInterface(IID_ID3DXEffect, (void **)ppEffect)) ) {
			*ppEffect = p;
			delete pRenderer;
		}
		else {
			p->Release();
			g_vecMirrorRenderer.push_back(pRenderer);
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CHookID3DXEffectMMEOBJRenderer::Begin(UINT *pPasses, DWORD Flags)
{
#if 0
	// 微妙に冗長だが、SetCameraPos/SetFocusPosの呼び出し箇所を可変にするため
	// (でも他スレッドからSetすると不都合だし結局ここで設定せざるを得ない？要検討)
	this->SetCameraPos(g_nMouseX_RSum, g_nMouseY_RSum);
	this->SetFocusPos(g_nMouseX_LSum, g_nMouseY_LSum);

	if( this->hCameraOffset )
		this->pOriginal->SetIntArray(this->hCameraOffset, this->nArrayCamera, 2);
	if( this->hFocusOffset )
		this->pOriginal->SetIntArray(this->hFocusOffset, this->nArrayFocus, 2);
#endif
	return this->pOriginal->Begin(pPasses, Flags);
}


#ifdef OVR_ENABLE
CHookID3DXEffectOVRRenderer::CHookID3DXEffectOVRRenderer(::ID3DXEffect *pEffect)
{
	this->hViewMatrix = NULL;
	this->hProjMatrix = NULL;

	if( pEffect ) {
		this->pOriginal = pEffect;
		this->AddRef();

		D3DXHANDLE handle;

		handle = this->pOriginal->GetParameterBySemantic(NULL, MMEHACK_EFFECT_SEMANTIC_OVR_VIEW);
		if( handle ) {
			this->hViewMatrix = handle;
		}
		else {
			this->hViewMatrix = NULL;
		}
		handle = this->pOriginal->GetParameterBySemantic(NULL, MMEHACK_EFFECT_SEMANTIC_OVR_PROJECTION);
		if( handle ) {
			this->hProjMatrix = handle;
		}
		else {
			this->hProjMatrix = NULL;
		}
	}
}

CHookID3DXEffectOVRRenderer::~CHookID3DXEffectOVRRenderer()
{
	/* pOriginal->Release() は CHookID3DXEffect::~CHookID3DXEffect で実施 */
}

// CHookID3DXEffectOVRRenderer::IUnknown
HRESULT	STDMETHODCALLTYPE CHookID3DXEffectOVRRenderer::QueryInterface(REFIID riid, void** ppvObject)
{
	if( riid == IID_ID3DXEffect ) {
		*ppvObject = this;
		this->AddRef();
		return S_OK;
	}
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

HRESULT STDMETHODCALLTYPE CHookID3DXEffectOVRRenderer::CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect)
{
	HRESULT hr;
	ID3DXEffect *p;
	CHookID3DXEffectOVRRenderer *pRenderer = NULL;

	hr = this->pOriginal->CloneEffect(pDevice, &p);
	if( SUCCEEDED(hr) ) {
		pRenderer = new CHookID3DXEffectOVRRenderer(p);
		if( !pRenderer ) {
			*ppEffect = p;
		}
		else if( FAILED(pRenderer->QueryInterface(IID_ID3DXEffect, (void **)ppEffect)) ) {
			*ppEffect = p;
			delete pRenderer;
		}
		else {
			p->Release();
			int nEye = this->GetOVREye();
			pRenderer->SetOVREye(nEye);
			pRenderer->SetProjMatrix(&g_matOVREyeProj[nEye]);
			g_vecOVREyeRT[this->nOVREye].push_back(pRenderer);
		}
	}

	return hr;
}

extern D3DXMATRIX matEyeView[OVR_EYE_NUM];
extern BOOL bSync;

HRESULT STDMETHODCALLTYPE CHookID3DXEffectOVRRenderer::Begin(UINT *pPasses, DWORD Flags)
{
	static D3DXMATRIX matEye[OVR_EYE_NUM];

	if( bSync ) {
		matEye[OVR_EYE_LEFT] = matEyeView[OVR_EYE_LEFT];
		matEye[OVR_EYE_RIGHT] = matEyeView[OVR_EYE_RIGHT];
		bSync = FALSE;
	}

	int nEye = this->nOVREye;

	if( this->hViewMatrix ) {
		this->pOriginal->SetMatrix(this->hViewMatrix, &matEye[nEye]);
	}

	return this->pOriginal->Begin(pPasses, Flags);
}

HRESULT STDMETHODCALLTYPE CHookID3DXEffectOVRRenderer::SetProjMatrix(D3DXMATRIX *matProj)
{
	HRESULT hr = S_OK;

	if( this->hProjMatrix ) {
		hr = this->pOriginal->SetMatrix(this->hProjMatrix, matProj);
	}
	else {
		hr = E_INVALIDARG;
	}

	return hr;
}

#endif
