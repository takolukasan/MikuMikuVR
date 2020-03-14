#include "stdafx.h"


#define WINDOW_TITLE		TEXT("MikuMikuVR for Oculus Rift Ver 0.02 alpha")
#define CLASS_NAME			TEXT("MMDOVRHookClass")
#define WINDOW_TITLE_OVR	TEXT("MikuMikuVR Mirror")
#define CLASS_NAME_OVR		TEXT("MMDOVRMirrorClass")

#define WINDOW_WIDTH	1280
#define WINDOW_HEIGHT	720


HWND g_hWnd;
HMENU g_hMenu;
RECT g_ClientRect;


#ifdef OVR_ENABLE
HWND g_hWndDistortion;
BOOL bOVREyeTexMirror;
#endif

/* MMDウィンドウ管理 */
HWND g_hWndMMD;
// RECT g_rectLastMMDSize;





static BOOL bCloseEnable = FALSE;

/* 初期値はグリッド線のみ表示 */
// MIRROR_RENDER_FLAGS g_MirrorRenderObj = MIRROR_RENDER_MODEL_INIT;
// BOOL g_bMMDSyncResize = TRUE;
WNDPROC g_WndMMDSubProc;

static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
static LRESULT CALLBACK WndProcDistortion( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
static void MenuSelectionProc( UINT uMenuItemId );
static LRESULT CALLBACK MMDSubWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );


HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    // wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
	wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    // wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = CLASS_NAME;
    // wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hIconSm = NULL;
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	rc.right = 1182;
	rc.bottom = 1461 / 2;

    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW & (~WS_OVERLAPPED), TRUE );
    g_hWnd = CreateWindow( wcex.lpszClassName, WINDOW_TITLE, WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

#ifdef OVR_ENABLE

    wcex.lpfnWndProc = WndProcDistortion;
	wcex.lpszMenuName = NULL;
    wcex.lpszClassName = CLASS_NAME_OVR;
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

	rc.right = WINDOW_WIDTH;
	rc.bottom = WINDOW_HEIGHT;


	// Create window
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW & (~WS_OVERLAPPED), TRUE );
    g_hWndDistortion = CreateWindow( wcex.lpszClassName, WINDOW_TITLE_OVR, WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWndDistortion )
        return E_FAIL;

#endif

    return S_OK;
}

HRESULT SetWindowTitleSuffix(TCHAR *pSuffix)
{
	TCHAR tWindowText[BUFFER_SIZE * 2];
	TCHAR *tStr = NULL;
	size_t len = 0;

	StringCchCopy(tWindowText, BUFFER_SIZE, WINDOW_TITLE);

#if 0
	if( g_bMMEHacked ) {
		tStr = L" " MME_ENABLED_INDICATE;
	}
	else {
		tStr = L" " MME_DISABLED_INDICATE;
	}
	StringCchLength(tStr, BUFFER_SIZE, &len);
	StringCchCatN(tWindowText, BUFFER_SIZE, tStr, len);

	if( pSuffix ) {
		StringCchLength(L" ", BUFFER_SIZE, &len);
		StringCchCatN(tWindowText, BUFFER_SIZE, L" ", len);
		StringCchLength(pSuffix, BUFFER_SIZE, &len);
		StringCchCatN(tWindowText, BUFFER_SIZE, pSuffix, len);
	}
#endif

	SetWindowText(g_hWnd, tWindowText);

	return S_OK;
}

#ifdef OVR_ENABLE
HRESULT SetOVRWindowTitleSuffix(TCHAR *pSuffix)
{
	TCHAR tWindowText[BUFFER_SIZE * 2];
	TCHAR *tStr = NULL;
	size_t len = 0;

	StringCchCopy(tWindowText, BUFFER_SIZE, WINDOW_TITLE_OVR);

	StringCchLength(tStr, BUFFER_SIZE, &len);
	StringCchCatN(tWindowText, BUFFER_SIZE, tStr, len);

	if( pSuffix ) {
		StringCchLength(L" ", BUFFER_SIZE, &len);
		StringCchCatN(tWindowText, BUFFER_SIZE, L" ", len);
		StringCchLength(pSuffix, BUFFER_SIZE, &len);
		StringCchCatN(tWindowText, BUFFER_SIZE, pSuffix, len);
	}

	SetWindowText(g_hWndDistortion, tWindowText);

	return S_OK;
}
#endif

