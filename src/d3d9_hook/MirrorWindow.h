#ifndef _MIRRORWINDOW_H
#define _MIRRORWINDOW_H



/* MMD定数 */
#define MMD_MENU_FULLSCREEN	(290)
#define MMD_MENU_FPSNOLIMIT	(234)

#define MMD_BUTTON_PLAY		(0x198)


#define BUFFER_SIZE	(1024)	/* 汎用バッファサイズ定義 */

#if 0
#define MME_ENABLED_INDICATE	TEXT("[MME Enabled]")
#define MME_DISABLED_INDICATE	TEXT("[MME Disabled]")
#endif


enum MIRROR_RENDER_FLAGS {
	MIRROR_RENDER_MODELONLY = 0x0000,

	MIRROR_RENDER_GRID = 0x0001,
	MIRROR_RENDER_MAKER = 0x0002,
	MIRROR_RENDER_ACS = 0x0004,
	MIRROR_RENDER_MODE_MMEHACK = 0x0100,

/* いつかここやめたい */
	MIRROR_RENDER_MODEL_GRID = 0x0001,
	MIRROR_RENDER_MODEL_MAKER = 0x0002,
	MIRROR_RENDER_MODEL_GRID_MAKER = 0x0003,
/* いつかここやめたい */

	MIRROR_RENDER_MODEL_INIT = 0x0005,

	MIRROR_RENDER_MAX = 0xffffffff
};



extern HWND g_hWnd;
extern RECT g_ClientRect;
extern WNDPROC g_WndMMDSubProc;


#ifdef OVR_ENABLE
extern HWND g_hWndDistortion;
extern BOOL bOVREyeTexMirror;
#endif

/* MMDウィンドウ管理 */
extern HWND g_hWndMMD;

extern double g_dMovingPosX;
extern double g_dMovingPosY;
extern double g_dMovingPosZ;
extern double g_dRotationY;


extern HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
extern HRESULT SetWindowTitleSuffix(TCHAR *pSuffix);
extern HRESULT SetOVRWindowTitleSuffix(TCHAR *pSuffix);
extern HRESULT SetupWindowState();
extern HRESULT HackMMDWindow(HWND hWndMMD);


#endif // _MIRRORWINDOW_H
