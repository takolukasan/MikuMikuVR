#include "stdafx.h"


CHookIDirect3DDevice9::CHookIDirect3DDevice9()
{
	this->pOriginal = NULL;
}

CHookIDirect3DDevice9::CHookIDirect3DDevice9(IDirect3DDevice9 *pDevice)
{
	this->ulRefCnt = 1;
	if( pDevice ) {
		this->pOriginal = pDevice;
		this->ulRefCnt = this->pOriginal->AddRef();
	}
}

CHookIDirect3DDevice9::~CHookIDirect3DDevice9()
{
	if( this->pOriginal ) {
		this->pOriginal->Release();
		this->pOriginal = NULL;
	}
}

// CHookIDirect3DDevice9::IUnknown
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObject)
{
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

ULONG STDMETHODCALLTYPE CHookIDirect3DDevice9::AddRef()
{
	this->ulRefCnt = this->pOriginal->AddRef();
    return this->ulRefCnt;
}

ULONG STDMETHODCALLTYPE CHookIDirect3DDevice9::Release()
{
	this->ulRefCnt = this->pOriginal->Release();
    return this->ulRefCnt;
}

// IDirect3DDevice9
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::TestCooperativeLevel()
{ return this->pOriginal->TestCooperativeLevel(); }
UINT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetAvailableTextureMem()
{ return this->pOriginal->GetAvailableTextureMem(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::EvictManagedResources()
{ return this->pOriginal->EvictManagedResources(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{ return this->pOriginal->GetDirect3D(ppD3D9); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{ return this->pOriginal->GetDeviceCaps(pCaps); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{ return this->pOriginal->GetDisplayMode(iSwapChain, pMode); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{ return this->pOriginal->GetCreationParameters(pParameters); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{ return this->pOriginal->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap); }
void	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{ return this->pOriginal->SetCursorPosition(X, Y, Flags); }
BOOL	STDMETHODCALLTYPE CHookIDirect3DDevice9::ShowCursor(BOOL bShow)
{ return this->pOriginal->ShowCursor(bShow); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
{ return this->pOriginal->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{ return this->pOriginal->GetSwapChain(iSwapChain, pSwapChain); }
UINT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetNumberOfSwapChains()
{ return this->pOriginal->GetNumberOfSwapChains(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{ return this->pOriginal->Reset(pPresentationParameters); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{ return this->pOriginal->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{ return this->pOriginal->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{ return this->pOriginal->GetRasterStatus(iSwapChain, pRasterStatus); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{ return this->pOriginal->SetDialogBoxMode(bEnableDialogs); }
void	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{ return this->pOriginal->SetGammaRamp(iSwapChain, Flags, pRamp); }
void	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{ return this->pOriginal->GetGammaRamp(iSwapChain, pRamp); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{ return this->pOriginal->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{ return this->pOriginal->UpdateTexture(pSourceTexture, pDestinationTexture); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{ return this->pOriginal->GetRenderTargetData(pRenderTarget, pDestSurface); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{ return this->pOriginal->GetFrontBufferData(iSwapChain, pDestSurface); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{ return this->pOriginal->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{ return this->pOriginal->ColorFill(pSurface, pRect, color); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{ return this->pOriginal->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{ return this->pOriginal->SetRenderTarget(RenderTargetIndex, pRenderTarget); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{ return this->pOriginal->GetRenderTarget(RenderTargetIndex, ppRenderTarget); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ return this->pOriginal->SetDepthStencilSurface(pNewZStencil); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ return this->pOriginal->GetDepthStencilSurface(ppZStencilSurface); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::BeginScene()
{ return this->pOriginal->BeginScene(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::EndScene()
{ return this->pOriginal->EndScene(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{ return this->pOriginal->Clear(Count, pRects, Flags, Color, Z, Stencil); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{ return this->pOriginal->SetTransform(State, pMatrix); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{ return this->pOriginal->GetTransform(State, pMatrix); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX * pMatrix)
{ return this->pOriginal->MultiplyTransform(State, pMatrix); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{ return this->pOriginal->SetViewport(pViewport); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{ return this->pOriginal->GetViewport(pViewport); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{ return this->pOriginal->SetMaterial(pMaterial); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{ return this->pOriginal->GetMaterial(pMaterial); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9 *pLight)
{ return this->pOriginal->SetLight(Index, pLight); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9 *pLight)
{ return this->pOriginal->GetLight(Index, pLight); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{ return this->pOriginal->LightEnable(Index, Enable); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{ return this->pOriginal->GetLightEnable(Index, pEnable); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{ return this->pOriginal->SetClipPlane(Index, pPlane); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{ return this->pOriginal->GetClipPlane(Index, pPlane); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{ return this->pOriginal->SetRenderState(State, Value); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{ return this->pOriginal->GetRenderState(State, pValue); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{ return this->pOriginal->CreateStateBlock(Type, ppSB); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::BeginStateBlock()
{ return this->pOriginal->BeginStateBlock(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{ return this->pOriginal->EndStateBlock(ppSB); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{ return this->pOriginal->SetClipStatus(pClipStatus); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{ return this->pOriginal->GetClipStatus(pClipStatus); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{ return this->pOriginal->GetTexture(Stage, ppTexture); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{ return this->pOriginal->SetTexture(Stage, pTexture); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{ return this->pOriginal->GetTextureStageState(Stage, Type, pValue); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{ return this->pOriginal->SetTextureStageState(Stage, Type, Value); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{ return this->pOriginal->GetSamplerState(Sampler, Type, pValue); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{ return this->pOriginal->SetSamplerState(Sampler, Type, Value); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{ return this->pOriginal->ValidateDevice(pNumPasses); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{ return this->pOriginal->SetPaletteEntries(PaletteNumber, pEntries); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{ return this->pOriginal->GetPaletteEntries(PaletteNumber, pEntries); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{ return this->pOriginal->SetCurrentTexturePalette(PaletteNumber); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{ return this->pOriginal->GetCurrentTexturePalette(PaletteNumber); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{ return this->pOriginal->SetScissorRect(pRect); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetScissorRect(RECT* pRect)
{ return this->pOriginal->GetScissorRect(pRect); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{ return this->pOriginal->SetSoftwareVertexProcessing(bSoftware); }
BOOL	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetSoftwareVertexProcessing()
{ return this->pOriginal->GetSoftwareVertexProcessing(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetNPatchMode(float nSegments)
{ return this->pOriginal->SetNPatchMode(nSegments); }
float	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetNPatchMode()
{ return this->pOriginal->GetNPatchMode(); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{ return this->pOriginal->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT PrimitiveCount)
{ return this->pOriginal->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{ return this->pOriginal->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{ return this->pOriginal->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{ return this->pOriginal->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{ return this->pOriginal->CreateVertexDeclaration(pVertexElements, ppDecl); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{ return this->pOriginal->SetVertexDeclaration(pDecl); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{ return this->pOriginal->GetVertexDeclaration(ppDecl); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetFVF(DWORD FVF)
{ return this->pOriginal->SetFVF(FVF); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetFVF(DWORD* pFVF)
{ return this->pOriginal->GetFVF(pFVF); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{ return this->pOriginal->CreateVertexShader(pFunction, ppShader); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{ return this->pOriginal->SetVertexShader(pShader); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{ return this->pOriginal->GetVertexShader(ppShader); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{ return this->pOriginal->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{ return this->pOriginal->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{ return this->pOriginal->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{ return this->pOriginal->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{ return this->pOriginal->SetVertexShaderConstantB(StartRegister, pConstantData,  BoolCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{ return this->pOriginal->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{ return this->pOriginal->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
{ return this->pOriginal->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Setting)
{ return this->pOriginal->SetStreamSourceFreq(StreamNumber, Setting); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* pSetting)
{ return this->pOriginal->GetStreamSourceFreq(StreamNumber, pSetting); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ return this->pOriginal->SetIndices(pIndexData); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{ return this->pOriginal->GetIndices(ppIndexData); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{ return this->pOriginal->CreatePixelShader(pFunction, ppShader); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{ return this->pOriginal->SetPixelShader(pShader); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{ return this->pOriginal->GetPixelShader(ppShader); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{ return this->pOriginal->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{ return this->pOriginal->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{ return this->pOriginal->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{ return this->pOriginal->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{ return this->pOriginal->SetPixelShaderConstantB(StartRegister, pConstantData,  BoolCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{ return this->pOriginal->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{ return this->pOriginal->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{ return this->pOriginal->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::DeletePatch(UINT Handle)
{ return this->pOriginal->DeletePatch(Handle); }
HRESULT	STDMETHODCALLTYPE CHookIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{ return this->pOriginal->CreateQuery(Type, ppQuery); }

