#include "stdafx.h"


#define WINDOW_TITLE		TEXT("MikuMikuVR for Oculus Rift Ver 0.39")
// #define WINDOW_TITLE		TEXT("MikuMikuVR for Oculus Rift Ver 0.39 RC2 Build " TEXT(__DATE__) TEXT(" / ") TEXT(__TIME__))
#define CLASS_NAME			TEXT("MMDOVRHookClass")


HWND g_hWnd;
HMENU g_hMenu;
static LPCTSTR g_pWindowTitle = WINDOW_TITLE;

/* MMDウィンドウ管理 */
HWND g_hWndMMD;

static const double g_MoveOffset = 0.2;
static const double g_RotateOffset = DEG2RAD(0.5);
static const double g_ZoomOffset = 0.01;

double g_dMovingPosX = 0.0;
double g_dMovingPosY = 0.0;
double g_dMovingPosZ = 0.0;
double g_dRotationY = 0.0;
double g_dFovZoom = 1.0;



static BOOL bCloseEnable = FALSE;


WNDPROC g_WndMMDSubProc;

static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
static LRESULT CALLBACK WndProcDistortion( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
static void KeyPressProc(WPARAM wpKey);
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

    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW & (~WS_OVERLAPPED), TRUE );
    g_hWnd = CreateWindow( wcex.lpszClassName, g_pWindowTitle, WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    return S_OK;
}

HRESULT SetupWindowState()
{
	g_hMenu = GetMenu(g_hWnd);

	return S_OK;
}

HRESULT HackMMDWindow(HWND hWndMMD)
{
	/* 実施済みならやらない */
	if( !g_hWndMMD ) {
		g_hWndMMD = hWndMMD;

		// MMD終了通知を乗っ取るためにサブクラス化
		g_WndMMDSubProc = (WNDPROC)SetWindowLongPtr(g_hWndMMD, GWLP_WNDPROC, (LONG_PTR)MMDSubWndProc);

		HMENU hMMDMenu = GetMenu(g_hWndMMD);
		EnableMenuItem(hMMDMenu, MMD_MENU_FULLSCREEN, MF_BYCOMMAND | MF_GRAYED);
		PostMessage(g_hWndMMD, WM_COMMAND, MMD_MENU_FPSNOLIMIT, 0);
	}
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
		case WM_CLOSE:
			/* 閉じるを無視する */
			/* MMD本体が終了する時にのみ許可する */
			if( bCloseEnable == TRUE ) {
				DestroyWindow(hWnd);
			}
			return 0;
		case WM_COMMAND:
			if( HIWORD(wParam) == 0 && ID_MENU < LOWORD(wParam) && LOWORD(wParam) < ID_MENU_END ) {
				MenuSelectionProc(LOWORD(wParam));
			}
			break;

		case WM_KEYDOWN:
			KeyPressProc(wParam);
			break;

		case WM_MMVR_EFFECTLOADED:
			{
				ovrHmdDesc hmdDesc;
				g_pRift->GetOvrHmdDesc(&hmdDesc);
				if( g_pMMEHookMirrorRT->GetTargetHmdType() != hmdDesc.Type) {
					TCHAR tWindowTitle[BUFFER_SIZE + 1];
					StringCchCopy(tWindowTitle, sizeof(tWindowTitle), g_pWindowTitle);
					StringCchCat(tWindowTitle, sizeof(tWindowTitle), TEXT(" | Warning: ") TEXT(MMEHACK_EFFECT_RENDERTARGET) TEXT(" HMD type unmatch."));
					SetWindowText(g_hWnd, tWindowTitle);
				}
			}
			break;
		case WM_MMVR_EFFECTUNLOADED:
			SetWindowText(g_hWnd, g_pWindowTitle);
			break;

		default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

static void KeyPressProc(WPARAM wpKey)
{
	switch( wpKey ) {
		case VK_UP:
		case VK_NUMPAD8:
			g_dMovingPosX += g_MoveOffset * sin(g_dRotationY);
			g_dMovingPosZ -= g_MoveOffset * cos(g_dRotationY);
			break;
		case VK_DOWN:
		case VK_NUMPAD2:
			g_dMovingPosX -= g_MoveOffset * sin(g_dRotationY);
			g_dMovingPosZ += g_MoveOffset * cos(g_dRotationY);
			break;
		case VK_LEFT:
		case VK_NUMPAD4:
			g_dMovingPosX += g_MoveOffset * cos(g_dRotationY);
			g_dMovingPosZ += g_MoveOffset * sin(g_dRotationY);
			break;
		case VK_RIGHT:
		case VK_NUMPAD6:
			g_dMovingPosX -= g_MoveOffset * cos(g_dRotationY);
			g_dMovingPosZ -= g_MoveOffset * sin(g_dRotationY);
			break;
		case VK_NUMPAD7:
			g_dMovingPosY += g_MoveOffset;
			break;
		case VK_NUMPAD9:
			g_dMovingPosY -= g_MoveOffset;
			break;
		case VK_NUMPAD1:
			g_dRotationY += g_RotateOffset;
			break;
		case VK_NUMPAD3:
			g_dRotationY -= g_RotateOffset;
			break;
		case VK_ADD:
			g_dFovZoom -= g_ZoomOffset;
			if( g_dFovZoom < g_ZoomOffset ) {
				g_dFovZoom = g_ZoomOffset;
			}
			break;
		case VK_SUBTRACT:
			g_dFovZoom += g_ZoomOffset;
			if( g_dFovZoom >= 1.0 ) {
				g_dFovZoom = 1.0;
			}
			break;
		case VK_SPACE:
			MenuSelectionProc(ID_MMDCTRL_PLAY);
			break;
		case 'R':
			MenuSelectionProc(ID_MENU_OCULUS_VIEWINIT);
			break;
		case 'M':
			MenuSelectionProc(ID_MENU_OVR_EYETEXTURE);
			break;
	}

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
		case ID_MENU_OCULUS_VIEWINIT:
			if( g_pRift )
				g_pRift->ResetTracking();

			g_dMovingPosX = 0.0;
			g_dMovingPosY = 0.0;
			g_dMovingPosZ = 0.0;
			g_dRotationY = 0.0;
			g_dFovZoom = 1.0;

			break;
#endif
		case ID_MMDCTRL_PLAY:
			PostMessage(g_hWndMMD, WM_COMMAND, MMD_MENU_FPSNOLIMIT, 0);
			PostMessage(g_hWndMMD, WM_COMMAND, MMD_BUTTON_PLAY, NULL);

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

		case ID_MENU_D_SRESET:
			{
				RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
			    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW & (~WS_OVERLAPPED), TRUE );
				SetWindowPos(g_hWnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
			}
			break;

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
		case WM_KEYDOWN:
			KeyPressProc(wParam);
			break;
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

