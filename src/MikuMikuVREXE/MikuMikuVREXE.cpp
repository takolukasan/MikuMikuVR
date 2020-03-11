// DX9HOOK.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define MMD_EXE		TEXT("MikuMikuDance.exe")
#define HOOK_DLL	TEXT("MikuMikuVR.dll")

#define FILENAME_LENGTH	(4096)

extern HMODULE LoadLibraryRemote(HANDLE hRemoteProcess, LPCTSTR lpLibraryPath);

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwLength;
	TCHAR tcMyProcessParentName[FILENAME_LENGTH];
	static const TCHAR tcYen[] = TEXT("\\");

	TCHAR CommandLine[FILENAME_LENGTH + 1] = MMD_EXE;

	printf("---------- MikuMikuVR for Oculus Rift ----------\n");

	dwLength = GetModuleFileName(NULL, tcMyProcessParentName, sizeof(tcMyProcessParentName));
	if( dwLength >= 0 ) {
		size_t len;

		if( SUCCEEDED(StringCchLength(tcMyProcessParentName, FILENAME_LENGTH, &len)) ) {
			int nCompResult;
			static const TCHAR tcNull[] = TEXT("\0");

			while( len-- ) {
				nCompResult = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, tcYen, 1, tcMyProcessParentName + (len), (int)1);
				if( nCompResult == CSTR_EQUAL )
					break;
			}
			StringCchCopyN(tcMyProcessParentName + len, FILENAME_LENGTH - (len), tcNull, 1);
		}
		SetCurrentDirectory(tcMyProcessParentName);
		StringCchCopy(CommandLine, FILENAME_LENGTH, TEXT("\""));
		StringCchCat(CommandLine, FILENAME_LENGTH, tcMyProcessParentName);
		StringCchCat(CommandLine, FILENAME_LENGTH, tcYen);
		StringCchCat(CommandLine, FILENAME_LENGTH, MMD_EXE);
		StringCchCat(CommandLine, FILENAME_LENGTH, TEXT("\""));
	}

	size_t CommandLength = 0;
	StringCchLength(CommandLine, FILENAME_LENGTH, &CommandLength);

	if( argc >= 2 ) {
		wcsncat_s(CommandLine, FILENAME_LENGTH, TEXT(" "), 1);
		wcsncat_s(CommandLine, FILENAME_LENGTH, argv[1], FILENAME_LENGTH - CommandLength - 2);
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	si.cb = sizeof(si);

	CreateProcess(MMD_EXE, CommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);

	LoadLibraryRemote(pi.hProcess, HOOK_DLL);

	ResumeThread(pi.hThread);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}

