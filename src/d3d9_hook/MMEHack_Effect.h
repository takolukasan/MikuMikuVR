/******************************************************************************/
/* MMDMirrorView                                                              */
/*                                                                            */
/* �t�@�C�����FMMEHack_Effect.cpp                                             */
/* �쐬�F�������J����                                                         */
/* �����FMME����D3DX�֐��R�[���̃t�b�N�ƃG�t�F�N�g����̃t�b�N                */
/*                                                                            */
/******************************************************************************/

#ifndef _MMEHACK_EFFECT_H
#define _MMEHACK_EFFECT_H


/* MMD(MME)�����D3DX�֐��R�[����x86:ANSI/x64:Unicode(���W�b�N���őΉ�) �܂��ɊO�� */
#define MMEHACK_EFFECT_RENDERTARGET	"MMDOVRCamera.fx"

#define MMEHACK_EFFECT_OVRRENDERL	"MMDOVRRenderL.fx"
#define MMEHACK_EFFECT_OVRRENDERR	"MMDOVRRenderR.fx"

#define MMEHACK_EFFECT_EYETEXTURENAMEL	"RTEyeViewL"
#define MMEHACK_EFFECT_EYETEXTURENAMER	"RTEyeViewR"

#define MMEHACK_EFFECT_VIEWTYPE		"MMDOVR_ViewType"
#define MMEHACK_EFFECT_VIEWEYE		"MMDOVR_ViewEye"

#define MMEHACK_EFFECT_PROJ_ZNEAR	"MMDOVR_zNear"
#define MMEHACK_EFFECT_PROJ_ZFAR	"MMDOVR_zFar"


#define MMEHACK_EFFECT_SEMANTIC_HMDTYPE			"MMEHACK_HMDTYPE"
#define MMEHACK_EFFECT_SEMANTIC_OVR_VIEW		"MMDOVR_VIEW"
#define MMEHACK_EFFECT_SEMANTIC_OVR_PROJECTION	"MMDOVR_PROJECTION"


// ���w�萔�E�֐���`
#define M_PI			(3.141592654)
#define DEG2RAD(deg)	(M_PI * (deg) / 180.0)
#define RAD2DEG(rad)	((rad) * 180.0 / M_PI)


// ���ʒ萔
#define MMEHACK_VIEWTYPE_DEFAULT	(0)		// �r���[�s��E�v���W�F�N�V�����s��������������Ȃ�
#define MMEHACK_VIEWTYPE_VIEWPROJ	(1)		// �r���[�s��E�v���W�F�N�V����������������
#define MMEHACK_VIEWTYPE_PROJONLY	(2)		// �v���W�F�N�V�����s��݂̂�����������(�r���[�s���MMD�̃J�������g��)


#define MMEHACK_VIEWEYE_LEFT		(0)
#define MMEHACK_VIEWEYE_RIGHT		(1)

// HMD�̃o�[�W�������ʗp
// OVR_CAPI.h �ɂĒ�`����Ă��� ovrHmdType ���Q�Ƃ̂���
#define MMEHACK_OVRHMDTYPE_DK2		(6)		// ovrHmd_DK2       = 6
#define MMEHACK_OVRHMDTYPE_CV1		(14)	// ovrHmd_CV1       = 14

#define MMEHACK_PROJ_ZNEAR_DEFAULT	(0.01)
#define MMEHACK_PROJ_ZFAR_DEFAULT	(100000.0)


// �t�b�N����֐��̒�`
// Import from d3dx9_**.dll

