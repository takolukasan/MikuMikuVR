#include "stdafx.h"

#define LIBRARY_MAX_PATH	(MAX_PATH * 3)

HMODULE LoadLibraryRemote(HANDLE hRemoteProcess, LPCTSTR lpLibraryPath)
{
	HMODULE hLoadedLibrary = NULL;

	// For VirtualAllocEx()
	LPVOID lpRemotePage = NULL;

	// For WriteProcessMemory()
	SIZE_T nBytesWritten = 0;
	BOOL bRet;

	// For CreateRemoteThread()
	//LPCTSTR pKernel32 = TEXT("kernel32.dll");
	volatile LPCTSTR pKernel32;
	volatile TCHAR tcKernel32[13];
	volatile CHAR tcLoadLibrary[13];
	volatile LPCSTR pLoadLibrary;

#ifndef UNICODE
	// LPCSTR pLoadLibrary = "LoadLibraryA";
	tcLoadLibrary[11] = 'A';
#else
	// LPCSTR pLoadLibrary = "LoadLibraryW";
	tcLoadLibrary[11] = 'W';
#endif
	tcKernel32[12] = L'\0';
	tcKernel32[7] = L'2';
	tcLoadLibrary[1] = 'o';
	tcLoadLibrary[8] = 'a';
	tcLoadLibrary[6] = 'b';
	tcKernel32[2] = L'r';
	tcLoadLibrary[4] = 'L';
	tcLoadLibrary[2] = 'a';
	tcKernel32[9] = L'd';
	tcLoadLibrary[12] = '\0';
	tcLoadLibrary[5] = 'i';
	tcKernel32[10] = L'l';
	tcLoadLibrary[3] = 'd';
	pLoadLibrary = (LPCSTR)&tcLoadLibrary[0];
	tcLoadLibrary[9] = 'r';
	tcKernel32[5] = L'l';
	tcKernel32[4] = L'e';
	tcKernel32[0] = L'k';
	tcKernel32[8] = L'.';
	pKernel32 = (LPCTSTR)(&tcKernel32[0]);
	tcKernel32[11] = L'l';
	tcKernel32[3] = L'n';
	tcLoadLibrary[10] = 'y';
	tcKernel32[6] = L'3';
	tcLoadLibrary[0] = 'L';
	tcKernel32[1] = L'e';
	tcLoadLibrary[7] = 'r';


	// Buffer of lpLibraryPath
	TCHAR tcLibraryPath[LIBRARY_MAX_PATH + 1];

	// Handle of Remote Thread
	HANDLE hLoadThread = NULL;

	ZeroMemory(tcLibraryPath, sizeof(tcLibraryPath));

	GetCurrentDirectory(sizeof(tcLibraryPath), tcLibraryPath);

	StringCchCatN(tcLibraryPath, sizeof(tcLibraryPath), TEXT("\\"), MAX_PATH);
	StringCchCatN(tcLibraryPath, sizeof(tcLibraryPath), lpLibraryPath, MAX_PATH);
	//if(FAILED(StringCchCopyN(tcLibraryPath, LIBRARY_MAX_PATH, lpLibraryPath, LIBRARY_MAX_PATH))) {
	//	return NULL;
	//}

	// ターゲットプロセスのアドレス空間にバッファを確保
	lpRemotePage = VirtualAllocEx(hRemoteProcess, NULL, sizeof(tcLibraryPath), MEM_COMMIT, PAGE_READWRITE);

	if( lpRemotePage ) {
		bRet = WriteProcessMemory(hRemoteProcess, lpRemotePage, tcLibraryPath, sizeof(tcLibraryPath), &nBytesWritten);

		// DLLの名前を書き込めたら、ターゲットプロセス内にてDLLを読み込むスレッドをサスペンド状態で起動。
		if( bRet && nBytesWritten == sizeof(tcLibraryPath) ) {
			hLoadThread = CreateRemoteThread(hRemoteProcess, NULL, 0,
				(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(pKernel32), pLoadLibrary), lpRemotePage, CREATE_SUSPENDED, NULL);
		}

		// スレッド作成に成功している場合のみ。
		if( hLoadThread ) {
			// DLL読み込みスレッドの優先度を高くして実行開始する。
#ifndef _DEBUG	// バグプログラム突っ込んどいて優先度が高すぎると動かなくなった場合困る
			SetThreadPriority(hLoadThread, THREAD_PRIORITY_HIGHEST);
#endif
			ResumeThread(hLoadThread);

			// スレッドの修了を待機する。
			WaitForSingleObject(hLoadThread, INFINITE);

			// hLoadedLibrary は読み込まれたDLLのハンドルとなる。
			// hLoadedLibrary != NULL であればDLLが読み込まれたことになる。
			// x64の場合hLoadedLibrary は下位32bitしか受け取れないため、
			// DLLの先頭アドレスを正しく計算することができない。
			GetExitCodeThread(hLoadThread, (LPDWORD)&hLoadedLibrary);

			CloseHandle(hLoadThread);
		}
	}

	// メモリ開放
	if( lpRemotePage ) {
		VirtualFreeEx(hRemoteProcess, lpRemotePage, 0, MEM_RELEASE);
	}

	return hLoadedLibrary;
}
