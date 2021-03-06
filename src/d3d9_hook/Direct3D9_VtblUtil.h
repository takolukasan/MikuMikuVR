#ifndef _DIRECT3D_VTBLUTIL_H
#define _DIRECT3D_VTBLUTIL_H

#ifndef CINTERFACE
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

typedef struct IDirect3DVertexBuffer9_PrivateData {
	// Lock() parameters
	BYTE *pBuffer;
	UINT Length;
	UINT OffsetToLock;
	UINT SizeToLock;
	DWORD Flags;
	BOOL BufferFirstWritten;
	BYTE *_pLockedBuffer;
} IDirect3DVertexBuffer9_PrivateData;

EXTERNC uintptr_t HookIDirect3DVertexBuffer9_Hookvtable(IDirect3DVertexBuffer9 *p, UINT Length);
EXTERNC uintptr_t HookIDirect3DVertexBuffer9_HookMethod(IDirect3DVertexBuffer9 *p, DWORD dwOfs, uintptr_t pFunc);
EXTERNC uintptr_t HookIDirect3DVertexBuffer9_GetMethod(IDirect3DVertexBuffer9 *p, DWORD dwOfs);
EXTERNC IDirect3DVertexBuffer9_PrivateData * HookIDirect3DVertexBuffer9_GetPrivateData(IDirect3DVertexBuffer9 *p);
EXTERNC DWORD HookIDirect3DVertexBuffer9_GetOffset_Lock(IDirect3DVertexBuffer9 *p);
EXTERNC DWORD HookIDirect3DVertexBuffer9_GetOffset_Unlock(IDirect3DVertexBuffer9 *p);

typedef HRESULT (WINAPI * tIDirect3DVertexBuffer9_Lock)(IDirect3DVertexBuffer9 *pthis, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
typedef HRESULT (WINAPI * tIDirect3DVertexBuffer9_Unlock)(IDirect3DVertexBuffer9 *pthis);


typedef struct IDirect3DIndexBuffer9_PrivateData {
	// Lock() parameters
	BYTE *pBuffer;
	UINT Length;
	UINT OffsetToLock;
	UINT SizeToLock;
	DWORD Flags;
	BOOL BufferFirstWritten;
	BYTE *_pLockedBuffer;
} IDirect3DIndexBuffer9_PrivateData;


#endif // _DIRECT3D_VTBLUTIL_H