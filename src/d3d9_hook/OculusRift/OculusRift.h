#ifndef _OCULUSRIFT_H
#define _OCULUSRIFT_H

#define OCULUSRIFT_SUPPORT_DIRECTX9

#define OculusRift_MaxDevices	(16)

#define MIRROR_EFFECT_FILE	L"OculusMirrorRender.fx"
#define MIRROR_EFFECT_VS	"VSMirror"
#define MIRROR_EFFECT_PS	"PSMirror"


struct MirrorVertexBuffer
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};


/* このクラスのインスタンスを直接生成してはいけません(できません) */
/* 必ず OculusRift クラスから生成すること。 */
class OculusRiftDevice {
	friend class OculusRift;

private:

	int nMyDeviceIndex;
	bool bDebugDevice;

	HANDLE hHeap;

	IDXGIDevice *pDXGI;
	IDXGIAdapter *pAdapter;
	IDXGIFactory *pFactory;

	ID3D11Device *pD3D11Device;
	ID3D11DeviceContext *pD3D11Context;

	ovrHmd HMD;
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
	ID3D11DepthStencilView *pMirrorDSV;

	ID3D11VertexShader *pMirrorVertexShader;
	ID3D11PixelShader *pMirrorPixelShader;
	ID3D11InputLayout *pMirrorVertexLayout;
	ID3D11Buffer *pMirrorVertexBuffer;
	ID3D11Buffer *pMirrorIndexBuffer;
	ID3D11SamplerState *pMirrorSamplerState;


	OculusRiftDevice(int nDeviceIndex, ovrHmd hmd);
	~OculusRiftDevice();

public:

	/* オプション */
	ovrHmd GetDevice() { return this->HMD; }
	ID3D11Device *GetD3D11Dev() { return pD3D11Device; }
	ID3D11DeviceContext *GetD3D11Context() { return pD3D11Context; }
	ID3D11Texture2D *GetCurrentEyeTex(ovrEyeType eye) {
		int nTexIndex = this->pTextureSet[eye]->CurrentIndex;
		ovrD3D11Texture *pTex = (ovrD3D11Texture *)&(this->pTextureSet[eye]->Textures[nTexIndex]);
		return pTex->D3D11.pTexture;
	}




	ovrResult GetOvrHmdDesc(ovrHmdDesc *ovrHmd);

	unsigned int GetHmdCaps() {
		this->uHmdCaps = ovrHmd_GetEnabledCaps(this->HMD);
		return this->uHmdCaps;
	}
	void SetHmdCaps(unsigned int uCaps) {
		this->uHmdCaps = uCaps & ovrHmdCap_Writable_Mask;
		ovrHmd_SetEnabledCaps(this->HMD, this->uHmdCaps);
	}
	unsigned int GetHmdTrackingCaps() {
		return this->uTrackingCaps;
	}
	ovrResult SetHmdTrackingCaps(unsigned int uCaps) {
		this->uTrackingCaps = uCaps;
		return ovrHmd_ConfigureTracking(this->HMD, this->uTrackingCaps, 0);
	}

	ovrResult ResetTracking() {
		ovrHmd_RecenterPose(this->HMD);
		return ovrSuccess;
	}

	/* ミラー表示したい時だけ */
	ovrResult SetupMirrorWindow(HWND *pWndMirrorWindow, HINSTANCE hInstance = NULL, UINT uWindowWidth = 0, UINT uWindowHeight = 0);

	/* コール必須 */
	/* 呼び出し側が元々持ってるDirect3Dデバイスインターフェースを使わせる */
	HRESULT InitializeDirectX(ID3D11Device *pDevice, ID3D11DeviceContext *pContext);
	/* レンダリング用にデバイス初期化も行わせる。引数を省略すると、既定のデバイス。 */
	HRESULT InitializeDirectX(UINT uAdapter = 0);


	ovrResult CreateEyeRenderTexture(OVR::Sizei EyeTexSize[ovrEye_Count]);
	ovrResult CreateEyeRenderTexture()
	{
		OVR::Sizei idealSize[ovrEye_Count];
		idealSize[ovrEye_Left] = ovrHmd_GetFovTextureSize(this->HMD, (ovrEyeType)ovrEye_Left, this->HMD->DefaultEyeFov[ovrEye_Left], 1.0f);
		idealSize[ovrEye_Right] = ovrHmd_GetFovTextureSize(this->HMD, (ovrEyeType)ovrEye_Right, this->HMD->DefaultEyeFov[ovrEye_Right], 1.0f);
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

public:
	OculusRift();
	~OculusRift();

	/* 別にstaticじゃなくてもいい？ */
	static ovrResult Initialize();
	static void UnInitialize();

	static int GetNumOfOculusRift();

public:
	ovrResult CreateDevice(int nDeviceIndex, OculusRiftDevice **ppDevice);
	ovrResult CreateDebugDevice(int nDeviceIndex, OculusRiftDevice **ppDevice);
	ovrResult DestoryDevice(int nDeviceIndex);
	ovrResult DestoryDevice(OculusRiftDevice *pDevice)
	{
		if( !pDevice )
			return ovrError_InvalidParameter;

		return this->DestoryDevice(pDevice->nMyDeviceIndex);
	}
};



#endif // _OCULUSRIFT_H
