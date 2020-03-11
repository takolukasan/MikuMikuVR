#ifndef _D3D9_IDIRECT3DVERTEXBUFFER9_H
#define _D3D9_IDIRECT3DVERTEXBUFFER9_H

class CHookIDirect3DVertexBuffer9 : IDirect3DVertexBuffer9
{
private:
	ULONG ulRefCnt;

protected:
	IDirect3DVertexBuffer9 *pOriginal;
	CHookIDirect3DVertexBuffer9();

public:
	CHookIDirect3DVertexBuffer9(IDirect3DVertexBuffer9 *pD3DVB);
	~CHookIDirect3DVertexBuffer9();

	ULONG STDMETHODCALLTYPE GetRefCnt() { return this->ulRefCnt; }

	/*** IUnknown methods ***/
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

	/*** IDirect3DResource9 methods ***/
	virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid);
	virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew);
	virtual DWORD STDMETHODCALLTYPE GetPriority();
	virtual void STDMETHODCALLTYPE PreLoad();
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();
	virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE Unlock();
	virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc);

	#ifdef D3D_DEBUG_INFO
	LPCWSTR Name;
	UINT Length;
	DWORD Usage;
	DWORD FVF;
	D3DPOOL Pool;
	DWORD Priority;
	UINT LockCount;
	LPCWSTR CreationCallStack;
	#endif
};

#endif // _D3D9_IDIRECT3DVERTEXBUFFER9_H
