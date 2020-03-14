// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"


#define MMD_PROCESS_NAME	TEXT("MikuMikuDance.exe")


HMODULE g_hModMyLibrary;

BOOL g_bMMDHacked;
BOOL g_bMMEHacked;

TCHAR g_tcApplicationDirectory[FILENAME_LENGTH];


#ifdef DEBUG_CONSOLE
void PrintConsole(const char *szString, const int *pInt)
{
	char szStrBuffer[1024];
	int nCharWritten = 0;
	if( szString )
		nCharWritten = sprintf_s(szStrBuffer, sizeof(szStrBuffer), "%s", szString);
	if( pInt )
		nCharWritten += sprintf_s(szStrBuffer + nCharWritten, sizeof(szStrBuffer) - nCharWritten, "%d", *pInt);

	nCharWritten += sprintf_s(szStrBuffer + nCharWritten, sizeof(szStrBuffer) - nCharWritten, "\r\n");

	DWORD dwWritten;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), szStrBuffer, nCharWritten, &dwWritten, NULL);
}
void PrintConsole(const char *szString, const char *szString2)
{
	char szStrBuffer[1024];
	int nCharWritten = 0;
	if( szString )
		nCharWritten = sprintf_s(szStrBuffer, sizeof(szStrBuffer), "%s", szString);
	if( szString2 )
		nCharWritten += sprintf_s(szStrBuffer + nCharWritten, sizeof(szStrBuffer) - nCharWritten, "%s", szString2);

	nCharWritten += sprintf_s(szStrBuffer + nCharWritten, sizeof(szStrBuffer) - nCharWritten, "\r\n");

	DWORD dwWritten;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), szStrBuffer, nCharWritten, &dwWritten, NULL);
}
#endif


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

				StringCchCopyN(g_tcApplicationDirectory, FILENAME_LENGTH, tcMyProcessParentName, len2 - len1);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL HookD3DFunctions()
{
	HMODULE hModMyProcess = GetModuleHandle(NULL);

	if( !HookFunctionOfModule(hModMyProcess, "Direct3DCreate9", Hook_Direct3DCreate9, (void **)&g_orgDirect3DCreate9) )
		return FALSE;
	if( !HookFunctionOfModule(hModMyProcess, "D3DXCreateEffectFromResourceA", Hook_D3DXCreateEffectFromResourceA, (void **)&g_orgD3DXCreateEffectFromResourceA) )
		return FALSE;
#ifdef D3D9EX_ENABLE
	if( !HookFunctionOfModule(hModMyProcess, "D3DXCreateTexture", Hook_D3DXCreateTexture, (void **)&g_orgD3DXCreateTexture) )
		return FALSE;
	if( !HookFunctionOfModule(hModMyProcess, "D3DXCreateTextureFromFileExA", Hook_D3DXCreateTextureFromFileExA, (void **)&g_orgD3DXCreateTextureFromFileExA) )
		return FALSE;
	if( !HookFunctionOfModule(hModMyProcess, "D3DXCreateTextureFromFileExW", Hook_D3DXCreateTextureFromFileExW, (void **)&g_orgD3DXCreateTextureFromFileExW) )
		return FALSE;
	if( !HookFunctionOfModule(hModMyProcess, "D3DXCreateTextureFromFileInMemoryEx", Hook_D3DXCreateTextureFromFileInMemoryEx, (void **)&g_orgD3DXCreateTextureFromFileInMemoryEx) )
		return FALSE;
	if( !HookFunctionOfModule(hModMyProcess, "D3DXLoadMeshFromXInMemory", Hook_D3DXLoadMeshFromXInMemory, (void **)&g_orgD3DXLoadMeshFromXInMemory))
		return FALSE;
	if( !HookFunctionOfModule(hModMyProcess, "D3DXLoadMeshFromXW", Hook_D3DXLoadMeshFromXW, (void **)&g_orgD3DXLoadMeshFromXW))
		return FALSE;
#endif

#ifdef D3D9EX_ENABLE
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

