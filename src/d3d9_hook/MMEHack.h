#ifndef _MMEHACK_H
#define _MMEHACK_H


extern BOOL WINAPI MMEHack();


// Import from d3d9.dll
extern tDirect3DCreate9 g_orgMMEDirect3DCreate9;
extern IDirect3D9 * WINAPI MMEHack_Direct3DCreate9(UINT SDKVersion);

// Import from d3dx9_**.dll
extern tD3DXCreateTexture g_orgMMEffectD3DXCreateTexture;
HRESULT WINAPI MMEffectHack_D3DXCreateTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE9*       ppTexture);

extern tD3DXCreateTextureFromFileExA g_orgMMEffectD3DXCreateTextureFromFileExA;
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
        LPDIRECT3DTEXTURE9*       ppTexture);


extern tD3DXCreateTexture g_orgD3DXCreateTexture;
HRESULT WINAPI MMEHack_D3DXCreateTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE9*       ppTexture);

extern tD3DXCreateTextureFromFileExA g_orgMMED3DXCreateTextureFromFileExA;
HRESULT WINAPI MMEHack_D3DXCreateTextureFromFileExA(
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
        LPDIRECT3DTEXTURE9*       ppTexture);


extern tD3DXCreateTextureFromFileExW g_orgMMED3DXCreateTextureFromFileExW;
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
        LPDIRECT3DTEXTURE9*       ppTexture);


extern tD3DXCreateTextureFromFileInMemoryEx g_orgMMED3DXCreateTextureFromFileInMemoryEx;
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
        LPDIRECT3DTEXTURE9*       ppTexture);


extern tD3DXCreateEffectFromResourceA g_orgMMED3DXCreateEffectFromResourceA;
HRESULT WINAPI MMEHack_D3DXCreateEffectFromResourceA(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);




class CHookIDirect3D9MME : public CHookIDirect3D9
{
private:

protected:

public:
	CHookIDirect3D9MME(::IDirect3D9 *pD3D);
	~CHookIDirect3D9MME();

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	virtual ULONG STDMETHODCALLTYPE Release();

	/*** IDirect3D9 methods ***/
	virtual HRESULT STDMETHODCALLTYPE CreateDevice(
		UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface);

};


class CHookIDirect3DDevice9MME : public CHookIDirect3DDevice9
{
private:
	/* Direct3D9 インターフェース */
	IDirect3DSwapChain9 *pSwapChainMME;

	D3DXMATRIX matEyeView[OVR_EYE_NUM];
	D3DXMATRIX matProjection[OVR_EYE_NUM];


public:
	CHookIDirect3DDevice9MME(::IDirect3DDevice9 *pDevice);
	~CHookIDirect3DDevice9MME();

	/*** IUnknown methods ***/
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** IDirect3DDevice9 methods ***/
	virtual HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE BeginScene();
    virtual HRESULT STDMETHODCALLTYPE EndScene();
    virtual HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);

	const D3DXMATRIX * GetEyeViewMatrix(ovrEyeType eye) {
		return &this->matEyeView[eye];
	}
	const D3DXMATRIX * GetProjectionMatrix(ovrEyeType eye) {
		return &this->matProjection[eye];
	}

};


/* Direct3D9 実装インターフェースポインタ */
extern CHookIDirect3D9MME *pMMEHookDirect3D9;
extern CHookIDirect3DDevice9MME *pMMEHookDirect3DDevice9;



#endif // _MMEHACK_H
