#ifndef _HOOKFUNCTIONS_H
#define _HOOKFUNCTIONS_H

#define D3D9_DLL_NAME		TEXT("d3d9.dll\0")

/* HookImportFunction.cpp */
BOOL HookFunctionOfModule(HMODULE hTargetModule, const char *szTargetFunctionName, void *pHookFunction, void **pTargetFunctionPrevious);


// Import from d3d9.dll
typedef IDirect3D9 * (WINAPI * tDirect3DCreate9)(UINT);
extern tDirect3DCreate9 g_orgDirect3DCreate9;
extern IDirect3D9 * WINAPI Hook_Direct3DCreate9(UINT SDKVersion);

typedef HRESULT (WINAPI * tDirect3DCreate9Ex)(UINT SDKVersion, IDirect3D9Ex**);
extern tDirect3DCreate9Ex DynDirect3DCreate9Ex;


// Import from d3dx9_**.dll
typedef HRESULT (WINAPI *tD3DXCreateTexture)(LPDIRECT3DDEVICE9, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, LPDIRECT3DTEXTURE9*);
extern tD3DXCreateTexture g_orgD3DXCreateTexture;
extern HRESULT WINAPI Hook_D3DXCreateTexture(
        LPDIRECT3DDEVICE9         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE9*       ppTexture);

typedef HRESULT (WINAPI *tD3DXCreateTextureFromFileExA)(
        LPDIRECT3DDEVICE9, LPCSTR, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL,
		DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY*, LPDIRECT3DTEXTURE9*);
extern tD3DXCreateTextureFromFileExA g_orgD3DXCreateTextureFromFileExA;
extern HRESULT WINAPI Hook_D3DXCreateTextureFromFileExA(
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


typedef HRESULT (WINAPI *tD3DXCreateTextureFromFileExW)(
        LPDIRECT3DDEVICE9, LPCWSTR, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL,
		DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY*, LPDIRECT3DTEXTURE9*);
extern tD3DXCreateTextureFromFileExW g_orgD3DXCreateTextureFromFileExW;
extern HRESULT WINAPI Hook_D3DXCreateTextureFromFileExW(
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


typedef HRESULT (WINAPI *tD3DXCreateTextureFromFileInMemoryEx)(
        LPDIRECT3DDEVICE9, LPCVOID, UINT, UINT, UINT, UINT, DWORD, D3DFORMAT,
		D3DPOOL, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY* , LPDIRECT3DTEXTURE9*);
extern tD3DXCreateTextureFromFileInMemoryEx g_orgD3DXCreateTextureFromFileInMemoryEx;
extern HRESULT WINAPI Hook_D3DXCreateTextureFromFileInMemoryEx(
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


typedef HRESULT (WINAPI * tD3DXCreateEffectFromResourceA)(
	LPDIRECT3DDEVICE9, HMODULE, LPCSTR, CONST D3DXMACRO *,
    LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT *,
    LPD3DXBUFFER *);
extern tD3DXCreateEffectFromResourceA g_orgD3DXCreateEffectFromResourceA;
extern HRESULT WINAPI Hook_D3DXCreateEffectFromResourceA(
        LPDIRECT3DDEVICE9               pDevice,
        HMODULE                         hSrcModule,
        LPCSTR                          pSrcResource,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);


#ifdef D3D9EX_ENABLE
typedef HRESULT (WINAPI *tD3DXLoadMeshFromXInMemory)(
        LPCVOID, DWORD, DWORD, LPDIRECT3DDEVICE9, 
        LPD3DXBUFFER *, LPD3DXBUFFER *, LPD3DXBUFFER *, DWORD *,
        LPD3DXMESH *);
extern tD3DXLoadMeshFromXInMemory g_orgD3DXLoadMeshFromXInMemory;
HRESULT WINAPI Hook_D3DXLoadMeshFromXInMemory(
        LPCVOID Memory,
        DWORD SizeOfMemory,
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);


typedef HRESULT (WINAPI *tD3DXLoadMeshFromXW)(
        LPCWSTR, DWORD, LPDIRECT3DDEVICE9 , LPD3DXBUFFER *,
        LPD3DXBUFFER *, LPD3DXBUFFER *, DWORD *,LPD3DXMESH *);
extern tD3DXLoadMeshFromXW g_orgD3DXLoadMeshFromXW;
HRESULT WINAPI Hook_D3DXLoadMeshFromXW(
        LPCWSTR pFilename, 
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh);
#endif


#endif // _HOOKFUNCTIONS_H
