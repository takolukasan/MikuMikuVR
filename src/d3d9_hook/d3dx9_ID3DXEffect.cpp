#include "stdafx.h"


CHookID3DXEffect::CHookID3DXEffect()
{
	this->ulRefCnt = 1;
}

CHookID3DXEffect::CHookID3DXEffect(::ID3DXEffect *pEffect)
{
	this->ulRefCnt = 1;
	if( pEffect ) {
		this->pOriginal = pEffect;
		this->ulRefCnt = this->pOriginal->AddRef();
	}
}

CHookID3DXEffect::~CHookID3DXEffect()
{
	if( this->pOriginal ) {
		this->pOriginal->Release();
		this->pOriginal = NULL;
	}
}

// CHookID3DXEffect::IUnknown
HRESULT	STDMETHODCALLTYPE CHookID3DXEffect::QueryInterface(REFIID riid, void** ppvObject)
{
	return this->pOriginal->QueryInterface(riid, ppvObject);
}

ULONG STDMETHODCALLTYPE CHookID3DXEffect::AddRef()
{
	this->ulRefCnt = this->pOriginal->AddRef();
    return this->ulRefCnt;
}

ULONG STDMETHODCALLTYPE CHookID3DXEffect::Release()
{
	this->ulRefCnt = this->pOriginal->Release();
    return this->ulRefCnt;
}

