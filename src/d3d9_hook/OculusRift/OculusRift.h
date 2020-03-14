#ifndef _OCULUSRIFT_H
#define _OCULUSRIFT_H

#define OCULUSRIFT_SUPPORT_DIRECTX9

#define OculusRift_MaxDevices	(1)


/* ���̃N���X�̃C���X�^���X�𒼐ڐ������Ă͂����܂���(�ł��܂���) */
/* �K�� OculusRift �N���X���琶�����邱�ƁB */
class OculusRiftDevice {
	friend class OculusRift;

private:

	int nMyDeviceIndex;

	HANDLE hHeap;

	IDXGIDevice *pDXGI;
	IDXGIAdapter *pAdapter;
	IDXGIFactory *pFactory;

	ID3D11Device *pD3D11Device;
	ID3D11DeviceContext *pD3D11Context;

	ovrSession Session;
	ovrGraphicsLuid luidGPU;
	ovrHmdDesc HmdDesc;
	unsigned int uHmdCaps;
	unsigned int uTrackingCaps;

	ovrVector3f HmdToEyeViewOffset[ovrEye_Count];
    ovrRecti eyeRenderViewport[ovrEye_Count];
	ovrEyeRenderDesc eyeRenderDesc[ovrEye_Count];
	ovrPosef EyeRenderPose[ovrEye_Count];

	ovrSwapTextureSet *pTextureSet[ovrEye_Count];
	ID3D11RenderTargetView **pD3D11RTVSet[ovrEye_Count];
	ID3D11ShaderResourceView **pMirrorTexSRV[ovrEye_Count];

	ID3D11Texture2D *TexDepth[ovrEye_Count];
	ID3D11DepthStencilView *pD3D11DSV[ovrEye_Count];

	HWND hWndMirrorWindow;
	UINT uMirrorWindowWidth;
	UINT uMirrorWindowHeight;

	IDXGISwapChain *pMirrorSwapChain;
	ovrTexture *pMirrorTexture;
	ID3D11Texture2D *pMirrorBackBuffer;
	ID3D11RenderTargetView *pMirrorRTV;
	ID3D11Texture2D *pMirrorBackBufferDepth;

	OculusRiftDevice(int nDeviceIndex, ovrSession session, ovrGraphicsLuid luid);
	~OculusRiftDevice();

public:

	/* �I�v�V���� */
	ovrHmd GetDevice() { return this->Session; }
	void GetLuid(LUID *pLUID) {
		LUID *p;
		if( pLUID ) {
			p = (LUID *)&(this->luidGPU);
			*pLUID = *p;
		}
	}
	ID3D11Device *GetD3D11Device() { return pD3D11Device; }
	ID3D11DeviceContext *GetD3D11Context() { return pD3D11Context; }
	ID3D11Texture2D *GetBackBuffer() { return this->pMirrorBackBuffer; }
	ID3D11Texture2D *GetMirrorTexture() { return ((ovrD3D11Texture *)this->pMirrorTexture)->D3D11.pTexture; }
	ID3D11Texture2D *GetCurrentEyeTex(ovrEyeType eye) {
		int nTexIndex = this->pTextureSet[eye]->CurrentIndex;
		ovrD3D11Texture *pTex = (ovrD3D11Texture *)&(this->pTextureSet[eye]->Textures[nTexIndex]);
		return pTex->D3D11.pTexture;
	}
	ovrResult GetEyeRenderTextureSize(ovrSizei *pLeft, ovrSizei *pRight) {
		if( !pLeft && !pRight )
			return ovrError_InvalidParameter;

		ovrSizei idealSize[ovrEye_Count];
		idealSize[ovrEye_Left] = ovr_GetFovTextureSize(this->Session, ovrEye_Left, this->HmdDesc.DefaultEyeFov[ovrEye_Left], 1.0f);
		idealSize[ovrEye_Right] = ovr_GetFovTextureSize(this->Session, ovrEye_Right, this->HmdDesc.DefaultEyeFov[ovrEye_Right], 1.0f);

		if( pLeft )
			*pLeft = idealSize[ovrEye_Left];
		if( pRight )
			*pRight = idealSize[ovrEye_Right];

		return ovrSuccess;
	}

