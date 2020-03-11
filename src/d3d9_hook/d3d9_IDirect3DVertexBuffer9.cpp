#include "stdafx.h"


CHookIDirect3DVertexBuffer9::CHookIDirect3DVertexBuffer9()
{
	this->pOriginal = NULL;
}

CHookIDirect3DVertexBuffer9::CHookIDirect3DVertexBuffer9(IDirect3DVertexBuffer9 *pD3DVB)
{
	this->ulRefCnt = 1;
	if( pD3DVB ) {
		this->pOriginal = pD3DVB;
		this->ulRefCnt = this->pOriginal->AddRef();
	}
}

CHookIDirect3DVertexBuffer9::~CHookIDirect3DVertexBuffer9()
{
	if( this->pOriginal ) {
		this->pOriginal->Release();
		this->pOriginal = NULL;
	}
}

// CHookIDirect3DVertexBuffer9::IUnknown
HRESULT	STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::QueryInterface(REFIID riid, void** ppvObj)
{
	return this->pOriginal->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::AddRef()
{
	this->ulRefCnt = this->pOriginal->AddRef();
    return this->ulRefCnt;
}

ULONG STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::Release()
{
	this->ulRefCnt = this->pOriginal->Release();
    return this->ulRefCnt;
}

// CHookIDirect3DVertexBuffer9::IDirect3D9VertexBuffer9
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::GetDevice(IDirect3DDevice9** ppDevice)
{ return this->pOriginal->GetDevice(ppDevice); }
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{ return this->pOriginal->SetPrivateData(refguid, pData, SizeOfData, Flags); }
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{ return this->pOriginal->GetPrivateData(refguid, pData, pSizeOfData); }
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::FreePrivateData(REFGUID refguid)
{ return this->pOriginal->FreePrivateData(refguid); }
DWORD STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::SetPriority(DWORD PriorityNew)
{ return this->pOriginal->SetPriority(PriorityNew); }
DWORD STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::GetPriority()
{ return this->pOriginal->GetPriority(); }
void STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::PreLoad()
{ return this->pOriginal->PreLoad(); }
D3DRESOURCETYPE STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::GetType()
{ return this->pOriginal->GetType(); }
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{ return this->pOriginal->Lock(OffsetToLock, SizeToLock, ppbData, Flags); }
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::Unlock()
{ return this->pOriginal->Unlock(); }
HRESULT STDMETHODCALLTYPE CHookIDirect3DVertexBuffer9::GetDesc(D3DVERTEXBUFFER_DESC *pDesc)
{ return this->pOriginal->GetDesc(pDesc); }

