#include "stdafx.h"


tDirect3DCreate9 g_orgDirect3DCreate9;
#ifdef D3D9EX_ENABLE
tDirect3DCreate9Ex DynDirect3DCreate9Ex;
#endif

tD3DXCreateEffectFromResourceA g_orgD3DXCreateEffectFromResourceA;
tD3DXCreateTexture g_orgD3DXCreateTexture;
tD3DXCreateTextureFromFileExA g_orgD3DXCreateTextureFromFileExA;
tD3DXCreateTextureFromFileExW g_orgD3DXCreateTextureFromFileExW;
tD3DXCreateTextureFromFileInMemoryEx g_orgD3DXCreateTextureFromFileInMemoryEx;
#ifdef D3D9EX_ENABLE
tD3DXLoadMeshFromXInMemory g_orgD3DXLoadMeshFromXInMemory;
tD3DXLoadMeshFromXW g_orgD3DXLoadMeshFromXW;
#endif



IDirect3D9 * WINAPI Hook_Direct3DCreate9(UINT SDKVersion)
{
	IDirect3D9 *pD3D = NULL,*pD3DHooked = NULL;

#ifndef D3D9EX_ENABLE
	pD3D = g_orgDirect3DCreate9(SDKVersion);
#else
	/* さすがにコレはだめ？→いける！ */
	IDirect3D9Ex *pD3DEx = NULL;
	if( g_bMMEHacked ) {
		pD3D = g_orgDirect3DCreate9(SDKVersion);
	}
	else {
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
	}

#endif
	if( !pD3D ) {
		return NULL;
	}

	pHookDirect3D9 = new CHookIDirect3D9MMD(pD3D);
	if( FAILED(pHookDirect3D9->QueryInterface(IID_IDirect3D9, (void **)&pD3DHooked)) ) {
		pD3DHooked = pD3D;
		delete pHookDirect3D9;
		pHookDirect3D9 = NULL;
	}
	else
	{
		pD3D->Release();
	}

	// メインスレッド再開
	ResumeThread(g_hMainProcThread);

	return pD3DHooked;
}


HRESULT WINAPI Hook_D3DXCreateTexture(
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
	return g_orgD3DXCreateTexture(pDevice, Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
}

HRESULT WINAPI
    Hook_D3DXCreateTextureFromFileExA(
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
	return g_orgD3DXCreateTextureFromFileExA(
		pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool,
		Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
}

HRESULT WINAPI Hook_D3DXCreateTextureFromFileExW(
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
	return g_orgD3DXCreateTextureFromFileExW(
		pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool,
		Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
}

HRESULT WINAPI Hook_D3DXCreateTextureFromFileInMemoryEx(
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
	return g_orgD3DXCreateTextureFromFileInMemoryEx(
		pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels,
		Usage, Format, Pool, Filter, MipFilter, ColorKey,
		pSrcInfo, pPalette, ppTexture);
}

HRESULT WINAPI Hook_D3DXCreateEffectFromResourceA(
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
	ID3DXEffect *pEffect;

	hr = g_orgD3DXCreateEffectFromResourceA(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags, pPool, &pEffect, ppCompilationErrors);

#if 0
	pHookEffect = new CHookID3DXEffectMMD(pEffect);

	if( FAILED(pHookEffect->QueryInterface(IID_ID3DXEffect, (void **)ppEffect) ) ) {
		*ppEffect = pEffect;
		delete pHookEffect;
		pHookEffect = NULL;
	}
	else {
		pEffect->Release();
	}
#else
	*ppEffect = pEffect;
#endif

	return hr;
}

#ifdef D3D9EX_ENABLE
HRESULT WINAPI Hook_D3DXLoadMeshFromXInMemory(
        LPCVOID Memory,
        DWORD SizeOfMemory,
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh)
{
	Options &= ~(D3DXMESH_MANAGED);
//	Options |= D3DXMESH_DYNAMIC | D3DXMESH_WRITEONLY;
	return g_orgD3DXLoadMeshFromXInMemory(Memory, SizeOfMemory, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, ppMesh);;
}

HRESULT WINAPI Hook_D3DXLoadMeshFromXW(
        LPCWSTR pFilename, 
        DWORD Options, 
        LPDIRECT3DDEVICE9 pD3DDevice, 
        LPD3DXBUFFER *ppAdjacency,
        LPD3DXBUFFER *ppMaterials, 
        LPD3DXBUFFER *ppEffectInstances, 
        DWORD *pNumMaterials,
        LPD3DXMESH *ppMesh)
{
	Options &= ~(D3DXMESH_MANAGED);
//	Options |= D3DXMESH_DYNAMIC | D3DXMESH_WRITEONLY;
	return g_orgD3DXLoadMeshFromXW(pFilename, Options, pD3DDevice, ppAdjacency, ppMaterials, ppEffectInstances, pNumMaterials, ppMesh);
}
#endif

