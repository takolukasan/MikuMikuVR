/******************************************************************************/
/* MMDMirrorView                                                              */
/*                                                                            */
/* ファイル名：MMEHack_Effect.cpp                                             */
/* 作成：たこルカさん                                                         */
/* 説明：MMEからD3DX関数コールのフックとエフェクト操作のフック                */
/*                                                                            */
/******************************************************************************/

#ifndef _MMEHACK_EFFECT_H
#define _MMEHACK_EFFECT_H


/* MMD(MME)からのD3DX関数コールはx86:ANSI/x64:Unicode(ロジック側で対応) まさに外道 */
#define MMEHACK_EFFECT_RENDERTARGET	"MMDOVRCamera.fx"

#define MMEHACK_EFFECT_OVRRENDERL	"MMDOVRRenderL.fx"
#define MMEHACK_EFFECT_OVRRENDERR	"MMDOVRRenderR.fx"

#define MMEHACK_EFFECT_EYETEXTURENAMEL	"RTEyeViewL"
#define MMEHACK_EFFECT_EYETEXTURENAMER	"RTEyeViewR"


#define MMEHACK_EFFECT_CAMERAOFFSET	"CameraOffset"
#define MMEHACK_EFFECT_FOCUSOFFSET	"FocusOffset"

#define MMEHACK_EFFECT_SEMANTIC_OVR_VIEW "MMDOVR_VIEW"
#define MMEHACK_EFFECT_SEMANTIC_OVR_PROJECTION "MMDOVR_PROJECTION"


#define MMEHACK_EFFECT_VIEWTYPE		"MMDOVR_ViewType"
#define MMEHACK_EFFECT_VIEWEYE		"MMDOVR_ViewEye"


// 数学定数・関数定義
#define M_PI			(3.141592654)
#define DEG2RAD(deg)	(M_PI * (deg) / 180.0)
#define RAD2DEG(rad)	((rad) * 180.0 / M_PI)


// 共通定数
#define MMEHACK_VIEWTYPE_DEFAULT	(0)		// ビュー行列・プロジェクション行列をを書き換えない
#define MMEHACK_VIEWTYPE_VIEWPROJ	(1)		// ビュー行列・プロジェクションを書き換える
#define MMEHACK_VIEWTYPE_PROJONLY	(2)		// プロジェクション行列のみを書き換える(ビュー行列はMMDのカメラを使う)


#define MMEHACK_VIEWEYE_LEFT		(0)
#define MMEHACK_VIEWEYE_RIGHT		(1)


// フックする関数の定義
// Import from d3dx9_**.dll

// x86: ANSI x64: Unicodeでインポートしているとかいう、まさに外道
#ifndef X64
typedef HRESULT (WINAPI *tD3DXCreateEffectFromFileA)(LPDIRECT3DDEVICE9, LPCSTR, CONST D3DXMACRO*, LPD3DXINCLUDE,
	DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
extern tD3DXCreateEffectFromFileA g_orgMMEffectD3DXCreateEffectFromFileA;
HRESULT WINAPI MMEffectHack_D3DXCreateEffectFromFileA(
        LPDIRECT3DDEVICE9               pDevice,
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
#else
typedef HRESULT (WINAPI *tD3DXCreateEffectFromFileW)(LPDIRECT3DDEVICE9, LPCWSTR, CONST D3DXMACRO*, LPD3DXINCLUDE,
	DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
extern tD3DXCreateEffectFromFileW g_orgMMEffectD3DXCreateEffectFromFileW;
HRESULT WINAPI MMEffectHack_D3DXCreateEffectFromFileW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
#endif



// フックするID3DXEffectインターフェースの実装

// RT
class CHookID3DXEffectMMEMirrorRT : public CHookID3DXEffect
{
private:
	IDirect3DSurface9 *pSurfRT;

#ifdef OVR_ENABLE
	D3DXHANDLE hEyeRT[OVR_EYE_NUM];
	IDirect3DSurface9 *pSurfRTEye[OVR_EYE_NUM];
	D3DSURFACE_DESC RTEyeTexDesc[OVR_EYE_NUM];
#endif

public:
	CHookID3DXEffectMMEMirrorRT(::ID3DXEffect *pEffect);
	~CHookID3DXEffectMMEMirrorRT();

	IDirect3DSurface9 * GetRTSurface() { return this->pSurfRT; }

#ifdef OVR_ENABLE
	IDirect3DSurface9 * GetEyeSurface(int eye) { return this->pSurfRTEye[eye]; }
	D3DSURFACE_DESC * GetRTEyeTexDesc(int eye) { return &(this->RTEyeTexDesc[eye]); }
#endif

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** ID3DXEffect methods ***/
    virtual HRESULT STDMETHODCALLTYPE SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture);
};


#ifdef OVR_ENABLE
class CHookID3DXEffectOVRRenderer : public CHookID3DXEffect
{
private:
	int nOVREye;
	int nViewType;
	D3DXHANDLE hViewMatrix;
	D3DXHANDLE hProjMatrix;
	D3DXHANDLE hViewType;

public:
	CHookID3DXEffectOVRRenderer(::ID3DXEffect *pEffect);
	~CHookID3DXEffectOVRRenderer();

	void STDMETHODCALLTYPE SetOVREye(int nEye) { this->nOVREye = nEye; };
	int STDMETHODCALLTYPE GetOVREye() { return this->nOVREye; }
	HRESULT STDMETHODCALLTYPE SetProjMatrix(const D3DXMATRIX *matProj);

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** ID3DXEffect methods ***/
    virtual HRESULT STDMETHODCALLTYPE CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect);
	virtual HRESULT STDMETHODCALLTYPE Begin(UINT *pPasses, DWORD Flags);
};
#endif


/* Direct3DX9 実装インターフェースポインタ */
extern CHookID3DXEffectMMEMirrorRT *g_pMMEHookMirrorRT;


#ifdef OVR_ENABLE
extern std::vector<CHookID3DXEffectOVRRenderer *> g_vecOVREyeRT[OVR_EYE_NUM];
#endif



#endif // _MMEHACK_EFFECT_H
