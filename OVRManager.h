#ifndef _OVRMANAGER_H
#define _OVRMANAGER_H


#define OVR_EYE_NUM		(2)
#define OVR_EYE_LEFT	(0)
#define OVR_EYE_RIGHT	(1)


using namespace OVR;
extern int g_nHMDDetected;
extern ovrHmd g_HMD;
extern Sizei g_OvrRenderSize;

extern int g_nOVRFPS;

extern ID3D11Device*                       g_pd3dDevice;
extern ID3D11DeviceContext*                g_pImmediateContext;
extern IDXGISwapChain*                     g_pSwapChain;
extern ID3D11RenderTargetView*             g_pRenderTargetView;

extern ovrEyeRenderDesc g_EyeRenderDesc[OVR_EYE_NUM];
extern ovrRecti g_EyeRenderViewport[OVR_EYE_NUM];


extern D3DXMATRIX g_matOVREyeProj[OVR_EYE_NUM];


extern HANDLE hOVREyeViewThread;



extern HRESULT OVRManager_Create();
extern HRESULT OVRManager_Cleanup();
extern HRESULT OVRDistortion_D3D11Init();
extern HRESULT OVRDistortion_Create();
extern HRESULT OVRDistortion_RenderBlank();
extern HRESULT OVRDistortion_Render1();
extern HRESULT OVRDistortion_Render2();
extern HRESULT OVRDistortion_Cleanup();





#endif // _OVRMANAGER_H

