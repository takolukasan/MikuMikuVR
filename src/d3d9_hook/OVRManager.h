#ifndef _OVRMANAGER_H
#define _OVRMANAGER_H


#define OVR_EYE_NUM		(ovrEye_Count)
#define OVR_EYE_LEFT	(ovrEye_Left)
#define OVR_EYE_RIGHT	(ovrEye_Right)



using namespace OVR;


extern int g_nOVRFPS;


extern ovrRecti g_EyeRenderViewport[OVR_EYE_NUM];

extern D3DXMATRIX g_matOVREyeProj[OVR_EYE_NUM];


extern OculusRiftDevice *g_pRift;

extern ovrPosef g_RiftLastPose[OVR_EYE_NUM];


extern HRESULT OVRManager_Create();
extern HRESULT OVRManager_Cleanup();
extern HRESULT OVRDistortion_D3D11Init();
extern HRESULT OVRDistortion_Create();
extern HRESULT OVRDistortion_RenderBlank();
extern HRESULT OVRDistortion_Render1();
extern HRESULT OVRDistortion_Render2();
extern HRESULT OVRDistortion_Cleanup();





#endif // _OVRMANAGER_H