// ID3DXEffect
// Descs
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetDesc(D3DXEFFECT_DESC* pDesc)
{ return this->pOriginal->GetDesc(pDesc); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetParameterDesc(D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc)
{ return this->pOriginal->GetParameterDesc(hParameter, pDesc); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetTechniqueDesc(D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc)
{ return this->pOriginal->GetTechniqueDesc(hTechnique, pDesc); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetPassDesc(D3DXHANDLE hPass, D3DXPASS_DESC* pDesc)
{ return this->pOriginal->GetPassDesc(hPass, pDesc); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetFunctionDesc(D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc)
{ return this->pOriginal->GetFunctionDesc(hShader, pDesc); }

// Handle operations
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetParameter(D3DXHANDLE hParameter, UINT Index)
{ return this->pOriginal->GetParameter(hParameter, Index); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName)
{ return this->pOriginal->GetParameterByName(hParameter, pName); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic)
{ return this->pOriginal->GetParameterBySemantic(hParameter, pSemantic); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetParameterElement(D3DXHANDLE hParameter, UINT Index)
{ return this->pOriginal->GetParameterElement(hParameter, Index); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetTechnique(UINT Index)
{ return this->pOriginal->GetTechnique(Index); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetTechniqueByName(LPCSTR pName)
{ return this->pOriginal->GetTechniqueByName(pName); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetPass(D3DXHANDLE hTechnique, UINT Index)
{ return this->pOriginal->GetPass(hTechnique, Index); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetPassByName(D3DXHANDLE hTechnique, LPCSTR pName)
{ return this->pOriginal->GetPassByName(hTechnique, pName); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetFunction(UINT Index)
{ return this->pOriginal->GetFunction(Index); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetFunctionByName(LPCSTR pName)
{ return this->pOriginal->GetFunctionByName(pName); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetAnnotation(D3DXHANDLE hObject, UINT Index)
{ return this->pOriginal->GetAnnotation(hObject, Index); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName)
{ return this->pOriginal->GetAnnotationByName(hObject, pName); }

// Get/Set Parameters
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes)
{ return this->pOriginal->SetValue(hParameter, pData, Bytes); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetValue(D3DXHANDLE hParameter, LPVOID pData, UINT Bytes)
{ return this->pOriginal->GetValue(hParameter, pData, Bytes); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetBool(D3DXHANDLE hParameter, BOOL b)
{ return this->pOriginal->SetBool(hParameter, b); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetBool(D3DXHANDLE hParameter, BOOL* pb)
{ return this->pOriginal->GetBool(hParameter, pb); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetBoolArray(D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count)
{ return this->pOriginal->SetBoolArray(hParameter, pb, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetBoolArray(D3DXHANDLE hParameter, BOOL* pb, UINT Count)
{ return this->pOriginal->GetBoolArray(hParameter, pb, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetInt(D3DXHANDLE hParameter, INT n)
{ return this->pOriginal->SetInt(hParameter, n); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetInt(D3DXHANDLE hParameter, INT* pn)
{ return this->pOriginal->GetInt(hParameter, pn); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetIntArray(D3DXHANDLE hParameter, CONST INT* pn, UINT Count)
{ return this->pOriginal->SetIntArray(hParameter, pn, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetIntArray(D3DXHANDLE hParameter, INT* pn, UINT Count)
{ return this->pOriginal->GetIntArray(hParameter, pn, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetFloat(D3DXHANDLE hParameter, FLOAT f)
{ return this->pOriginal->SetFloat(hParameter, f); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetFloat(D3DXHANDLE hParameter, FLOAT* pf)
{ return this->pOriginal->GetFloat(hParameter, pf); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetFloatArray(D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count)
{ return this->pOriginal->SetFloatArray(hParameter, pf, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetFloatArray(D3DXHANDLE hParameter, FLOAT* pf, UINT Count)
{ return this->pOriginal->GetFloatArray(hParameter, pf, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetVector(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector)
{ return this->pOriginal->SetVector(hParameter, pVector); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetVector(D3DXHANDLE hParameter, D3DXVECTOR4* pVector)
{ return this->pOriginal->GetVector(hParameter, pVector); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetVectorArray(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count)
{ return this->pOriginal->SetVectorArray(hParameter, pVector, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetVectorArray(D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count)
{ return this->pOriginal->GetVectorArray(hParameter, pVector, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetMatrix(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix)
{ return this->pOriginal->SetMatrix(hParameter, pMatrix); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetMatrix(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix)
{ return this->pOriginal->GetMatrix(hParameter, pMatrix); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetMatrixArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count)
{ return this->pOriginal->SetMatrixArray(hParameter, pMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetMatrixArray(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count)
{ return this->pOriginal->GetMatrixArray(hParameter, pMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetMatrixPointerArray(D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count)
{ return this->pOriginal->SetMatrixPointerArray(hParameter, ppMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetMatrixPointerArray(D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count)
{ return this->pOriginal->GetMatrixPointerArray(hParameter, ppMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetMatrixTranspose(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix)
{ return this->pOriginal->SetMatrixTranspose(hParameter, pMatrix); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetMatrixTranspose(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix)
{ return this->pOriginal->GetMatrixTranspose(hParameter, pMatrix); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetMatrixTransposeArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count)
{ return this->pOriginal->SetMatrixTransposeArray(hParameter, pMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetMatrixTransposeArray(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count)
{ return this->pOriginal->GetMatrixTransposeArray(hParameter, pMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetMatrixTransposePointerArray(D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count)
{ return this->pOriginal->SetMatrixTransposePointerArray(hParameter, ppMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetMatrixTransposePointerArray(D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count)
{ return this->pOriginal->GetMatrixTransposePointerArray(hParameter, ppMatrix, Count); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetString(D3DXHANDLE hParameter, LPCSTR pString)
{ return this->pOriginal->SetString(hParameter, pString); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetString(D3DXHANDLE hParameter, LPCSTR* ppString)
{ return this->pOriginal->GetString(hParameter, ppString); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture)
{ return this->pOriginal->SetTexture(hParameter, pTexture); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture)
{ return this->pOriginal->GetTexture(hParameter, ppTexture); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetPixelShader(D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader)
{ return this->pOriginal->GetPixelShader(hParameter, ppPShader); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetVertexShader(D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader)
{ return this->pOriginal->GetVertexShader(hParameter, ppVShader); }

//Set Range of an Array to pass to device
//Usefull for sending only a subrange of an array down to the device
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetArrayRange(D3DXHANDLE hParameter, UINT uStart, UINT uEnd)
{ return this->pOriginal->SetArrayRange (hParameter, uStart, uEnd); }
// ID3DXBaseEffect


// Pool
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetPool(LPD3DXEFFECTPOOL* ppPool)
{ return this->pOriginal->GetPool(ppPool); }

// Selecting and setting a technique
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetTechnique(D3DXHANDLE hTechnique)
{ return this->pOriginal->SetTechnique(hTechnique); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::GetCurrentTechnique()
{ return this->pOriginal->GetCurrentTechnique(); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::ValidateTechnique(D3DXHANDLE hTechnique)
{ return this->pOriginal->ValidateTechnique(hTechnique); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::FindNextValidTechnique(D3DXHANDLE hTechnique, D3DXHANDLE *pTechnique)
{ return this->pOriginal->FindNextValidTechnique(hTechnique, pTechnique); }
BOOL STDMETHODCALLTYPE CHookID3DXEffect::IsParameterUsed(D3DXHANDLE hParameter, D3DXHANDLE hTechnique)
{ return this->pOriginal->IsParameterUsed(hParameter, hTechnique); }

// Using current technique
// Begin           starts active technique
// BeginPass       begins a pass
// CommitChanges   updates changes to any set calls in the pass. This should be called before
//                 any DrawPrimitive call to d3d
// EndPass         ends a pass
// End             ends active technique
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::Begin(UINT *pPasses, DWORD Flags)
{ return this->pOriginal->Begin(pPasses, Flags); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::BeginPass(UINT Pass)
{ return this->pOriginal->BeginPass(Pass); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::CommitChanges()
{ return this->pOriginal->CommitChanges(); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::EndPass()
{ return this->pOriginal->EndPass(); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::End()
{ return this->pOriginal->End(); }

// Managing D3D Device
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetDevice(LPDIRECT3DDEVICE9* ppDevice)
{ return this->pOriginal->GetDevice(ppDevice); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::OnLostDevice()
{ return this->pOriginal->OnLostDevice(); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::OnResetDevice()
{ return this->pOriginal->OnResetDevice(); }

// Logging device calls
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetStateManager(LPD3DXEFFECTSTATEMANAGER pManager)
{ return this->pOriginal->SetStateManager(pManager); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::GetStateManager(LPD3DXEFFECTSTATEMANAGER *ppManager)
{ return this->pOriginal->GetStateManager(ppManager); }

// Parameter blocks
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::BeginParameterBlock()
{ return this->pOriginal->BeginParameterBlock(); }
D3DXHANDLE STDMETHODCALLTYPE CHookID3DXEffect::EndParameterBlock()
{ return this->pOriginal->EndParameterBlock(); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::ApplyParameterBlock(D3DXHANDLE hParameterBlock)
{ return this->pOriginal->ApplyParameterBlock(hParameterBlock); }
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::DeleteParameterBlock(D3DXHANDLE hParameterBlock)
{ return this->pOriginal->DeleteParameterBlock(hParameterBlock); }

// Cloning
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect)
{ return this->pOriginal->CloneEffect(pDevice, ppEffect); }

// Fast path for setting variables directly in ID3DXEffect
HRESULT STDMETHODCALLTYPE CHookID3DXEffect::SetRawValue(D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes)
{ return this->pOriginal->SetRawValue(hParameter, pData, ByteOffset, Bytes); }


