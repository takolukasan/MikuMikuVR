#ifndef _D3D9_HOOK_H
#define _D3D9_HOOK_H



#define RELEASE(p) if(p) { (p)->Release(); (p) = NULL; }



#if 0
#ifdef OVR_ENABLE

#ifdef MIRROR_RENDER
#define MMD_RENDER_REPEAT_COUNT			(3)
#define MMD_RENDER_REPEATSTAGE_ORIGINAL	(0)
#define MMD_RENDER_REPEATSTAGE_OVR		(1)
#define MMD_RENDER_REPEATSTAGE_MIRROR	(2)
#else
#define MMD_RENDER_REPEAT_COUNT			(1)
#define MMD_RENDER_REPEATSTAGE_ORIGINAL	(1)
#endif


#else


#ifdef MIRROR_RENDER
#define MMD_RENDER_REPEAT_COUNT			(2)
#define MMD_RENDER_REPEATSTAGE_ORIGINAL	(0)
#define MMD_RENDER_REPEATSTAGE_MIRROR	(1)
#else
#define MMD_RENDER_REPEAT_COUNT			(1)
#define MMD_RENDER_REPEATSTAGE_ORIGINAL	(1)
#endif

#endif

#endif

/* 0-5まで g_matRotateViewPoints[] のインデックスと一致させる */
enum VIEW_POINTS {
	VIEW_POINT_LEFT			= 0,
	VIEW_POINT_RIGHT		= 1,
	VIEW_POINT_TOP			= 2,
	VIEW_POINT_BOTTOM		= 3,
	VIEW_POINT_OPPOSITE		= 4,
	VIEW_POINT_ORIGINAL		= 5,
	VIEW_POINT_SELECT_MAX,

	VIEW_POINT_FIX			= 10
};


/* dllmain.cpp */
extern HMODULE g_hModMyLibrary;
extern BOOL g_bMMDHacked;
extern BOOL g_bMMEHacked;


// スレッドと同期用オブジェクト
extern HANDLE g_hMainProcThread;
extern DWORD g_dwMainProcThreadId;
extern HANDLE g_hMutexD3DCreateBlock;
extern HANDLE g_hSemaphoreMMDInitBlock;
extern HANDLE g_hSemaphoreMMDShutdownBlock;
#ifdef OVR_ENABLE
extern HANDLE g_hSemaphoreOVRRenderSync;
extern HANDLE g_hSemaphoreMMDRenderSync;
#endif



extern VIEW_POINTS g_ViewPoint;


/* Direct3D9 インターフェース */
extern IDirect3DSurface9 *g_pPrimaryBackBuffer;
extern IDirect3DSurface9 *g_pPrimaryDepthStencil;
extern IDirect3DSwapChain9 *g_pMirSwapChain;
extern IDirect3DSurface9 *g_pMirBackBuffer;
extern IDirect3DSurface9 *g_pMirDepthStencil;


#ifdef OVR_ENABLE
extern IDirect3DTexture9 *g_pEyeTex[OVR_EYE_NUM];
extern IDirect3DSurface9 *g_pEyeSurf[OVR_EYE_NUM];
extern IDirect3DSurface9 *g_pEyeDepth[OVR_EYE_NUM];
#endif


/* Direct3D共有リソース用ハンドル */
#ifdef D3D9EX_ENABLE
#ifdef OVR_ENABLE
extern HANDLE g_hEyeTexShareHandle[OVR_EYE_NUM];
#endif
#endif

extern DWORD WINAPI MainProc(LPVOID lpParameter);

extern D3DVIEWPORT9 g_vpMirror;

class CHookIDirect3D9MMD : public CHookIDirect3D9
{
private:

protected:

public:
	CHookIDirect3D9MMD(::IDirect3D9 *pD3D);
	~CHookIDirect3D9MMD();

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	virtual ULONG STDMETHODCALLTYPE Release();

	/*** IDirect3D9 methods ***/
	virtual HRESULT STDMETHODCALLTYPE CreateDevice(
		UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface);

};

class CHookIDirect3DDevice9MMD : public CHookIDirect3DDevice9
{
private:
	BOOL bShadowTarget;
	BOOL bResizeFlag;

public:
	CHookIDirect3DDevice9MMD(::IDirect3DDevice9 *pDevice);
	~CHookIDirect3DDevice9MMD();

	void TriggerWindowResize() {
		bResizeFlag = TRUE;
	}

	/*** IUnknown methods ***/
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** IDirect3DDevice9 methods ***/
    virtual HRESULT STDMETHODCALLTYPE BeginScene();
    virtual HRESULT STDMETHODCALLTYPE EndScene();
	virtual HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
	virtual HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
	virtual HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
	virtual HRESULT STDMETHODCALLTYPE SetViewport(CONST D3DVIEWPORT9* pViewport);
	virtual HRESULT STDMETHODCALLTYPE SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
};

class CHookIDirect3DVertexBuffer9MMD : public CHookIDirect3DVertexBuffer9
{
private:
protected:

public:
	CHookIDirect3DVertexBuffer9MMD(::IDirect3DVertexBuffer9 *pD3DVB);
	~CHookIDirect3DVertexBuffer9MMD();

	/*** IUnknown methods ***/
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj);

	/*** IDirect3DResource9 methods ***/
	virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
};

class CHookID3DXEffectMMD : public CHookID3DXEffect
{
private:
	D3DXMATRIX matViewPoint; /* 初期化は g_ViewPoint == VIEW_POINT_ORIGINAL であることを期待 */

public:
	CHookID3DXEffectMMD(::ID3DXEffect *pEffect);
	~CHookID3DXEffectMMD();

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** ID3DXEffect methods ***/
	virtual HRESULT STDMETHODCALLTYPE SetMatrix(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix);
};


extern CHookIDirect3D9MMD *pHookDirect3D9;
extern CHookIDirect3DDevice9MMD *pHookDirect3DDevice9;
extern CHookID3DXEffectMMD *pHookEffect;



#endif // _D3D9_HOOK_H