HRESULT SetupWindowState()
{
	TCHAR tWindowText[BUFFER_SIZE * 2];
	GetWindowText(g_hWnd, tWindowText, BUFFER_SIZE / sizeof(TCHAR));

	SetWindowTitleSuffix(NULL);

	g_hMenu = GetMenu(g_hWnd);

	return S_OK;
}

HRESULT HackMMDWindow(HWND hWndMMD)
{
	g_hWndMMD = hWndMMD;

	// MMD終了通知を乗っ取るためにサブクラス化
	g_WndMMDSubProc = (WNDPROC)SetWindowLongPtr(g_hWndMMD, GWLP_WNDPROC, (LONG_PTR)MMDSubWndProc);

	HMENU hMMDMenu = GetMenu(g_hWndMMD);
	EnableMenuItem(hMMDMenu, MMD_MENU_FULLSCREEN, MF_BYCOMMAND | MF_GRAYED);
	PostMessage(g_hWndMMD, WM_COMMAND, MMD_MENU_FPSNOLIMIT, 0);

	return S_OK;
}

static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{

    switch( message )
    {
 		case WM_CREATE:
			return 0;
		case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
		case WM_SIZE:
			GetClientRect(hWnd, &g_ClientRect);
            return TRUE;
		case WM_CLOSE:
			/* 閉じるを無視する */
			/* MMD本体が終了する時にのみ許可する */
			if( bCloseEnable == TRUE ) {
#ifdef OVR_ENABLE
				DestroyWindow(g_hWndDistortion);
#endif
				DestroyWindow(hWnd);
			}
			return 0;
		case WM_COMMAND:
			if( HIWORD(wParam) == 0 && ID_MENU < LOWORD(wParam) && LOWORD(wParam) < ID_MENU_END ) {
				MenuSelectionProc(LOWORD(wParam));
			}
			break;
		default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

static void MenuSelectionProc( UINT uMenuItemId )
{
	BOOL bItemChecked;
	BOOL bItemChecking;

	DWORD dwChecked;

	dwChecked = CheckMenuItem(g_hMenu, uMenuItemId, MF_BYCOMMAND);

	if( dwChecked & MF_CHECKED )
		bItemChecked = TRUE;
	else
		bItemChecked = FALSE;

	// Default
	bItemChecking = bItemChecked;

	switch(uMenuItemId) {
#ifdef OVR_ENABLE
		case ID_MENU_OVR_MIRROR:
			bItemChecking = !bItemChecked;
			{
				unsigned int caps;
				caps = ovrHmd_GetEnabledCaps(g_HMD);
				if( !bItemChecking ) {
					caps |= ovrHmdCap_NoMirrorToWindow;
				}
				else {
					caps &= ~ovrHmdCap_NoMirrorToWindow;
				}
				ovrHmd_SetEnabledCaps(g_HMD, caps);
			}
			break;
		case ID_MENU_OVR_EYETEXTURE:
			bItemChecking = !bItemChecked;
			
			bOVREyeTexMirror = bItemChecking;

			break;
		case ID_MENU_OCULUS_VIEWINIT:
			ovrHmd_RecenterPose(g_HMD);
			break;
		case ID_MENU_OCULUS_REINIT:
			{
				SuspendThread(hOVREyeViewThread);

				ovrHmd_Destroy(g_HMD);
				g_nHMDDetected = ovrHmd_Detect();
				if( g_nHMDDetected >= 1 ) {
					// Change index 0...g_nHMDDetected-1 for render device
					g_HMD = ovrHmd_Create(0);
				}
				else {
					g_HMD = NULL;
				}

				if( !g_HMD ) {
					g_HMD = ovrHmd_CreateDebug(ovrHmd_DK2);
				}

				Sizei Tex0Size,Tex1Size;
				Tex0Size = ovrHmd_GetFovTextureSize(g_HMD, ovrEye_Left, g_HMD->DefaultEyeFov[0], 1.0f);
				Tex1Size = ovrHmd_GetFovTextureSize(g_HMD, ovrEye_Right, g_HMD->DefaultEyeFov[1], 1.0f);
				g_OvrRenderSize.w = Tex0Size.w + Tex1Size.w;
				g_OvrRenderSize.h = max(Tex0Size.h, Tex1Size.h);


				g_EyeRenderViewport[0].Pos  = Vector2i(0,0);
				g_EyeRenderViewport[0].Size = Sizei(g_OvrRenderSize.w / 2, g_OvrRenderSize.h);
				g_EyeRenderViewport[1].Pos  = Vector2i(0,0); // 1枚テクスチャの場合 Vector2i((g_OvrRenderSize.w + 1) / 2, 0);
				g_EyeRenderViewport[1].Size = g_EyeRenderViewport[0].Size;

				ovrBool ob;
				ob = ovrHmd_AttachToWindow(g_HMD, g_hWndDistortion, NULL, NULL);

				ovrD3D11Config conf;
				int width = g_HMD->Resolution.w;
				int height = g_HMD->Resolution.h;

				ZeroMemory(&conf, sizeof(conf));
				conf.D3D11.Header.API         = ovrRenderAPI_D3D11;
				conf.D3D11.Header.BackBufferSize = Sizei(width, height);
				conf.D3D11.Header.Multisample = 1;	/* これやるときれいになる？ */
				conf.D3D11.pDevice            = g_pd3dDevice;
				conf.D3D11.pDeviceContext     = g_pImmediateContext;
				conf.D3D11.pBackBufferRT      = g_pRenderTargetView;
				conf.D3D11.pSwapChain         = g_pSwapChain;

				ovrFovPort eyeFov[2];
				eyeFov[0] = g_HMD->DefaultEyeFov[0];
				eyeFov[1] = g_HMD->DefaultEyeFov[1];


				ob = ovrHmd_ConfigureRendering(g_HMD, &conf.Config,
					ovrDistortionCap_Vignette | ovrDistortionCap_Chromatic | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive | ovrDistortionCap_HqDistortion,
					eyeFov, g_EyeRenderDesc);

				ovrHmd_SetEnabledCaps(g_HMD, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction | ovrHmdCap_NoMirrorToWindow);
				ovrHmd_ConfigureTracking(g_HMD, ovrTrackingCap_Orientation |
												ovrTrackingCap_MagYawCorrection |
												ovrTrackingCap_Position, 0);

				ovrHmd_DismissHSWDisplay(g_HMD);

				ResumeThread(hOVREyeViewThread);

				CheckMenuItem(g_hMenu, ID_MENU_OVR_MIRROR, MF_BYCOMMAND | MF_UNCHECKED);

			}
			break;
#endif
		case ID_MENU_RENDER_FOV:
			bItemChecking = !bItemChecked;
			break;
#if 0
		case ID_MENU_D_MMD:

			bItemChecking = TRUE;
			CheckMenuItem(g_hMenu, (UINT)ID_MENU_D_FIXED, MF_BYCOMMAND | MF_UNCHECKED);

			SetWindowPos(g_hWnd, 0, 0, 0,
				g_rectLastMMDSize.right - g_rectLastMMDSize.left,
				g_rectLastMMDSize.bottom - g_rectLastMMDSize.top,
				SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

			g_bMMDSyncResize = TRUE;

			break;
		case ID_MENU_D_FIXED:

			bItemChecking = TRUE;
			CheckMenuItem(g_hMenu, (UINT)ID_MENU_D_MMD, MF_BYCOMMAND | MF_UNCHECKED);

			g_bMMDSyncResize = FALSE;

			break;
		case ID_MENU_D_TOP:

			bItemChecking = !bItemChecked;

			if( bItemChecking ) {
				SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			else {
				SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}

			break;
#endif

		default:
			break;
	}

	CheckMenuItem(g_hMenu, uMenuItemId, MF_BYCOMMAND | (bItemChecking ? MF_CHECKED : 0));
}

static LRESULT CALLBACK WndProcDistortion( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{

    switch( message )
    {
 		case WM_CREATE:
			return 0;
		case WM_DESTROY:
            break;
		case WM_CLOSE:
			/* 閉じるを無視する */
			/* MMD本体が終了する時にのみ許可する */
			if( bCloseEnable == TRUE ) {
				DestroyWindow(hWnd);
			}
			return 0;
		default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

// MMD終了通知を乗っ取るためにサブクラス化
static LRESULT CALLBACK MMDSubWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	if( message == WM_CLOSE ) {
	}
	else if( message == WM_DESTROY ) {
		bCloseEnable = TRUE;
		SendMessage(g_hWnd, WM_CLOSE, 0, 0);
		g_hWnd = NULL;

		WaitForSingleObject(g_hSemaphoreMMDShutdownBlock, INFINITE);
	}
	return CallWindowProc(g_WndMMDSubProc, hWnd, message, wParam, lParam);
}

