// Windows ヘッダー ファイル:
#include <windows.h>

#define CINTERFACE

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.inl>

#include "Direct3D9_VtblUtil.h"



typedef struct IDirect3DVertexBuffer9Vtbl_Hook {
	IDirect3DVertexBuffer9Vtbl lpVtbl[2];
	IDirect3DVertexBuffer9Vtbl *lpOrgVtbl;
	IDirect3DVertexBuffer9_PrivateData PrivateData;
} IDirect3DVertexBuffer9Vtbl_Hook;


typedef struct IDirect3DTexture9Vtbl_Hook {
	IDirect3DTexture9Vtbl lpVtbl[2];
	IDirect3DTexture9Vtbl *lpOrgVtbl;
} IDirect3DTexture9Vtbl_Hook;



#define DEFINE_GETOFFSET(_interface, _method)							\
	DWORD Hook##_interface##_GetOffset_##_method(_interface *p)			\
{																		\
	_interface##Vtbl *pVtbl = p->lpVtbl;								\
	return (DWORD)((uintptr_t)(&pVtbl->_method) - (uintptr_t)pVtbl);	\
}


static ULONG STDMETHODCALLTYPE Hook_IDirect3DVertexBuffer9_Release(IDirect3DVertexBuffer9 *pthis)
{
	IDirect3DVertexBuffer9Vtbl_Hook *pHook = (IDirect3DVertexBuffer9Vtbl_Hook *)(pthis->lpVtbl);
	ULONG ulRefCnt = pHook->lpOrgVtbl->Release(pthis);
	if( ulRefCnt == 1 ) {
		IDirect3DVertexBuffer9_PrivateData *pPrivateData = HookIDirect3DVertexBuffer9_GetPrivateData(pthis);
		if( pPrivateData->pBuffer ) {
			free(pPrivateData->pBuffer);
			pthis->lpVtbl = pHook->lpOrgVtbl;
			free(pHook);
		}
		ulRefCnt = pthis->lpVtbl->Release(pthis);
	}
    return ulRefCnt;
}

uintptr_t HookIDirect3DVertexBuffer9_Hookvtable(IDirect3DVertexBuffer9 *p, UINT Length)
{
	IDirect3DVertexBuffer9Vtbl_Hook *pRet = (IDirect3DVertexBuffer9Vtbl_Hook *)malloc(sizeof(IDirect3DVertexBuffer9Vtbl_Hook));

	memset(pRet, 0, sizeof(IDirect3DVertexBuffer9Vtbl_Hook));

	(pRet->lpVtbl[0]) = *(p->lpVtbl);
	pRet->lpOrgVtbl = (p->lpVtbl);
	pRet->PrivateData.Length = Length;
	pRet->PrivateData.pBuffer = (BYTE *)malloc(Length);
	if( pRet->PrivateData.pBuffer ) {
		memset(pRet->PrivateData.pBuffer, 0, Length);
	}

	p->lpVtbl = (IDirect3DVertexBuffer9Vtbl *)pRet;
	p->lpVtbl->AddRef(p);
	HookIDirect3DVertexBuffer9_HookMethod(p, (DWORD)((uintptr_t)(&pRet->lpVtbl->Release) - (uintptr_t)pRet->lpVtbl), (uintptr_t)Hook_IDirect3DVertexBuffer9_Release);

	return (uintptr_t)pRet;
}

uintptr_t HookIDirect3DVertexBuffer9_HookMethod(IDirect3DVertexBuffer9 *p, DWORD dwOfs, uintptr_t pFunc)
{
	IDirect3DVertexBuffer9Vtbl_Hook *pHook = (IDirect3DVertexBuffer9Vtbl_Hook *)(p->lpVtbl);
	int nIndex = dwOfs / sizeof(uintptr_t);

	*((uintptr_t *)(&pHook->lpVtbl[1]) + nIndex) = *((uintptr_t *)(&pHook->lpVtbl[0]) + nIndex);
	*((uintptr_t *)(&pHook->lpVtbl[0]) + nIndex) = pFunc;

	return *((uintptr_t *)(&pHook->lpVtbl[1]) + nIndex);
}

uintptr_t HookIDirect3DVertexBuffer9_GetMethod(IDirect3DVertexBuffer9 *p, DWORD dwOfs)
{
	IDirect3DVertexBuffer9Vtbl_Hook *pHook = (IDirect3DVertexBuffer9Vtbl_Hook *)(p->lpVtbl);
	int nIndex = dwOfs / sizeof(uintptr_t);

	return *((uintptr_t *)(&pHook->lpVtbl[1]) + nIndex);
}

IDirect3DVertexBuffer9_PrivateData * HookIDirect3DVertexBuffer9_GetPrivateData(IDirect3DVertexBuffer9 *p)
{
	IDirect3DVertexBuffer9Vtbl_Hook *pHook = (IDirect3DVertexBuffer9Vtbl_Hook *)(p->lpVtbl);
	return &(pHook->PrivateData);
}


DEFINE_GETOFFSET(IDirect3DVertexBuffer9, Lock);
DEFINE_GETOFFSET(IDirect3DVertexBuffer9, Unlock);

