#include "stdafx.h"


CHookIDirect3D9::CHookIDirect3D9()
{
	this->pOriginal = NULL;
}

CHookIDirect3D9::CHookIDirect3D9(IDirect3D9 *pD3D)
{
	this->ulRefCnt = 1;
	if( pD3D ) {
		this->pOriginal = pD3D;
		this->ulRefCnt = this->pOriginal->AddRef();
	}
}

CHookIDirect3D9::~CHookIDirect3D9()
{
	if( this->pOriginal ) {
		this->pOriginal->Release();
		this->pOriginal = NULL;
	}
}

// CHookIDirect3D9::IUnknown
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::QueryInterface(REFIID riid, void** ppvObject)
{
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

ULONG STDMETHODCALLTYPE CHookIDirect3D9::AddRef()
{
	this->ulRefCnt = this->pOriginal->AddRef();
    return this->ulRefCnt;
}

ULONG STDMETHODCALLTYPE CHookIDirect3D9::Release()
{
	this->ulRefCnt = this->pOriginal->Release();
    return this->ulRefCnt;
}

// IDirect3D9
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{ return this->pOriginal->RegisterSoftwareDevice(pInitializeFunction); }
UINT STDMETHODCALLTYPE CHookIDirect3D9::GetAdapterCount()
{ return this->pOriginal->GetAdapterCount(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{ return this->pOriginal->GetAdapterIdentifier(Adapter, Flags, pIdentifier); }
UINT STDMETHODCALLTYPE CHookIDirect3D9::GetAdapterModeCount(UINT Adapter,D3DFORMAT Format)
{ return this->pOriginal->GetAdapterModeCount(Adapter, Format); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::EnumAdapterModes( UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
{ return this->pOriginal->EnumAdapterModes(Adapter, Format, Mode, pMode); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::GetAdapterDisplayMode(UINT Adapter,D3DDISPLAYMODE* pMode)
{ return this->pOriginal->GetAdapterDisplayMode(Adapter, pMode); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::CheckDeviceType(UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
{ return this->pOriginal->CheckDeviceType(Adapter, DevType,AdapterFormat, BackBufferFormat, bWindowed); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{ return this->pOriginal->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
{ return this->pOriginal->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
{ return this->pOriginal->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
{ return this->pOriginal->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
{ return this->pOriginal->GetDeviceCaps(Adapter, DeviceType, pCaps); }
HMONITOR STDMETHODCALLTYPE CHookIDirect3D9::GetAdapterMonitor(UINT Adapter)
{ return this->pOriginal->GetAdapterMonitor(Adapter); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3D9::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{ return this->pOriginal->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface); }


