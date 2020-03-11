#ifndef _D3DX9_ID3DXEFFECT_H
#define _D3DX9_ID3DXEFFECT_H

class CHookID3DXEffect : public ID3DXEffect
{
private:
	ULONG ulRefCnt;

protected:
	ID3DXEffect *pOriginal;
	CHookID3DXEffect();

public:
	CHookID3DXEffect(::ID3DXEffect *pEffect);
	~CHookID3DXEffect();

	ULONG STDMETHODCALLTYPE GetRefCnt() { return this->ulRefCnt; }

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

	/*** ID3DXEffect methods ***/

    // Descs
    virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DXEFFECT_DESC* pDesc);
    virtual HRESULT STDMETHODCALLTYPE GetParameterDesc(D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc);
    virtual HRESULT STDMETHODCALLTYPE GetTechniqueDesc(D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc);
    virtual HRESULT STDMETHODCALLTYPE GetPassDesc(D3DXHANDLE hPass, D3DXPASS_DESC* pDesc);
    virtual HRESULT STDMETHODCALLTYPE GetFunctionDesc(D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc);

    // Handle operations
    virtual D3DXHANDLE STDMETHODCALLTYPE GetParameter(D3DXHANDLE hParameter, UINT Index);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetParameterElement(D3DXHANDLE hParameter, UINT Index);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetTechnique(UINT Index);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetTechniqueByName(LPCSTR pName);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetPass(D3DXHANDLE hTechnique, UINT Index);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetPassByName(D3DXHANDLE hTechnique, LPCSTR pName);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetFunction(UINT Index);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetFunctionByName(LPCSTR pName);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetAnnotation(D3DXHANDLE hObject, UINT Index);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName);

    // Get/Set Parameters
    virtual HRESULT STDMETHODCALLTYPE SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes);
    virtual HRESULT STDMETHODCALLTYPE GetValue(D3DXHANDLE hParameter, LPVOID pData, UINT Bytes);
    virtual HRESULT STDMETHODCALLTYPE SetBool(D3DXHANDLE hParameter, BOOL b);
    virtual HRESULT STDMETHODCALLTYPE GetBool(D3DXHANDLE hParameter, BOOL* pb);
    virtual HRESULT STDMETHODCALLTYPE SetBoolArray(D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetBoolArray(D3DXHANDLE hParameter, BOOL* pb, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetInt(D3DXHANDLE hParameter, INT n);
    virtual HRESULT STDMETHODCALLTYPE GetInt(D3DXHANDLE hParameter, INT* pn);
    virtual HRESULT STDMETHODCALLTYPE SetIntArray(D3DXHANDLE hParameter, CONST INT* pn, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetIntArray(D3DXHANDLE hParameter, INT* pn, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetFloat(D3DXHANDLE hParameter, FLOAT f);
    virtual HRESULT STDMETHODCALLTYPE GetFloat(D3DXHANDLE hParameter, FLOAT* pf);
    virtual HRESULT STDMETHODCALLTYPE SetFloatArray(D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetFloatArray(D3DXHANDLE hParameter, FLOAT* pf, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetVector(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector);
    virtual HRESULT STDMETHODCALLTYPE GetVector(D3DXHANDLE hParameter, D3DXVECTOR4* pVector);
    virtual HRESULT STDMETHODCALLTYPE SetVectorArray(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetVectorArray(D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetMatrix(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix);
    virtual HRESULT STDMETHODCALLTYPE GetMatrix(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix);
    virtual HRESULT STDMETHODCALLTYPE SetMatrixArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetMatrixArray(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetMatrixPointerArray(D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetMatrixPointerArray(D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetMatrixTranspose(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix);
    virtual HRESULT STDMETHODCALLTYPE GetMatrixTranspose(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix);
    virtual HRESULT STDMETHODCALLTYPE SetMatrixTransposeArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetMatrixTransposeArray(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetMatrixTransposePointerArray(D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE GetMatrixTransposePointerArray(D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count);
    virtual HRESULT STDMETHODCALLTYPE SetString(D3DXHANDLE hParameter, LPCSTR pString);
    virtual HRESULT STDMETHODCALLTYPE GetString(D3DXHANDLE hParameter, LPCSTR* ppString);
    virtual HRESULT STDMETHODCALLTYPE SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture);
    virtual HRESULT STDMETHODCALLTYPE GetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture);
    virtual HRESULT STDMETHODCALLTYPE GetPixelShader(D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader);
    virtual HRESULT STDMETHODCALLTYPE GetVertexShader(D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader);

	//Set Range of an Array to pass to device
	//Usefull for sending only a subrange of an array down to the device
	virtual HRESULT STDMETHODCALLTYPE SetArrayRange(D3DXHANDLE hParameter, UINT uStart, UINT uEnd); 
	// ID3DXBaseEffect
    
    
    // Pool
    virtual HRESULT STDMETHODCALLTYPE GetPool(LPD3DXEFFECTPOOL* ppPool);

    // Selecting and setting a technique
    virtual HRESULT STDMETHODCALLTYPE SetTechnique(D3DXHANDLE hTechnique);
    virtual D3DXHANDLE STDMETHODCALLTYPE GetCurrentTechnique();
    virtual HRESULT STDMETHODCALLTYPE ValidateTechnique(D3DXHANDLE hTechnique);
    virtual HRESULT STDMETHODCALLTYPE FindNextValidTechnique(D3DXHANDLE hTechnique, D3DXHANDLE *pTechnique);
    virtual BOOL STDMETHODCALLTYPE IsParameterUsed(D3DXHANDLE hParameter, D3DXHANDLE hTechnique);

    // Using current technique
    // Begin           starts active technique
    // BeginPass       begins a pass
    // CommitChanges   updates changes to any set calls in the pass. This should be called before
    //                 any DrawPrimitive call to d3d
    // EndPass         ends a pass
    // End             ends active technique
    virtual HRESULT STDMETHODCALLTYPE Begin(UINT *pPasses, DWORD Flags);
    virtual HRESULT STDMETHODCALLTYPE BeginPass(UINT Pass);
    virtual HRESULT STDMETHODCALLTYPE CommitChanges();
    virtual HRESULT STDMETHODCALLTYPE EndPass();
    virtual HRESULT STDMETHODCALLTYPE End();

    // Managing D3D Device
    virtual HRESULT STDMETHODCALLTYPE GetDevice(LPDIRECT3DDEVICE9* ppDevice);
    virtual HRESULT STDMETHODCALLTYPE OnLostDevice();
    virtual HRESULT STDMETHODCALLTYPE OnResetDevice();

    // Logging device calls
    virtual HRESULT STDMETHODCALLTYPE SetStateManager(LPD3DXEFFECTSTATEMANAGER pManager);
    virtual HRESULT STDMETHODCALLTYPE GetStateManager(LPD3DXEFFECTSTATEMANAGER *ppManager);

    // Parameter blocks
    virtual HRESULT STDMETHODCALLTYPE BeginParameterBlock();
    virtual D3DXHANDLE STDMETHODCALLTYPE EndParameterBlock();
    virtual HRESULT STDMETHODCALLTYPE ApplyParameterBlock(D3DXHANDLE hParameterBlock);
    virtual HRESULT STDMETHODCALLTYPE DeleteParameterBlock(D3DXHANDLE hParameterBlock);

    // Cloning
    virtual HRESULT STDMETHODCALLTYPE CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect);
    
    // Fast path for setting variables directly in ID3DXEffect
    virtual HRESULT STDMETHODCALLTYPE SetRawValue(D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes);

};

#endif // _D3DX9_ID3DXEFFECT_H
