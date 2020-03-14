#ifndef _MIRRORWINDOW_H
#define _MIRRORWINDOW_H

#define WINDOW_WIDTH	1280
#define WINDOW_HEIGHT	720



/* MMD定数 */
#define MMD_MENU_FULLSCREEN	(290)
#define MMD_MENU_FPSNOLIMIT	(234)

#define MMD_BUTTON_PLAY		(0x198)


#define BUFFER_SIZE	(1024)	/* 汎用バッファサイズ定義 */



extern HWND g_hWnd;
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
extern double g_dFovZoom;


extern HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
extern HRESULT SetOVRWindowTitleSuffix(TCHAR *pSuffix);
extern HRESULT SetupWindowState();
extern HRESULT HackMMDWindow(HWND hWndMMD);


#endif // _MIRRORWINDOW_H