// x86: ANSI x64: Unicode�ŃC���|�[�g���Ă���Ƃ������A�܂��ɊO��
#ifndef X64
typedef HRESULT (WINAPI *tD3DXCreateEffectFromFileA)(LPDIRECT3DDEVICE9, LPCSTR, CONST D3DXMACRO*, LPD3DXINCLUDE,
	DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
extern tD3DXCreateEffectFromFileA g_orgMMEffectD3DXCreateEffectFromFileA;
HRESULT WINAPI MMEffectHack_D3DXCreateEffectFromFileA(
        LPDIRECT3DDEVICE9               pDevice,
        LPCSTR                          pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
#else
typedef HRESULT (WINAPI *tD3DXCreateEffectFromFileW)(LPDIRECT3DDEVICE9, LPCWSTR, CONST D3DXMACRO*, LPD3DXINCLUDE,
	DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT*, LPD3DXBUFFER*);
extern tD3DXCreateEffectFromFileW g_orgMMEffectD3DXCreateEffectFromFileW;
HRESULT WINAPI MMEffectHack_D3DXCreateEffectFromFileW(
        LPDIRECT3DDEVICE9               pDevice,
        LPCWSTR                         pSrcFile,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        DWORD                           Flags,
        LPD3DXEFFECTPOOL                pPool,
        LPD3DXEFFECT*                   ppEffect,
        LPD3DXBUFFER*                   ppCompilationErrors);
#endif



// �t�b�N����ID3DXEffect�C���^�[�t�F�[�X�̎���

// RT
class CHookID3DXEffectMMEMirrorRT : public CHookID3DXEffect
{
private:

#ifdef OVR_ENABLE
	ovrHmdType TargetHMDType;
	D3DXHANDLE hEyeRT[OVR_EYE_NUM];
	IDirect3DSurface9 *pSurfRTEye[OVR_EYE_NUM];
	D3DSURFACE_DESC RTEyeTexDesc[OVR_EYE_NUM];

	float fProjZNear;
	float fProjZFar;
	D3DXHANDLE hProjZNear;
	D3DXHANDLE hProjZFar;

#endif

public:
	CHookID3DXEffectMMEMirrorRT(::ID3DXEffect *pEffect);
	~CHookID3DXEffectMMEMirrorRT();

#ifdef OVR_ENABLE
	ovrHmdType GetTargetHmdType() { return this->TargetHMDType; }
	IDirect3DSurface9 * GetEyeSurface(int eye) { return this->pSurfRTEye[eye]; }
	D3DSURFACE_DESC * GetRTEyeTexDesc(int eye) { return &(this->RTEyeTexDesc[eye]); }
	float GetProjZNear() { return this->fProjZNear; }
	float GetProjZFar() { return this->fProjZFar; }
#endif

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** ID3DXEffect methods ***/
	virtual HRESULT STDMETHODCALLTYPE Begin(UINT *pPasses, DWORD Flags);
    virtual HRESULT STDMETHODCALLTYPE SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture);
};


#ifdef OVR_ENABLE
class CHookID3DXEffectOVRRenderer : public CHookID3DXEffect
{
private:
	int nOVREye;
	int nViewType;

	D3DXHANDLE hViewMatrix;
	D3DXHANDLE hProjMatrix;
	D3DXHANDLE hViewType;

public:
	CHookID3DXEffectOVRRenderer(::ID3DXEffect *pEffect);
	~CHookID3DXEffectOVRRenderer();

	void STDMETHODCALLTYPE SetOVREye(int nEye) { this->nOVREye = nEye; };
	int STDMETHODCALLTYPE GetOVREye() { return this->nOVREye; }
	HRESULT STDMETHODCALLTYPE SetProjMatrix(const D3DXMATRIX *matProj);

	/*** IUnknown methods ***/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	/*** ID3DXEffect methods ***/
    virtual HRESULT STDMETHODCALLTYPE CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect);
	virtual HRESULT STDMETHODCALLTYPE Begin(UINT *pPasses, DWORD Flags);
};
#endif


/* Direct3DX9 �����C���^�[�t�F�[�X�|�C���^ */
extern CHookID3DXEffectMMEMirrorRT *g_pMMEHookMirrorRT;


#ifdef OVR_ENABLE
extern std::vector<CHookID3DXEffectOVRRenderer *> g_vecOVREyeRT[OVR_EYE_NUM];
#endif



#endif // _MMEHACK_EFFECT_H
