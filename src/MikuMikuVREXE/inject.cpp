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

	// �^�[�Q�b�g�v���Z�X�̃A�h���X��ԂɃo�b�t�@���m��
	lpRemotePage = VirtualAllocEx(hRemoteProcess, NULL, sizeof(tcLibraryPath), MEM_COMMIT, PAGE_READWRITE);

	if( lpRemotePage ) {
		bRet = WriteProcessMemory(hRemoteProcess, lpRemotePage, tcLibraryPath, sizeof(tcLibraryPath), &nBytesWritten);

		// DLL�̖��O���������߂���A�^�[�Q�b�g�v���Z�X���ɂ�DLL��ǂݍ��ރX���b�h���T�X�y���h��ԂŋN���B
		if( bRet && nBytesWritten == sizeof(tcLibraryPath) ) {
			hLoadThread = CreateRemoteThread(hRemoteProcess, NULL, 0,
				(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(pKernel32), pLoadLibrary), lpRemotePage, CREATE_SUSPENDED, NULL);
		}

		// �X���b�h�쐬�ɐ������Ă���ꍇ�̂݁B
		if( hLoadThread ) {
			// DLL�ǂݍ��݃X���b�h�̗D��x���������Ď��s�J�n����B
#ifndef _DEBUG	// �o�O�v���O�����˂�����ǂ��ėD��x����������Ɠ����Ȃ��Ȃ����ꍇ����
			SetThreadPriority(hLoadThread, THREAD_PRIORITY_HIGHEST);
#endif
			ResumeThread(hLoadThread);

			// �X���b�h�̏C����ҋ@����B
			WaitForSingleObject(hLoadThread, INFINITE);

			// hLoadedLibrary �͓ǂݍ��܂ꂽDLL�̃n���h���ƂȂ�B
			// hLoadedLibrary != NULL �ł����DLL���ǂݍ��܂ꂽ���ƂɂȂ�B
			// x64�̏ꍇhLoadedLibrary �͉���32bit�����󂯎��Ȃ����߁A
			// DLL�̐擪�A�h���X�𐳂����v�Z���邱�Ƃ��ł��Ȃ��B
			GetExitCodeThread(hLoadThread, (LPDWORD)&hLoadedLibrary);

			CloseHandle(hLoadThread);
		}
	}

	// �������J��
	if( lpRemotePage ) {
		VirtualFreeEx(hRemoteProcess, lpRemotePage, 0, MEM_RELEASE);
	}

	return hLoadedLibrary;
}
