#ifndef _D3D9_IDIRECT3DDEVICE9_H
#define _D3D9_IDIRECT3DDEVICE9_H

class CHookIDirect3DDevice9 : IDirect3DDevice9
{
private:
	ULONG ulRefCnt;

protected:
	IDirect3DDevice9 *pOriginal;
	CHookIDirect3DDevice9();

public:
	CHookIDirect3DDevice9(IDirect3DDevice9 *pDevice);
	~CHookIDirect3DDevice9();

	ULONG STDMETHODCALLTYPE GetRefCnt() { return this->ulRefCnt; }

    /*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    /*** IDirect3DDevice9 methods ***/
    virtual HRESULT STDMETHODCALLTYPE TestCooperativeLevel();
    virtual UINT	STDMETHODCALLTYPE GetAvailableTextureMem();
    virtual HRESULT STDMETHODCALLTYPE EvictManagedResources();
    virtual HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D9** ppD3D9);
    virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS9* pCaps);
    virtual HRESULT STDMETHODCALLTYPE GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode);
    virtual HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
    virtual HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
    virtual void	STDMETHODCALLTYPE SetCursorPosition(int X,int Y,DWORD Flags);
    virtual BOOL	STDMETHODCALLTYPE ShowCursor(BOOL bShow);
    virtual HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain);
    virtual HRESULT STDMETHODCALLTYPE GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain);
    virtual UINT	STDMETHODCALLTYPE GetNumberOfSwapChains();
    virtual HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
    virtual HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);
    virtual HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
    virtual HRESULT STDMETHODCALLTYPE GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus);
    virtual HRESULT STDMETHODCALLTYPE SetDialogBoxMode(BOOL bEnableDialogs);
    virtual void	STDMETHODCALLTYPE SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
    virtual void	STDMETHODCALLTYPE GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp);
    virtual HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
    virtual HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
    virtual HRESULT STDMETHODCALLTYPE GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
    virtual HRESULT STDMETHODCALLTYPE GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface);
    virtual HRESULT STDMETHODCALLTYPE StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
    virtual HRESULT STDMETHODCALLTYPE ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
    virtual HRESULT STDMETHODCALLTYPE CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
    virtual HRESULT STDMETHODCALLTYPE SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
    virtual HRESULT STDMETHODCALLTYPE GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
    virtual HRESULT STDMETHODCALLTYPE SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
    virtual HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
    virtual HRESULT STDMETHODCALLTYPE BeginScene();
    virtual HRESULT STDMETHODCALLTYPE EndScene();
    virtual HRESULT STDMETHODCALLTYPE Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
    virtual HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
    virtual HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix);
    virtual HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX * pMatrix);
    virtual HRESULT STDMETHODCALLTYPE SetViewport(CONST D3DVIEWPORT9* pViewport);
    virtual HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT9* pViewport);
    virtual HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL9* pMaterial);
    virtual HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL9* pMaterial);
    virtual HRESULT STDMETHODCALLTYPE SetLight(DWORD Index,CONST D3DLIGHT9 *pLight);
    virtual HRESULT STDMETHODCALLTYPE GetLight(DWORD Index,D3DLIGHT9 *pLight);
    virtual HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index,BOOL Enable);
    virtual HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD Index,BOOL* pEnable);
    virtual HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index,CONST float* pPlane);
    virtual HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD Index,float* pPlane);
    virtual HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
    virtual HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue);
    virtual HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB);
    virtual HRESULT STDMETHODCALLTYPE BeginStateBlock();
    virtual HRESULT STDMETHODCALLTYPE EndStateBlock(IDirect3DStateBlock9** ppSB);
    virtual HRESULT STDMETHODCALLTYPE SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus);
    virtual HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS9* pClipStatus);
    virtual HRESULT STDMETHODCALLTYPE GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture);
    virtual HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture);
    virtual HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue);
    virtual HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
    virtual HRESULT STDMETHODCALLTYPE GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
    virtual HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
    virtual HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses);
    virtual HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries);
    virtual HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries);
    virtual HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT PaletteNumber);
    virtual HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT *PaletteNumber);
    virtual HRESULT STDMETHODCALLTYPE SetScissorRect(CONST RECT* pRect);
    virtual HRESULT STDMETHODCALLTYPE GetScissorRect(RECT* pRect);
    virtual HRESULT STDMETHODCALLTYPE SetSoftwareVertexProcessing(BOOL bSoftware);
    virtual BOOL	STDMETHODCALLTYPE GetSoftwareVertexProcessing();
    virtual HRESULT STDMETHODCALLTYPE SetNPatchMode(float nSegments);
    virtual float	STDMETHODCALLTYPE GetNPatchMode();
    virtual HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
    virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE Type,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT PrimitiveCount);
    virtual HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
    virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
    virtual HRESULT STDMETHODCALLTYPE ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags);
    virtual HRESULT STDMETHODCALLTYPE CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
    virtual HRESULT STDMETHODCALLTYPE SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);
    virtual HRESULT STDMETHODCALLTYPE GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl);
    virtual HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF);
    virtual HRESULT STDMETHODCALLTYPE GetFVF(DWORD* pFVF);
    virtual HRESULT STDMETHODCALLTYPE CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
    virtual HRESULT STDMETHODCALLTYPE SetVertexShader(IDirect3DVertexShader9* pShader);
    virtual HRESULT STDMETHODCALLTYPE GetVertexShader(IDirect3DVertexShader9** ppShader);
    virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
    virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount);
    virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
    virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount);
    virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
    virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
    virtual HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
    virtual HRESULT STDMETHODCALLTYPE GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride);
    virtual HRESULT STDMETHODCALLTYPE SetStreamSourceFreq(UINT StreamNumber,UINT Setting);
    virtual HRESULT STDMETHODCALLTYPE GetStreamSourceFreq(UINT StreamNumber,UINT* pSetting);
    virtual HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer9* pIndexData);
    virtual HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer9** ppIndexData);
    virtual HRESULT STDMETHODCALLTYPE CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
    virtual HRESULT STDMETHODCALLTYPE SetPixelShader(IDirect3DPixelShader9* pShader);
    virtual HRESULT STDMETHODCALLTYPE GetPixelShader(IDirect3DPixelShader9** ppShader);
    virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
    virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount);
    virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
    virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount);
    virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
    virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
    virtual HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
    virtual HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
    virtual HRESULT STDMETHODCALLTYPE DeletePatch(UINT Handle);
    virtual HRESULT STDMETHODCALLTYPE CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);
};

#endif // _D3D9_IDIRECT3DDEVICE9_H
