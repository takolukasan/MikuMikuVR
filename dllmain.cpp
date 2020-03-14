// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"



#define TIMELIMIT_ENABLE

#ifdef TIMELIMIT_ENABLE
SYSTEMTIME systimeDllLoad;

#define TIMELIMIT_YEAR	(2015)
#define TIMELIMIT_MONTH	(4)
#define TIMELIMIT_DAY	(30)

#endif


#define MMD_PROCESS_NAME	TEXT("MikuMikuDance.exe")
#define FILENAME_LENGTH		(1024)


HMODULE g_hModMyLibrary;

BOOL g_bMMDHacked;
BOOL g_bMMEHacked;

BOOL CheckTargetProcess()
{
	DWORD dwLength;
	TCHAR *tTargetProcess = MMD_PROCESS_NAME;
	TCHAR tcMyProcessParentName[FILENAME_LENGTH + 1];

	size_t len1,len2;
	int nCompResult;

	dwLength = GetModuleFileName(NULL, tcMyProcessParentName, sizeof(tcMyProcessParentName));
	if( dwLength >= 0 ) {
		if( SUCCEEDED(StringCchLength(tTargetProcess, FILENAME_LENGTH, &len1))
		 && SUCCEEDED(StringCchLength(tcMyProcessParentName, FILENAME_LENGTH, &len2)) ) {
		
			nCompResult = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, tTargetProcess, (int)len1, tcMyProcessParentName + (len2 - len1), (int)len1);
			if( CSTR_EQUAL == nCompResult ) {
				/* Found target */
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL HookD3DFunctions()
{
	if( !HookFunctionOfModule(GetModuleHandle(NULL), "Direct3DCreate9", Hook_Direct3DCreate9, (void **)&g_orgDirect3DCreate9) )
		return FALSE;
	if( !HookFunctionOfModule(GetModuleHandle(NULL), "D3DXCreateEffectFromResourceA", Hook_D3DXCreateEffectFromResourceA, (void **)&g_orgD3DXCreateEffectFromResourceA) )
		return FALSE;
	if( !HookFunctionOfModule(GetModuleHandle(NULL), "D3DXCreateTexture", Hook_D3DXCreateTexture, (void **)&g_orgD3DXCreateTexture) )
		return FALSE;
	if( !HookFunctionOfModule(GetModuleHandle(NULL), "D3DXCreateTextureFromFileExA", Hook_D3DXCreateTextureFromFileExA, (void **)&g_orgD3DXCreateTextureFromFileExA) )
		return FALSE;
	if( !HookFunctionOfModule(GetModuleHandle(NULL), "D3DXCreateTextureFromFileExW", Hook_D3DXCreateTextureFromFileExW, (void **)&g_orgD3DXCreateTextureFromFileExW) )
		return FALSE;
	if( !HookFunctionOfModule(GetModuleHandle(NULL), "D3DXCreateTextureFromFileInMemoryEx", Hook_D3DXCreateTextureFromFileInMemoryEx, (void **)&g_orgD3DXCreateTextureFromFileInMemoryEx) )
		return FALSE;

	TCHAR tcD3D9DllPath[FILENAME_LENGTH + 1] = TEXT("");
	HMODULE hModD3D9Dll;

	GetSystemDirectory(tcD3D9DllPath, FILENAME_LENGTH);
	StringCchCat(tcD3D9DllPath, FILENAME_LENGTH, TEXT("\\"));
	StringCchCat(tcD3D9DllPath, FILENAME_LENGTH, D3D9_DLL_NAME);

	hModD3D9Dll = GetModuleHandle(tcD3D9DllPath);

	DynDirect3DCreate9Ex = NULL;
	if( hModD3D9Dll ) {
		DynDirect3DCreate9Ex = (tDirect3DCreate9Ex)GetProcAddress(hModD3D9Dll, "Direct3DCreate9Ex");
	}

#if X64
	if( !DynDirect3DCreate9Ex )
		return FALSE;
#endif

	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
				/* ここでFALSEを返してDLL読み込み失敗とさせると、 MainProc() のコードが消えてお亡くなりになるので取り合えずTRUEを返すこと。 */

#ifdef TIMELIMIT_ENABLE
			/* DLL ロード条件を判定 */
			// GetLocaleInfo
			// GetSystemDefaultLCID
			// GetUserDefaultLCID
			GetLocalTime(&systimeDllLoad);
			if(   systimeDllLoad.wYear <  TIMELIMIT_YEAR
			 || ( systimeDllLoad.wYear == TIMELIMIT_YEAR && systimeDllLoad.wMonth <  TIMELIMIT_MONTH )
			 || ( systimeDllLoad.wYear == TIMELIMIT_YEAR && systimeDllLoad.wMonth == TIMELIMIT_MONTH && systimeDllLoad.wDay <= TIMELIMIT_DAY )
			) {
				/* 起動OK */
			}
			else {
				return FALSE;
			}

#endif

			if( CheckTargetProcess() ) {
				g_hModMyLibrary = hModule;

				g_bMMDHacked = HookD3DFunctions();

				g_bMMEHacked = MMEHack();

				g_hMainProcThread = CreateThread(NULL, 0, MainProc, NULL, 0, &g_dwMainProcThreadId);

			}
			else {
				g_hModMyLibrary = hModule;
				g_bMMDHacked = FALSE;
				g_bMMEHacked = FALSE;
				g_hMainProcThread = NULL;
				g_dwMainProcThreadId = 0;
			}

			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