	ovrResult GetOvrHmdDesc(ovrHmdDesc *ovrHmd);

	unsigned int GetHmdCaps() {
		this->uHmdCaps = ovr_GetEnabledCaps(this->Session);
		return this->uHmdCaps;
	}
	void SetHmdCaps(unsigned int uCaps) {
		this->uHmdCaps = uCaps & ovrHmdCap_Writable_Mask;
		ovr_SetEnabledCaps(this->Session, this->uHmdCaps);
	}
	unsigned int GetHmdTrackingCaps() {
		return this->uTrackingCaps;
	}
	ovrResult SetHmdTrackingCaps(unsigned int uCaps) {
		this->uTrackingCaps = uCaps;
		return ovr_ConfigureTracking(this->Session, this->uTrackingCaps, 0);
	}

	ovrResult ResetTracking() {
		ovr_RecenterPose(this->Session);
		return ovrSuccess;
	}

	/* �~���[�\�������������� */
	ovrResult SetupMirrorWindow(HWND *pWndMirrorWindow, HINSTANCE hInstance = NULL, UINT uWindowWidth = 0, UINT uWindowHeight = 0);

	/* �R�[���K�{ */
	/* �Ăяo���������X�����Ă�Direct3D�f�o�C�X�C���^�[�t�F�[�X���g�킹�� */
	HRESULT InitializeDirectX(ID3D11Device *pDevice, ID3D11DeviceContext *pContext);
	/* �����_�����O�p�Ƀf�o�C�X���������s�킹��B�������ȗ�����ƁA����̃f�o�C�X�B */
	HRESULT InitializeDirectX(UINT uAdapter = 0);

	ovrResult CreateEyeRenderTexture(ovrSizei EyeTexSize[ovrEye_Count]);

	ovrResult CreateEyeRenderTexture()
	{
		ovrSizei idealSize[ovrEye_Count];
		idealSize[ovrEye_Left] = ovr_GetFovTextureSize(this->Session, ovrEye_Left, this->HmdDesc.DefaultEyeFov[ovrEye_Left], 1.0f);
		idealSize[ovrEye_Right] = ovr_GetFovTextureSize(this->Session, ovrEye_Right, this->HmdDesc.DefaultEyeFov[ovrEye_Right], 1.0f);
		return CreateEyeRenderTexture(idealSize);
	}

	ovrResult BeginFrame(ovrPosef *ppEyeRenderPose = nullptr);
	ovrResult BeginEyeRender(ovrEyeType eye, bool bClearRTAndDSV, const float fRTClearColor[4]); // fRTClearColor: Important that alpha=0, if want pixels to be transparent, for manual layers
	ovrResult EndFrame();


	XMMATRIX * GetProjectionMatrix(XMMATRIX xmProjection[ovrEye_Count]);
	XMMATRIX * GetViewMatrixFromLastPose(XMMATRIX xmEyePositionAndRotation[ovrEye_Count]);

#ifdef OCULUSRIFT_SUPPORT_DIRECTX9
	D3DXMATRIX * GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count]);
	D3DXMATRIX * GetProjectionMatrix(D3DXMATRIX matProjection[ovrEye_Count], double CustomFovOffset);
	D3DXMATRIX * GetViewMatrixFromLastPose(D3DXMATRIX matEyePositionAndRotation[ovrEye_Count]);
#endif

};

class OculusRift {
	OculusRiftDevice *pDevices[OculusRift_MaxDevices];

private:
	static const int nTimeoutToDetectRiftInMiliseconds = 100;

public:
	OculusRift();
	~OculusRift();

	/* �ʂ�static����Ȃ��Ă������H */
	static ovrResult Initialize();
	static void UnInitialize();

	static int GetNumOfOculusRift();

public:
	ovrResult CreateDevice(int nDeviceIndex, OculusRiftDevice **ppDevice);
	ovrResult DestoryDevice(int nDeviceIndex);
	ovrResult DestoryDevice(OculusRiftDevice *pDevice)
	{
		if( !pDevice )
			return ovrError_InvalidParameter;

		return this->DestoryDevice(pDevice->nMyDeviceIndex);
	}
};



#endif // _OCULUSRIFT_H
